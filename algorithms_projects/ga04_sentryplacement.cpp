#include "ga04_sentryplacement.h"

#include <QPainter>
#include <QDebug>
#include <stack>

TriangleLookupNode::TriangleLookupNode(SentryField* field)
    :triangle(field),
     children{nullptr}
{
    vertices[0] = field->innerComponent->origin->coords;
    vertices[1] = field->innerComponent->next->origin->coords;
    vertices[2] = field->innerComponent->next->next->origin->coords;
}

SentryVertex::operator QString() const
{
    if(nullptr == this->coords)
    {
        return "(UNKOWN, UNKNOWN)";
    }

    QString result = QStringLiteral("(%1,%2|%3)").arg(this->coords->x()).arg(this->coords->y()).arg(this->color);
    return result;
}

SentryField::operator QString() const
{
    const int cNumEdges = 3;
    QString result = "[";

    SentryHalfEdge *fieldEdge = this->innerComponent;

    for(int i=0; i<cNumEdges; ++i)
    {
        if(nullptr == fieldEdge)
        {
            result += "NULL ] - rest is unreachable";
            break;
        }

        if(nullptr == fieldEdge->origin)
        {
            result += "NULL";
        }
        else
        {
            result += (QString)(*(fieldEdge->origin));
        }

        if(i < cNumEdges - 1)
        {
            result += ",";
        }

        fieldEdge = fieldEdge->next;
    }

    result += "]";
    return result;
}

SentryField::~SentryField()
{
    // Postavljamo pokazivace na nullptr, da bismo otkrili potencijalnu gresku u dealokaciji.
    //
    outerComponent = nullptr;
    innerComponent = nullptr;

    // Unistavamo veze koje cvor pretrage ima sa ostatkom stabla, kao i povratnu referencu,
    // pre njegovog brisanja.
    //
    if(nullptr != lookupNode)
    {
        lookupNode->children[0] = nullptr;
        lookupNode->children[1] = nullptr;
        lookupNode->children[2] = nullptr;
        lookupNode->triangle = nullptr;
        if(nullptr != lookupNode->vertices[0])
        {
            delete lookupNode->vertices[0];
        }

        if(nullptr != lookupNode->vertices[1])
        {
            delete lookupNode->vertices[1];
        }

        if(nullptr != lookupNode->vertices[2])
        {
            delete lookupNode->vertices[2];
        }

        delete lookupNode;
    }
}

SentryDcell::~SentryDcell()
{
    std::set<QPoint*> allPoints;
    for(auto it = _vertices.begin(); it!=_vertices.end(); it++)
    {
        allPoints.insert((*it)->coords);
    }

    for(auto it = _vertices.begin(); it!=_vertices.end(); it++)
    {
        delete *it;
    }

    for(auto it = _edges.begin(); it!=_edges.end(); it++)
    {
        delete *it;
    }

    for(auto it = _fields.begin(); it!=_fields.end(); it++)
    {
        delete *it;
    }

    for(auto it = _deletedEdges.begin(); it!=_deletedEdges.end(); it++)
    {
        delete *it;
    }

    for(auto it = _deletedFields.begin(); it!=_deletedFields.end(); it++)
    {
        delete *it;
    }

    delete _unlimitedField;
}

SentryDcell::operator QString() const
{
    QString result = QString("Broj trouglova: %1").arg(this->_fields.size());
    for(auto it = this->_fields.cbegin(); it != this->_fields.cend(); ++it)
    {
        result += "," + QString(**it);
    }

    return result;
}

void SentryDcell::deleteField(SentryField *triangle)
{
    _deletedFields.push_back(triangle);
    _fields.erase(std::remove(_fields.begin(), _fields.end(), triangle), _fields.end());
}

void SentryDcell::deleteEdge(SentryHalfEdge *edge)
{
    _deletedEdges.push_back(edge);
    _edges.erase(std::remove(_edges.begin(), _edges.end(), edge), _edges.end());
}

SentryPlacement::SentryPlacement(
        QWidget *pRenderer,
        int delayMs,
        std::string fileName,
        bool isNaive,
        double timeoutSeconds,
        int numberOfPoints)
    :AlgorithmBase (pRenderer, delayMs),
      _timeoutSeconds{timeoutSeconds},
      _isNaive{isNaive},
      _completed{false},
      _triangulation{nullptr},
      _triangleLookupTreeRoot{nullptr},
      _currentPoint{nullptr},
      _edgeToHighlight{nullptr},
      _newlyAddedEdgeVertexX{nullptr},
      _newlyAddedEdgeVertexY{nullptr}
{
    if(fileName == "")
        _roomCorners = generateRandomSimplePoly(numberOfPoints);
    else
        _roomCorners = readPointsFromFile(fileName);
}

void SentryPlacement::runNaiveAlgorithm()
{
    std::set<QPoint, CmpQPoint> visibleCorners;
    _completed = false;

    while(_completed != true)
    {
        QPoint nextSentry = this -> getRandomSentry();

        // Ukoliko je x koordinata sledećeg slučajno odabranog stražara negativna,
        // to znači da je ukupno dozvoljeno vreme izvrsavanja prekoračeno, tako da treba uključiti stražara u spisak
        // samo ukoliko je vrednost x koordinate nenegativna.
        if(nextSentry.x() >= 0)
        {
            _selectedSentries.push_back(nextSentry);
            this -> updateVisibleCorners(nextSentry, visibleCorners);
        }

        if(_consumedSeconds > _timeoutSeconds || visibleCorners.size() == _roomCorners.size())
        {
            _timedOut = _consumedSeconds > _timeoutSeconds;
            _completed = true;
        }

        AlgorithmBase_updateCanvasAndBlock();
    }

    emit animationFinished();

    // Brišemo podatke od trenutnog pokretanja algoritma.
    // Ovo je neophodno zbog pokretanja naivnog i glavnog algoritma u neinteraktivnom modu,
    // koji po svemu sudeci ocekuju da je objekat za pokretanje algoritma moguce koristiti vise puta uzastopno.
    //
    clearRunData();
}

void SentryPlacement::runAlgorithm()
{
    // Postavljamo oznaku da algoritam nije zavrsen.
    // Ovo ne moze biti uradjeno u okviru ciscenja podataka algoritma, zato sto bi u suprotnom svaki end-to-end functional test
    // vratio pogresan rezultat, jer ne bi bio u stanju da ocita krajnji rezultat rada algoritma.
    //
    _completed = false;

    // Skretnica kojom se preskače na naivni algoritam, ukoliko je odgovarajuća promenljiva postavljena na 'true'.
    //
    if(_isNaive)
    {
        runNaiveAlgorithm();
        return;
    }

    // Inicijalizacija triangulacije.
    // Postavlja i povezuje sve potrebne strukture pretrage i odgovorna je za kreiranje sveobuhvatnog trougla.
    //
    this->initTriangulation();

    // Glavna petlja algoritma, u kojoj se jedna po jedna tačka dodaju u trenutnu triangulaciju.
    //
    for(std::vector<QPoint>::iterator it = _verticesToInsert.begin(); it != _verticesToInsert.end(); it++)
    {
        this->insertVertexToTriangulation(&(*it));
    }

    // Uklanjanje linija koje vode do imaginarnih i ogranicavajucih temena na crtezu.
    //
    _includeSymbolicLines = false;
    AlgorithmBase_updateCanvasAndBlock();

    try
    {
        this->doThreeColoring();
        this->selectFinalSentries();
    }
    catch(...)
    {
        qDebug() << "Unsuccessful coloring";
    }

    _completed = true;

    AlgorithmBase_updateCanvasAndBlock();
    emit animationFinished();

    // Brišemo podatke od trenutnog pokretanja algoritma.
    // Ovo je neophodno zbog pokretanja naivnog i glavnog algoritma u neinteraktivnom modu,
    // koji po svemu sudeci ocekuju da je objekat za pokretanje algoritma moguce koristiti vise puta uzastopno.
    //
    clearRunData();
}

void SentryPlacement::drawAlgorithm(QPainter &painter) const
{
    this->drawRoom(painter);
    this->drawCurrentPoint(painter);
    this->drawTriangulationLines(painter);
    this->drawEdgeToHighlight(painter);
    this->drawSentries(painter);
    this->drawSentriesCountText(painter);
    this->drawCurrentLookupNode(painter);
}

bool SentryPlacement::isCompleted() const
{
    return _completed;
}

void SentryPlacement::initDcell()
{
    if(nullptr != _triangulation)
    {
        delete _triangulation;
    }

    // Kreiramo novu, praznu DCELL strukturu za potrebe algoritma.
    //
    _triangulation = new SentryDcell();

    // Prvo teme koje dodajemo jeste ekstremno teme - teme sa najvećom y koordinatom (i najvećom x, u slučaju jednakih y koordinata).
    //
    _triangulation->lvReal = new QPoint(_boundingPointX, _boundingPointY);

    // Kreiramo dva nova polja, jedno za ograničeni, a drugo za neograničeni deo ravni.
    //
    SentryField* f = new SentryField();
    SentryField* f_unlimited = new SentryField();

    // Kreiramo tri ograničavajuća temena.
    //
    QPoint symbolic_1 = SentryPlacementUtils::makeP1Symbolic();
    QPoint symbolic_2 = SentryPlacementUtils::makeP2Symbolic();
    QPoint real_1 = *(_triangulation->lvReal);

    SentryVertex *a = new SentryVertex(&symbolic_2);
    SentryVertex *b = new SentryVertex(&real_1);
    SentryVertex *c = new SentryVertex(&symbolic_1);

    // Biće nam potrebno 6 ivica za povezivanje, tri spoljašnje i tri unutrašnje.
    //
    SentryHalfEdge *ab = new SentryHalfEdge();
    SentryHalfEdge *ba = new SentryHalfEdge();
    SentryHalfEdge *ac = new SentryHalfEdge();
    SentryHalfEdge *ca = new SentryHalfEdge();
    SentryHalfEdge *bc = new SentryHalfEdge();
    SentryHalfEdge *cb = new SentryHalfEdge();

    // Postavljamo ivice dveju oblasti.
    // Neograničena oblast po definiciji nema spoljašnju komponentu.
    //
    f->outerComponent = ab;
    f->innerComponent = ba;
    f_unlimited->outerComponent = nullptr;
    f_unlimited->innerComponent = ab;

    // Za svaki čvor navodimo po neku ivicu koja izlazi iz njega.
    //
    //a->incidentEdge = ab;
    //b->incidentEdge = bc;
    //c->incidentEdge = ca;

    // Povezujemo u lanac ivice.
    //
    ab->next = bc;
    bc->next = ca;
    ca->next = ab;
    ba->next = ac;
    ac->next = cb;
    cb->next = ba;

    // Povezujemo ivice sa svojim blizancima.
    //
    ab->twin = ba;
    ba->twin = ab;
    bc->twin = cb;
    cb->twin = bc;
    ca->twin = ac;
    ac->twin = ca;

    // Povezujemo svaku ivicu sa njenim matičnim temenom.
    //
    ab->origin = a;
    bc->origin = b;
    ca->origin = c;
    ba->origin = b;
    ac->origin = a;
    cb->origin = c;

    // Povezujemo svaku ivicu sa poljem kojem je unutrašnja.
    //
    ab->incidentFace = f_unlimited;
    bc->incidentFace = f_unlimited;
    ca->incidentFace = f_unlimited;
    ac->incidentFace = f;
    cb->incidentFace = f;
    ba->incidentFace = f;

    _triangulation->_unlimitedField = f_unlimited;
    _triangulation->_fields.push_back(f);
    _triangulation->_vertices.push_back(a);
    _triangulation->_vertices.push_back(b);
    _triangulation->_vertices.push_back(c);
    _triangulation->_edges.push_back(ab);
    _triangulation->_edges.push_back(bc);
    _triangulation->_edges.push_back(ca);
    _triangulation->_edges.push_back(ac);
    _triangulation->_edges.push_back(cb);
    _triangulation->_edges.push_back(ba);
}

void SentryPlacement::insertVertexToTriangulation(QPoint *pNewVertexCoords)
{
    // Označavamo posebnom bojom teme koje se trenutno obrađuje.
    //
    _currentPoint = pNewVertexCoords;

    // Novo teme koje se obrađuje se dodaje u spisak temena uključenih u triangulaciju.
    //
    SentryVertex *n = new SentryVertex(pNewVertexCoords);
    qDebug() << "U triangulaciju unosimo teme:"<< *n;
    _triangulation->_vertices.push_back(n);
    qDebug() << "Dodato je u vektor teme:" << *(_triangulation->_vertices.back());

    // Tražimo polje u strukturi za pretragu koje sadrži novu tačku.
    // Dokle god nismo došli do lista drveta pretrage, spuštamo se jedan nivo dublje.
    // Svaki element koji nije list može imati dvoje ili troje dece.
    //
    TriangleLookupNode* owner = _triangleLookupTreeRoot;
    _currentLookupNode = owner;
    while(owner->children[0] != nullptr)
    {
        owner = owner->children[SentryPlacementUtils::getChildIndex(owner, _currentPoint)];
        _currentLookupNode = owner;
        AlgorithmBase_updateCanvasAndBlock();
    }

    _currentLookupNode = nullptr;

    // Odredili smo trougao koji sadrži tačku - sada proveravamo da li je tačka na nekoj od ivica trougla, ili je u unutrašnjosti.
    // Od toga zavisi na koji način će se algoritam dalje odvijati.
    //
    SentryHalfEdge *edgeContainingPoint = SentryPlacementUtils::onTheTriangleEdge(n, owner->triangle);

    if(nullptr != edgeContainingPoint)
    {
        this->insertVertexOnEdge(edgeContainingPoint, n);
    }
    else
    {
        this->insertVertexInInterior(owner->triangle, n);
    }
}

void SentryPlacement::insertVertexOnEdge(SentryHalfEdge *edgeContainingPoint, SentryVertex *n)
{
    // Vrsimo imenovanje promenljivih radi lakseg pracenja algoritma.
    // Novo teme (n) treba dodati na ivicu ij.
    // Neka su polja fk i fl susedna ivici ij.
    // Neka su k i l 'treca temena' polja fk i fl, respektivno.
    // Tada je potrebno:
    // - ukloniti poluivice ij i ji
    // - ukloniti polja fk i fl
    // - kreirati cetiri para novih poluivica, koje ce povezivati teme n sa temenima i,k,j,l.
    // - kreirati cetiri nova polja, za cetiri nova trougla koji nastaju.
    //

    // Identifikovanje komponenti zahvacenih promenom.
    //
    SentryHalfEdge *ij = edgeContainingPoint;
    SentryHalfEdge *ji = ij->twin;
    SentryHalfEdge *ik = ji->next;
    SentryHalfEdge *kj = ik->next;
    SentryHalfEdge *jl = ij->next;
    SentryHalfEdge *li = jl->next;

    SentryVertex *i = ij->origin;
    SentryVertex *j = ji->origin;
    SentryVertex *k = kj->origin;
    SentryVertex *l = li->origin;

    SentryField *fl = ij->incidentFace;
    SentryField *fk = ji->incidentFace;

    qDebug() << "Nova tacka se nalazi na ivici [(" << i->coords->x() << "," << i->coords->y()
             << "), (" << j->coords->x() << "," << j->coords->y() << ")]"
             << "a druge dve tacke su [(" << k->coords->x() << "," << k->coords->y()
                         << "), (" << l->coords->x() << "," << l->coords->y() << ")]";

    // Uklanjamo polja koja vise nece biti potrebna iz triangulacije.
    //
    _triangulation->deleteField(fl);
    _triangulation->deleteField(fk);

    // Uklanjamo poluivice koje nece vise biti potrebne iz triangulacije.
    //
    _triangulation->deleteEdge(ij);
    _triangulation->deleteEdge(ji);

    // Kreiramo cetiri nova polja.
    //
    SentryField *fjl = new SentryField();
    SentryField *fli = new SentryField();
    SentryField *fik = new SentryField();
    SentryField *fkj = new SentryField();

    // Kreiramo cetiri nova para poluivica.
    //
    SentryHalfEdge *nj = new SentryHalfEdge();
    SentryHalfEdge *jn = new SentryHalfEdge();
    SentryHalfEdge *ni = new SentryHalfEdge();
    SentryHalfEdge *in = new SentryHalfEdge();
    SentryHalfEdge *nk = new SentryHalfEdge();
    SentryHalfEdge *kn = new SentryHalfEdge();
    SentryHalfEdge *nl = new SentryHalfEdge();
    SentryHalfEdge *ln = new SentryHalfEdge();

    // Povezivanje novih polja
    //
    fjl->innerComponent = jl;
    fjl->outerComponent = jl->twin;
    fli->innerComponent = li;
    fli->outerComponent = li->twin;
    fik->innerComponent = ik;
    fik->outerComponent = ik->twin;
    fkj->innerComponent = kj;
    fkj->outerComponent = kj->twin;

    // Prelancavanje unutrasnjih ivica
    //
    jl->next = ln; //
    ln->next = nj; // polje fjl
    nj->next = jl; //

    li->next = in; //
    in->next = nl; // polje fli
    nl->next = li; //

    ik->next = kn; //
    kn->next = ni; // polje fik
    ni->next = ik; //

    kj->next = jn; //
    jn->next = nk; // polje fkj
    nk->next = kj; //

    // Povezivanje sa pripadajucim unutrasnjim oblastima
    //
    kj->incidentFace = fkj;
    jn->incidentFace = fkj;
    nk->incidentFace = fkj;

    jl->incidentFace = fjl;
    ln->incidentFace = fjl;
    nj->incidentFace = fjl;

    li->incidentFace = fli;
    in->incidentFace = fli;
    nl->incidentFace = fli;

    ik->incidentFace = fik;
    kn->incidentFace = fik;
    ni->incidentFace = fik;

    // Povezivanje novih parova blizanaca.
    //
    nj->twin = jn;
    jn->twin = nj;
    nl->twin = ln;
    ln->twin = nl;
    ni->twin = in;
    in->twin = ni;
    nk->twin = kn;
    kn->twin = nk;

    // Povezivanje novih ivica sa temenima.
    //
    nj->origin = n;
    jn->origin = j;
    nl->origin = n;
    ln->origin = l;
    ni->origin = n;
    in->origin = i;
    nk->origin = n;
    kn->origin = k;

    // Povezivanje novog temena sa nekom od novih stranica
    // koje iz njega počinju.
    //
    //n->incidentEdge = nj;

    // Dodavanje podataka u stablo pretrage.
    //
    fl->lookupNode->children[0] = new TriangleLookupNode(fjl);
    fl->lookupNode->children[1] = new TriangleLookupNode(fli);
    fl->lookupNode->children[2] = nullptr;

    fk->lookupNode->children[0] = new TriangleLookupNode(fik);
    fk->lookupNode->children[1] = new TriangleLookupNode(fkj);
    fk->lookupNode->children[2] = nullptr;

    // Dodavanje povratnih referenci na stablo pretrage.
    //
    fjl->lookupNode = fl->lookupNode->children[0];
    fli->lookupNode = fl->lookupNode->children[1];
    fik->lookupNode = fk->lookupNode->children[0];
    fkj->lookupNode = fk->lookupNode->children[1];

    // Dodavanje novostvorenih trouglova i ivica u triangulaciju.
    //
    _triangulation->_fields.push_back(fjl);
    _triangulation->_fields.push_back(fli);
    _triangulation->_fields.push_back(fik);
    _triangulation->_fields.push_back(fkj);

    _triangulation->_edges.push_back(ni);
    _triangulation->_edges.push_back(in);
    _triangulation->_edges.push_back(nj);
    _triangulation->_edges.push_back(jn);
    _triangulation->_edges.push_back(nl);
    _triangulation->_edges.push_back(ln);
    _triangulation->_edges.push_back(nk);
    _triangulation->_edges.push_back(kn);

    // Osvežavanje prikaza nakon izvršene inicijalne podele.
    //
    AlgorithmBase_updateCanvasAndBlock();

    // Legalizacija ivica.
    //
    std::stack<SentryHalfEdge*> edgesForLegalization;
    edgesForLegalization.push(jl);
    edgesForLegalization.push(li);
    edgesForLegalization.push(ik);
    edgesForLegalization.push(kj);
    legalizeEdges(n, edgesForLegalization);

    // Osvežavanje prikaza nakon završene legalizacije ivica.
    //
    AlgorithmBase_updateCanvasAndBlock();
}

void SentryPlacement::insertVertexInInterior(SentryField *f, SentryVertex *n)
{
    // Identifikovanje komponenti zahvaćenih promenom.
    //
    SentryHalfEdge *ab = f->innerComponent;
    SentryHalfEdge *bc = ab->next;
    SentryHalfEdge *ca = bc->next;

    SentryHalfEdge *ba = ab->twin;
    SentryHalfEdge *cb = bc->twin;
    SentryHalfEdge *ac = ca->twin;

    // Trougao u kojem smo trenutno se uklanja iz triangulacije.
    //
    _triangulation->deleteField(f);

    // Ukoliko se tačka nalazi baš u unutrašnjosti polja,
    // onda je potrebno podeliti dato polje na tri nova polja.
    //
    SentryField *nab = new SentryField();
    SentryField *nbc = new SentryField();
    SentryField *nca = new SentryField();

    _triangulation->_fields.push_back(nab);
    _triangulation->_fields.push_back(nbc);
    _triangulation->_fields.push_back(nca);

    // ... i dodati šest novih poluivica.
    //
    SentryHalfEdge *an = new SentryHalfEdge();
    SentryHalfEdge *na = new SentryHalfEdge();
    SentryHalfEdge *bn = new SentryHalfEdge();
    SentryHalfEdge *nb = new SentryHalfEdge();
    SentryHalfEdge *cn = new SentryHalfEdge();
    SentryHalfEdge *nc = new SentryHalfEdge();

    _triangulation->_edges.push_back(an);
    _triangulation->_edges.push_back(na);
    _triangulation->_edges.push_back(bn);
    _triangulation->_edges.push_back(nb);
    _triangulation->_edges.push_back(cn);
    _triangulation->_edges.push_back(nc);

    // Povezivanje novih polja.
    //
    nab->outerComponent = ba;
    nab->innerComponent = ab;
    nbc->outerComponent = cb;
    nbc->innerComponent = bc;
    nca->outerComponent = ac;
    nca->innerComponent = ca;

    // Prelančavanje unutrašnjih lanaca.
    //
    ab->next = bn;
    bn->next = na;
    na->next = ab;

    bc->next = cn;
    cn->next = nb;
    nb->next = bc;

    ca->next = an;
    an->next = nc;
    nc->next = ca;

    // Povezivanje sa pripadajućim unutrašnjim oblastima.
    //
    ab->incidentFace = nab;
    bn->incidentFace = nab;
    na->incidentFace = nab;
    bc->incidentFace = nbc;
    cn->incidentFace = nbc;
    nb->incidentFace = nbc;
    ca->incidentFace = nca;
    an->incidentFace = nca;
    nc->incidentFace = nca;

    // Povezivanje novih parova blizanaca.
    //
    an->twin = na;
    na->twin = an;
    bn->twin = nb;
    nb->twin = bn;
    cn->twin = nc;
    nc->twin = cn;

    // Povezivanje novih ivica sa temenima.
    //
    an->origin = ab->origin;
    bn->origin = bc->origin;
    cn->origin = ca->origin;
    na->origin = n;
    nb->origin = n;
    nc->origin = n;

    // Povezivanje novog temena sa nekom od novih stranica
    // koje iz njega počinju.
    //
    // n->incidentEdge = na;

    // Kreiranje novih čvorova stabla pretrage.
    //
    TriangleLookupNode *tnab = new TriangleLookupNode(nab);
    TriangleLookupNode *tnbc = new TriangleLookupNode(nbc);
    TriangleLookupNode *tnca = new TriangleLookupNode(nca);

    // Dodavanje novih čvorova kao dece trenutnog čvora koji se obradjuje.
    //
    f->lookupNode->children[0] = tnab;
    f->lookupNode->children[1] = tnbc;
    f->lookupNode->children[2] = tnca;

    // Dodavanje povratnih referenci na stablo pretrage.
    //
    nab->lookupNode = tnab;
    nbc->lookupNode = tnbc;
    nca->lookupNode = tnca;

    // Osvežavanje prikaza nakon izvršene inicijalne podele.
    //
    AlgorithmBase_updateCanvasAndBlock();

    // Legalizacija ivica.
    //
    std::stack<SentryHalfEdge*> edgesForLegalization;
    edgesForLegalization.push(ca);
    edgesForLegalization.push(ab);
    edgesForLegalization.push(bc);
    legalizeEdges(n, edgesForLegalization);

    // Osvežavanje prikaza nakon završene legalizacije ivica.
    //
    AlgorithmBase_updateCanvasAndBlock();
}

SentryField *SentryPlacement::getFirstNonImaginaryField()
{
    for(auto fieldIterator = _triangulation->_fields.begin(); fieldIterator != _triangulation->_fields.end(); ++fieldIterator)
    {
        if(hasNoImaginaryOrBoundingPoint(*fieldIterator))
        {
            return *fieldIterator;
        }
    }

    return nullptr;
}

bool SentryPlacement::hasNoImaginaryOrBoundingPoint(SentryField *pField)
{
    return !isSymbolicOrBounding(pField->innerComponent->origin)
            && !isSymbolicOrBounding(pField->innerComponent->next->origin)
            && !isSymbolicOrBounding(pField->innerComponent->next->next->origin);
}

bool SentryPlacement::isSymbolicOrBounding(SentryVertex *pVertex) const
{
    return SentryPlacement::isSymbolicOrBounding(pVertex->coords);
}

bool SentryPlacement::isSymbolicOrBounding(QPoint *pPoint) const
{
    return SentryPlacementUtils::isSymbolic(pPoint) ||
            ((pPoint->x() == _boundingPointX || pPoint->x() == 0) && (pPoint->y() == _boundingPointY || pPoint->y() == 0));
}

void SentryPlacement::doThreeColoring()
{
    // Vrsimo tri-bojenje svih temena
    //
    SentryField *firstField = getFirstNonImaginaryField();
    qDebug() << *_triangulation;

    if(nullptr == firstField)
    {
        qDebug() << "Ima suvise malo temena, te ne postoji nijedan trougao u triangulaciji koji ima sve neimaginarne ivice.";
        return;
    }

    std::list<SentryField*> fieldsStack;
    fieldsStack.push_front(firstField);

    while(!fieldsStack.empty())
    {
        SentryField *currentField = fieldsStack.front();
        fieldsStack.pop_front();
        if(nullptr == currentField->outerComponent || !SentryPlacementUtils::colorize(currentField))
        {
            continue;
        }

        AlgorithmBase_updateCanvasAndBlock();

        fieldsStack.push_back(currentField->innerComponent->twin->incidentFace);
        fieldsStack.push_back(currentField->innerComponent->next->twin->incidentFace);
        fieldsStack.push_back(currentField->innerComponent->next->next->twin->incidentFace);
    }
    AlgorithmBase_updateCanvasAndBlock();
}

void SentryPlacement::selectFinalSentries()
{
    int colorCount[5] = {0};

    for(auto itVertex = _triangulation->_vertices.cbegin(); itVertex != _triangulation->_vertices.cend(); ++itVertex)
    {
        ++colorCount[(*itVertex)->color];
    }

    SentryVertexColoring::Color minColor = SentryVertexColoring::Color::Blue;
    if(colorCount[SentryVertexColoring::Color::Green] < colorCount[minColor])
    {
        minColor = SentryVertexColoring::Color::Green;
    }

    if(colorCount[SentryVertexColoring::Color::Red] < colorCount[minColor])
    {
        minColor = SentryVertexColoring::Color::Red;
    }

    for(auto itVertex = _triangulation->_vertices.cbegin(); itVertex != _triangulation->_vertices.cend(); ++itVertex)
    {
        if((*itVertex)->color == minColor && !isSymbolicOrBounding(*itVertex))
        {
            _selectedSentries.push_back(*((*itVertex)->coords));
        }
    }

    AlgorithmBase_updateCanvasAndBlock();
}

SentryPlacement::~SentryPlacement()
{
    delete _triangulation;
}

void SentryPlacement::initTriangulation()
{
    qDebug() << "--------------------------------------------------------------------------";
    qDebug() << "Tacke koriscene u ovoj triangulaciji, navedene u smeru kazaljke na satu.";
    for(auto roomVertexIterator = _roomCorners.cbegin(); roomVertexIterator != _roomCorners.end(); ++roomVertexIterator)
    {
        qDebug()<< (*roomVertexIterator).x() << (*roomVertexIterator).y();
    }
    qDebug() << "--------------------------------------------------------------------------";

    // Inicijalizujemo triangulaciju trouglom koji ima jedno pravo teme (sa leksikografski najvećim koordinatama)
    // i dva imaginarna (beskonacno udaljena) temena
    //
    this->initDcell();

    qDebug() << "Triangulacija zapocinje temenom (" << _triangulation->lvReal->x() << "," << _triangulation->lvReal->y() << ")";

    // Temena treba da se dodaju u krajnju triangulaciju slučajnim redosledom.
    // Međutim, zbog iscrtavanja, potrebna nam je kopija niza koja neće biti izmešana.
    //
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

    _verticesToInsert = _roomCorners;
    std::shuffle(_verticesToInsert.begin(), _verticesToInsert.end(), std::default_random_engine(seed));

    // U triangulaciju je već dodato ekstremno teme, tako da njega moramo izbaciti.
    //
    // _verticesToInsert.erase(std::remove(_verticesToInsert.begin(), _verticesToInsert.end(), *(_triangulation->lvReal)));
    _verticesToInsert.insert(_verticesToInsert.begin(), QPoint(0, _boundingPointY));
    _verticesToInsert.insert(_verticesToInsert.begin(), QPoint(0,0));
    _verticesToInsert.insert(_verticesToInsert.begin(), QPoint(_boundingPointX,0));

    // Inicijalizujemo strukturu za pretragu prvim poljem koje je dodato u spisak polja triangulacije,
    // a to je ograniceno polje.
    //
    _triangleLookupTreeRoot = new TriangleLookupNode((_triangulation->_fields)[0]);

    // Dodajemo povratnu referencu na lookupNode iz trougla.
    //
    _triangulation->_fields[0]->lookupNode = _triangleLookupTreeRoot;
}

int SentryPlacementUtils::getChildIndex(
    TriangleLookupNode* currentNode,
    QPoint* newPoint)
{
    if(triangleContainsPoint(currentNode->children[0], newPoint))
    {
        return 0;
    }

    if(triangleContainsPoint(currentNode->children[1], newPoint))
    {
        return 1;
    }

    if(triangleContainsPoint(currentNode->children[2], newPoint))
    {
        return 2;
    }

    return -1;
}

bool SentryPlacementUtils::triangleContainsPoint(
    TriangleLookupNode* currentNode,
    QPoint* newPoint)
{
    QPoint *a = currentNode->vertices[0];
    QPoint *b = currentNode->vertices[1];
    QPoint *c = currentNode->vertices[2];

    // Tacka je unutar trougla, ukoliko pripada bilo kojoj od ivica trougla.
    // Razmatraju se samo ivice koje nisu razapete izmedju simbolickih tacaka,
    // zato sto su simbolicke tacke i odabrane tako, da sve tacke koje se razmatraju budu strogo unutar ivica trougla.
    //
    if((!SentryPlacementUtils::isSymbolic(a) && !SentryPlacementUtils::isSymbolic(b) && utils::isBetween(*a, *b, *newPoint))
        || (!SentryPlacementUtils::isSymbolic(b) && !SentryPlacementUtils::isSymbolic(c) && utils::isBetween(*b, *c, *newPoint))
        || (!SentryPlacementUtils::isSymbolic(c) && !SentryPlacementUtils::isSymbolic(a) && utils::isBetween(*c, *a, *newPoint)))
    {
        qDebug() << "Tacka (" << newPoint->x() << "," << newPoint->y() << ") se nalazi na jednoj od ivica trougla.";
        return true;
    }

    //Tačka je unutar trougla, ukoliko se za svaku orijentisanu duž trougla tačka nalazi sa leve strane.
    //
    bool leftOfab = onTheLeftSideOfAB(newPoint, a, b);
    bool leftOfbc = onTheLeftSideOfAB(newPoint, b, c);
    bool leftOfca = onTheLeftSideOfAB(newPoint, c, a);

    bool allLeft = leftOfab && leftOfbc && leftOfca;
    qDebug() << "Tacka (" << newPoint->x() << "," << newPoint->y() << ") "
             << (allLeft? "" : "ne") << "pripada trouglu [("
             << a->x() << "," << a->y() << "), ("
             << b->x() << "," << b->y() << "), ("
             << c->x() << "," << c->y() << ")]";

    return allLeft;
}

bool SentryPlacementUtils::onTheLeftSideOfAB(
    QPoint* pointToCheck,
    QPoint* pointA,
    QPoint* pointB
)
{
    // Ukoliko su obe tačke simboličke, onda je tražena tačka sa leve strane,
    // ako je redosled navođenja p_2, p_1.
    if(isSymbolic(pointA) && isSymbolic(pointB))
    {
        return isP2(pointA) && isP1(pointB);
    }

    if(isP1(pointB))
    {
        return lexGreater(pointToCheck, pointA);
    }

    if(isP2(pointA))
    {
        return lexGreater(pointToCheck, pointB);
    }

    if(isP2(pointB))
    {
        return !lexGreater(pointToCheck, pointA);
    }

    if(isP1(pointA))
    {
        return !lexGreater(pointToCheck, pointB);
    }

    return counterClockwise(pointToCheck, pointA, pointB);
}

bool SentryPlacementUtils::counterClockwise(QPoint *p1, QPoint *p2, QPoint *p3)
{
   return ((p2->x() - p1->x())*(p3->y() - p1->y()) - (p3->x() - p1->x())*(p2->y() - p1->y())) > 0;
}

SentryHalfEdge* SentryPlacementUtils::onTheTriangleEdge(const SentryVertex *n, const SentryField *triangle)
{
    SentryHalfEdge *currentEdge = triangle->innerComponent;

    // Brojac održavamo kao zaštitu od beskonačne petlje, u slučaju lošeg ulančavanja.
    //
    int edgesVisited = 0;

    do
    {
        if(SentryPlacementUtils::edgeContainsVertex(currentEdge, n))
        {
            return currentEdge;
        }

        currentEdge = currentEdge->next;
        ++edgesVisited;
    }
    while(currentEdge != triangle->innerComponent && edgesVisited < 3);

    // Posto nijedna od ivica nije vratila rezultat, pretpostavljamo da se teme ne nalazi ni na jednoj od ivica.
    //
    return nullptr;
}

bool SentryPlacementUtils::edgeContainsVertex(const SentryHalfEdge *edge, const SentryVertex *vertex)
{
    return utils::isBetween(*(edge->origin->coords), *(edge->twin->origin->coords), *(vertex->coords));
}

bool SentryPlacementUtils::isSymbolic(
    SentryVertex *pVertex
)
{
    if(pVertex == nullptr || pVertex->coords == nullptr)
    {
        throw "Nullpointer exception";
    }

    return isSymbolic(pVertex->coords);
}

bool SentryPlacementUtils::isSymbolic(
    QPoint *pPoint
)
{
    int x = pPoint->x();
    int y = pPoint->y();
    return x == y && (x == P1_POINT_COORDS || x == P2_POINT_COORDS);
}

bool SentryPlacementUtils::isP1(
    const QPoint *pPoint
)
{
    int x = pPoint->x();
    int y = pPoint->y();
    return x == y && x == P1_POINT_COORDS;
}

bool SentryPlacementUtils::isP2(
        const QPoint *pPoint)
{
    int x = pPoint->x();
    int y = pPoint->y();
    return x == y && x == P2_POINT_COORDS;
}

QPoint SentryPlacementUtils::makeP1Symbolic()
{
    return QPoint(P1_POINT_COORDS, P1_POINT_COORDS);
}

QPoint SentryPlacementUtils::makeP2Symbolic()
{
    return QPoint(P2_POINT_COORDS, P2_POINT_COORDS);
}

bool SentryPlacementUtils::lexGreater(
    SentryVertex* a,
    SentryVertex* b)
{
    return lexGreater(a->coords, b->coords);
}

bool SentryPlacementUtils::lexGreater(
    const QPoint* a,
    const QPoint* b)
{
    return a->y() > b->y() || (a->y() == b->y() && a->x() > b->x());
}

bool SentryPlacementUtils::alreadyLegal(
    SentryVertex* pNewVertex,
    SentryHalfEdge *pEdgeToFlip)
{    
    // Ako je sused beskonačno polje, onda ne postoji potreba za legalizacijom ivice
    // - ovakve ivice su uvek legalne.
    //
    if(pEdgeToFlip->twin->incidentFace->outerComponent == nullptr)
    {
        qDebug() << "Ivica je legalna, zato sto je sused beskonacno polje.";
        return true;
    }

    SentryVertex *i = pEdgeToFlip->origin;
    SentryVertex *j = pEdgeToFlip->next->origin;
    SentryVertex *k = pEdgeToFlip->twin->next->next->origin;

    bool thereAreSymbolicVertices =
        isSymbolic(i)
        || isSymbolic(j)
        || isSymbolic(k);

    // Ukolko je neka od tacaka simbolicka, preskacemo legalizaciju.
    // Problem je u tome sto postoji slucaj, koji je tesko detektovati, u kojem dolazi do degeneracije ivica.
    //
    if(thereAreSymbolicVertices)
    {
        return true;
    }

    // Ukoliko se temena i,j nalaze sa iste strane prave kl, onda to znaci da bi ukidanje ivice ij dovelo do nepravilnog trougla.
    // Zato proglasavamo ivicu legalnom.
    //
    if(onTheLeftSideOfAB(i->coords, k->coords, pNewVertex->coords) == onTheLeftSideOfAB(j->coords, k->coords, pNewVertex->coords))
    {
        qDebug() << "Ivica [" << *i << "," << *j << "] je legalna, zato što bi njeno ukidanje dovelo do degenerisanja mreže";
        return true;
    }

    // Ukoliko nema simboličkih temena među temenima datih ivica i
    // ukoliko se novo teme nalazi van kruga opisanog oko trougla koje čine preostala tri temena,
    // onda je posmatrana ivica već legalna i nema potrebe za modifikacijom.
    //
    if(!thereAreSymbolicVertices
        && utils::pointOutsideOfCircumcircle(
            i->coords,
            j->coords,
            k->coords,
            pNewVertex->coords))
    {
        qDebug() << "Ivica je legalna po definiciji, posto bismo legalizacijom dobili ruzniji cetvorougao.";
        return true;
    }

    // Inače, proveravamo slučajeve kada je barem neko od temena simboličko.
    // Tada treba preskočiti legalizaciju ivice akko je min(i,j) < min(k,l),
    // gde su i,j indeksi temena ivice koju želimo potencijalno da legalizujemo,
    // a k,l indeksi potencijalne nove ivice koju treba dodati.
    // U knjizi Computational geometry, na strani 204, moguće je videti detaljniji opis zašto ovakva procedura ima smisla.
    // Budući da je u konkretnoj implementaciji vrednost X koordinate simboličkih tačaka postavljena na -1 i -2,
    // to je dovoljno proveriti da li postoji barem neka simbolička tačka, i vršiti operaciju poređenja nad X koordinatama.
    //
    Q_ASSERT(P1_POINT_COORDS == -1);
    Q_ASSERT(P2_POINT_COORDS == -2);

    if(thereAreSymbolicVertices
        && (std::min(
                pNewVertex->coords->x(),
                k->coords->x())
            < std::min(
                i->coords->x(),
                j->coords->x())))
    {
        qDebug() << "Ivica je legalna prema definiciji legalnosti za ivice koje sadrže imaginarne tačke.";
        return true;
    }

    return false;
}

QPoint SentryPlacementUtils::normalizeCoords(const QPoint *p, const int highestPointY)
{
    if(isP1(p))
    {
        return QPoint(30000, -30000);
    }

    if(isP2(p))
    {
        return QPoint(-30000, highestPointY + 10);
    }

    return QPoint(p->x(), p->y());
}

QColor SentryPlacementUtils::getQtColor(SentryVertex *pv)
{
    switch(pv->color)
    {
    case SentryVertexColoring::Color::White:
        return Qt::white;
    case SentryVertexColoring::Color::Red:
        return Qt::red;
    case SentryVertexColoring::Color::Green:
        return Qt::green;
    case SentryVertexColoring::Color::Blue:
        return Qt::blue;
    }

    return Qt::black;
}

bool SentryPlacementUtils::colorize(SentryField *pField)
{
    SentryVertex *currentVertex = SentryPlacementUtils::getUncoloredVertex(pField);
    if(nullptr == currentVertex)
    {
        return false;
    }

    while(currentVertex != nullptr)
    {
        currentVertex->color = SentryPlacementUtils::getUnusedColor(pField);
        currentVertex = SentryPlacementUtils::getUncoloredVertex(pField);
    }

    return true;
}

SentryVertexColoring::Color SentryPlacementUtils::getUnusedColor(SentryField *pField)
{
    int colorCounts[5] = {0};
    ++colorCounts[(int)pField->innerComponent->origin->color];
    ++colorCounts[(int)pField->innerComponent->next->origin->color];
    ++colorCounts[(int)pField->innerComponent->next->next->origin->color];

    if(0 == colorCounts[1])
    {
        return SentryVertexColoring::Color::Red;
    }

    if(0 == colorCounts[2])
    {
        return SentryVertexColoring::Color::Green;
    }

    return SentryVertexColoring::Color::Blue;
}

SentryVertex *SentryPlacementUtils::getUncoloredVertex(SentryField *pField)
{
    SentryHalfEdge *currentEdge = pField->innerComponent;
    if(currentEdge->origin->color == SentryVertexColoring::Color::White)
    {
        return currentEdge->origin;
    }

    if(currentEdge->next->origin->color == SentryVertexColoring::Color::White)
    {
        return currentEdge->next->origin;
    }

    if(currentEdge->next->next->origin->color == SentryVertexColoring::Color::White)
    {
        return currentEdge->next->next->origin;
    }

    return nullptr;
}

void SentryPlacement::legalizeEdges(SentryVertex *newPoint, std::stack<SentryHalfEdge*> &refEdgesToLegalize)
{
    while(!(refEdgesToLegalize.empty()))
    {
        SentryHalfEdge *edgeToFlip = refEdgesToLegalize.top();
        refEdgesToLegalize.pop();

        if(SentryPlacementUtils::alreadyLegal(newPoint, edgeToFlip))
        {
            qDebug() << "Vršimo potencijalnu legalizaciju ivice [" << *(edgeToFlip->origin) << "," << *(edgeToFlip->twin->origin) << "]"
                     << "u odnosu na teme" << *newPoint;
            continue;
        }

        // Inače, znači da dijagonala četvorougla ikjl koju trenutno posmatramo prestaje da bude legalna dodavanjem nove tačke,
        // te vršimo legalizaciju ivice.

        // Imenujemo sve trenutne ivice, radi lakšeg praćenja rada algoritma.
        //
        SentryHalfEdge *ij = edgeToFlip;
        SentryHalfEdge *ji = edgeToFlip->twin;
        SentryHalfEdge *jl = ij->next;
        SentryHalfEdge *li = jl->next;
        SentryHalfEdge *ik = ji->next;
        SentryHalfEdge *kj = ik->next;

        // Polja zahvacena promenom, koja treba ukloniti iz triangulacije.
        //
        SentryField *f1 = ij->incidentFace;
        SentryField *f2 = ji->incidentFace;

        // Imenujemo temena koja učestvuju u legalizaciji, radi lakšeg praćenja algoritma.
        //
        SentryVertex *l = li->origin;
        SentryVertex *i = ik->origin;
        SentryVertex *k = kj->origin;
        SentryVertex *j = jl->origin;

        qDebug() << "Ivica [ "<< *i << "," << *j <<"]"
                 << " nije legalna u odnosu na temena "
                 << *l << " i " << *k;

        // Vrsimo prikaz na ekranu parova ivica koji ucestvuju u legalizaciji.
        //
        _edgeToHighlight = ij;
        _newlyAddedEdgeVertexX = l;
        _newlyAddedEdgeVertexY = k;
        AlgorithmBase_updateCanvasAndBlock();

        // Nakon zavrsenog prikaza, anuliramo ove podatke,
        // u slucaju da nisu potrebni za sledeci korak algoritma.
        //
        _edgeToHighlight = nullptr;
        _newlyAddedEdgeVertexX = nullptr;
        _newlyAddedEdgeVertexY = nullptr;

        // Kreiramo dve nove ivice koje ce povezivati suprotne čvorove.
        //
        SentryHalfEdge* lk = new SentryHalfEdge();
        SentryHalfEdge* kl = new SentryHalfEdge();

        // Kreiramo dva nova polja koja nastaju prevezivanjem.
        //
        SentryField* f3 = new SentryField();
        SentryField* f4 = new SentryField();

        // Postavljamo origin novih ivica.
        //
        lk->origin = l;
        kl->origin = k;

        // Postavljamo blizance novih ivica
        //
        kl->twin = lk;
        lk->twin = kl;

        // Postavljamo pripadajući trougao na pravu vrednost.
        //
        kl->incidentFace = f3;
        li->incidentFace = f3;
        ik->incidentFace = f3;
        lk->incidentFace = f4;
        kj->incidentFace = f4;
        jl->incidentFace = f4;

        // Povezujemo novoformirane ivice sa predjašnjima.
        //
        lk->next = kj;
        kj->next = jl;
        jl->next = lk;
        kl->next = li;
        li->next = ik;
        ik->next = kl;

        // Postavljamo unutrašnju ivicu dve novoformirane oblasti na dve novoformirane ivice.
        //
        f3->innerComponent = kl;
        f4->innerComponent = lk;

        // Postavljamo spoljašnju ivicu dve novoformirane oblasti na dve novoformirane ivice.
        //
        f3->outerComponent = lk;
        f4->outerComponent = kl;

        // Kreiramo dva nova lookup čvora.
        //
        TriangleLookupNode* f3Lookup = new TriangleLookupNode(f3);
        TriangleLookupNode* f4Lookup = new TriangleLookupNode(f4);

        // Dodajemo povratnu vezu iz trougla na lookup.
        //
        f3->lookupNode = f3Lookup;
        f4->lookupNode = f4Lookup;

        // Dodajemo lookup čvorove u stablo kao decu prethodnih.
        //
        f1->lookupNode->children[0] = f3Lookup;
        f1->lookupNode->children[1] = f4Lookup;
        f2->lookupNode->children[0] = f3Lookup;
        f2->lookupNode->children[1] = f4Lookup;

        // Uklanjamo stara polja i ivice iz triangulacije.
        //
        _triangulation->deleteField(f1);
        _triangulation->deleteField(f2);
        _triangulation->deleteEdge(ij);
        _triangulation->deleteEdge(ji);

        // Dodajemo nove ivice u triangulaciju.
        //
        _triangulation->_edges.push_back(lk);
        _triangulation->_edges.push_back(kl);

        // Dodajemo nova polja u triangulaciju.
        //
        _triangulation->_fields.push_back(f3);
        _triangulation->_fields.push_back(f4);

        // Vrsimo osvezavanje prikaza, nakon zavrsenog prevezivanja ivice.
        //
        AlgorithmBase_updateCanvasAndBlock();

        // Na stek za legalizaciju dodajemo nove dve ivice koje mozda vise nisu legalne.
        //
        refEdgesToLegalize.push(ik);
        refEdgesToLegalize.push(kj);
    }
}

void SentryPlacement::drawRoom(QPainter& painter) const
{
    if(0 == _roomCorners.size())
    {
        return;
    }

    QPen pen = QPen(Qt::black);
    pen.setWidth(3);
    painter.setPen(pen);
    painter.drawPolygon(_roomCorners.data(), _roomCorners.size());

    painter.setBrush(QBrush(Qt::white));
    painter.drawPolygon(_roomCorners.data(), _roomCorners.size());
}

void SentryPlacement::drawSentries(QPainter &painter) const
{
    QPen outerLinePen(Qt::black);
    outerLinePen.setWidth(2);
    painter.setPen(outerLinePen);
    painter.setBrush(QBrush(Qt::yellow));

    foreach(QPoint sentry, _selectedSentries)
    {
        painter.drawEllipse(sentry, SENTRY_DIAMETER, SENTRY_DIAMETER);
    }
}

void SentryPlacement::drawEdgeToHighlight(QPainter& painter) const
{
    if(nullptr == _edgeToHighlight || nullptr == _newlyAddedEdgeVertexX || nullptr == _newlyAddedEdgeVertexY)
    {
        return;
    }

    QPen thickRedPen = QPen(Qt::red);
    thickRedPen.setStyle(Qt::PenStyle::DashLine);
    thickRedPen.setWidth(2);
    painter.setPen(thickRedPen);
    painter.drawLine(SentryPlacementUtils::normalizeCoords(_edgeToHighlight->origin->coords, this->_triangulation->lvReal->y()), SentryPlacementUtils::normalizeCoords(_edgeToHighlight->twin->origin->coords,this->_triangulation->lvReal->y()));

    QPen thickGreenPen = QPen(Qt::green);
    thickGreenPen.setStyle(Qt::PenStyle::DashLine);
    thickGreenPen.setWidth(2);
    painter.setPen(thickGreenPen);
    painter.drawLine(SentryPlacementUtils::normalizeCoords(_newlyAddedEdgeVertexX->coords,this->_triangulation->lvReal->y()), SentryPlacementUtils::normalizeCoords(_newlyAddedEdgeVertexY->coords,this->_triangulation->lvReal->y()));
}

void SentryPlacement::drawSentriesCountText(QPainter& painter) const
{
    QPoint sentriesTextAnchor(30, 30);

    if(_timedOut)
    {
        painter.setPen(QPen(Qt::red));
    }
    else
    {
        painter.setPen(QPen(Qt::black));
    }

    if(_completed)
    {
        QFont font = painter.font();
        font.setPointSize(font.pointSize() * 2);
        font.setBold(true);
        painter.setFont(font);
    }

    painter.drawText(sentriesTextAnchor, QString("Broj postavljenih čuvara: %1.").arg(_selectedSentries.size()));
}

void SentryPlacement::drawTriangulationLines(QPainter &painter) const
{
    if(nullptr != _triangulation)
    {
        QPen linesPen(Qt::magenta);
        linesPen.setStyle(Qt::PenStyle::DashLine);
        linesPen.setWidth(1);
        QPen normalPen(Qt::black);

        for(auto it = _triangulation->_edges.begin(); it != _triangulation->_edges.end(); it++)
        {
            QPoint *pA = (*it)->origin->coords;
            QPoint *pB = (*it)->next->origin->coords;

            QPoint a = SentryPlacementUtils::normalizeCoords(pA,this->_triangulation->lvReal->y());
            QPoint b = SentryPlacementUtils::normalizeCoords(pB,this->_triangulation->lvReal->y());

            bool isAsymbolic = isSymbolicOrBounding(pA);
            bool isBsymbolic = isSymbolicOrBounding(pB);

            if(!isAsymbolic)
            {
                painter.setPen(normalPen);
                painter.setBrush(QBrush(SentryPlacementUtils::getQtColor((*it)->origin)));
                painter.drawEllipse(a, VERTEX_DIAMETER, VERTEX_DIAMETER);
            }

            if(!isBsymbolic)
            {
                painter.setPen(normalPen);
                painter.setBrush(SentryPlacementUtils::getQtColor((*it)->next->origin));
                painter.drawEllipse(b, VERTEX_DIAMETER, VERTEX_DIAMETER);
            }

            if(_includeSymbolicLines || (!isAsymbolic && !isBsymbolic))
            {
                painter.setPen(linesPen);
                painter.drawLine(a, b);
            }
        }
    }
}

void SentryPlacement::drawCurrentPoint(QPainter& painter) const
{
    if(nullptr == this->_currentPoint)
    {
        return;
    }

    painter.setBrush(Qt::green);
    painter.drawEllipse(*(this->_currentPoint), VERTEX_DIAMETER, VERTEX_DIAMETER);
}

void SentryPlacement::drawCurrentLookupNode(QPainter &painter) const
{
    if(nullptr == this->_currentLookupNode)
    {
        return;
    }

    painter.setPen(QPen(Qt::cyan,Qt::PenStyle::DotLine));
    QPoint a = SentryPlacementUtils::normalizeCoords(_currentLookupNode->vertices[0],this->_triangulation->lvReal->y());
    QPoint b = SentryPlacementUtils::normalizeCoords(_currentLookupNode->vertices[1],this->_triangulation->lvReal->y());
    QPoint c = SentryPlacementUtils::normalizeCoords(_currentLookupNode->vertices[2],this->_triangulation->lvReal->y());
    painter.drawLine(a,b);
    painter.drawLine(a,c);
    painter.drawLine(b,c);
}

QPoint SentryPlacement::getRandomSentry()
{
    QPoint randomSentry;

    // Petlja koja se ponavlja sve dok se ne generise tacka koja se nalazi unutar poligona,
    // a da nije vec odabrana ranije.
    do
    {
        std::clock_t startTicks = std::clock();
        randomSentry = getNextRandomPoint();
        std::clock_t endTicks = std::clock();
        _consumedSeconds += double(endTicks - startTicks) / CLOCKS_PER_SEC;
        if(_consumedSeconds > _timeoutSeconds)
        {
            // U slucaju da je vreme prekoraceno, vraca se nelegalna tacka,
            // tako da pozivajuci metod bude svestan da izracunavanje nije zavrseno.
            //
            return SentryPlacementUtils::makeP1Symbolic();
        }
    }
    while(
          !this -> AlgorithmBase::fastPolyContains(_roomCorners, randomSentry)
          || std::find(_selectedSentries.begin(), _selectedSentries.end(), randomSentry) != _selectedSentries.end());

    return randomSentry;
}

void SentryPlacement::updateVisibleCorners(const QPoint& sentry, std::set<QPoint, CmpQPoint>& visibleCorners)
{
    for(unsigned cornerInd = 0; cornerInd < _roomCorners.size(); cornerInd++)
    {
        QPoint corner = _roomCorners.at(cornerInd);
        if(visibleCorners.find(corner) != visibleCorners.end())
        {
            continue;
        }

        bool isCornerVisible = true;

        for(unsigned indx = 0; indx < _roomCorners.size(); indx++)
        {
            if(cornerInd == indx || cornerInd == (indx + 1) % _roomCorners.size())
            {
                continue;
            }

            std::clock_t startTicks = std::clock();
            if(utils::segmentIntersection((QPointF)sentry, (QPointF)corner, (QPointF)_roomCorners[indx], (QPointF)_roomCorners[(indx + 1) % _roomCorners.size()], nullptr))
            {
                isCornerVisible = false;
                break;
            }
            std::clock_t endTicks = std::clock();
            _consumedSeconds += double(endTicks - startTicks) / CLOCKS_PER_SEC;
            if(_consumedSeconds > _timeoutSeconds)
            {
                _completed = true;
                return;
            }
        }

        if(isCornerVisible)
        {
            visibleCorners.insert(corner);
        }
    }
}

void SentryPlacement::clearRunData()
{
    if(nullptr != _triangulation)
    {
        delete _triangulation;
        _triangulation = nullptr;
    }

    _timedOut = false;
    _includeSymbolicLines = true;

    _selectedSentries.clear();
    _randomPointsCache.clear();
    _roomCorners.clear();
    _verticesToInsert.clear();

    _consumedSeconds = 0.0;
    _randomPointsCacheSize = STARTING_RANDOM_POINTS_CACHE_SIZE;

    _edgeToHighlight = nullptr;
    _newlyAddedEdgeVertexX = nullptr;
    _newlyAddedEdgeVertexY = nullptr;
    _currentPoint = nullptr;
    _currentLookupNode = nullptr;
    _triangleLookupTreeRoot = nullptr;

    _boundingPointX = nullptr == _pRenderer? NO_RENDERER_MAX_X : _pRenderer->width();
    _boundingPointY = nullptr == _pRenderer? NO_RENDERER_MAX_Y : _pRenderer->height();
}

QPoint SentryPlacement::getNextRandomPoint()
{
    QPoint pointToReturn;
    if(_randomPointsCache.size() == 0)
    {
        _randomPointsCacheSize *= 2 ;
        _randomPointsCache = generateRandomPoints(_randomPointsCacheSize + 1);
    }

    pointToReturn = _randomPointsCache.back();
    _randomPointsCache.pop_back();
    return pointToReturn;
}
