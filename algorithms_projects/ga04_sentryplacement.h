/*
Autor: Bozidar Radivojevic, 1024/2015
Godina: 2018.
Kratak opis problema:
    Potrebno je u datom prostoru oblika prostog poligona odrediti raspored cuvara,
    tako da svaki deo prostora bude u vidnom polju barem jednog cuvara.
*/

#ifndef GA04_SENTRYPLACEMENT_H
#define GA04_SENTRYPLACEMENT_H

#include "algorithmbase.h"
#include "utils.h"
#include <random>
#include <set>
#include <stack>
#include <ctime>
#include <chrono>

// Deklaracije medjuzavisnih struktura
//
struct SentryField;
struct SentryVertex;
struct SentryHalfEdge;

///
/// \brief The CmpQPoint struct
///  - koristi se za poredjenje QPoint tačaka prema koordinatama.
///
struct CmpQPoint
{
    bool operator()(const QPoint& lhs, const QPoint& rhs) const
    {
        return (lhs.x() < rhs.x() || (lhs.x() >= rhs.x() && lhs.y() < rhs.y()));
    }
};

///
/// \brief The TriangleLookupNode struct
/// Element drveta za čuvanje i pretragu podataka o odnosu između trouglova triangulacije.
/// Svaki list predstavlja jedan trenutno aktuelni trougao u triangulaciji,
/// dok unutrašnji čvorovi predstavljaju "put" koji je pređen od početnog sveobuhvatnog trougla
/// do trenutnog trougla, omogućavaujući efikasnu pretragu vlasnika sledeće tačke koja treba da bude uneta.
///
struct TriangleLookupNode
{
    ///
    /// \brief triangle
    /// Trougao triangulacije na koji se odnosi trenutni čvor.
    /// Ovu promenljivu je bezbedno koristiti, jedino ako je u pitanju list,
    /// zato što se inače unutrašnje ivice ovog polja prevezuju.
    /// Za potrebe geometrijskog odredjivanja polozaja, moguce je koristiti niz koordinata vertices,
    /// na koji ni u kom smislu ne uticu promene u mrezi triangulacije.
    ///
    SentryField *triangle;

    ///
    /// \brief vertices
    /// Koordinate temena trougla.
    /// Čuvaju se radi direktnijeg iscrtavanja komponenti.
    ///
    QPoint* vertices[3];

    ///
    /// \brief children
    /// Pokazivači na decu, ukoliko postoje.
    /// Broj dece moze biti 0, 2 ili 3, ukoliko je čvor list,
    /// ili je podeljen na dva nova trougla tačkom sa ivice,
    /// ili je podeljen na tri nova trougla tačkom iz unutrašnjosti.
    ///
    TriangleLookupNode* children[3];

    ///
    /// \brief TriangleLookupNode
    /// Konstruktor klase.
    ///
    /// \param triangle
    /// Trougao u triangulaciji na koji se odnosi trenutni čvor.
    ///
    TriangleLookupNode(SentryField* triangle);

    /// \brief ~TriangleLookupNode
    /// Destruktor klase.
    /// Ovaj destruktor ne unistava nijedan od objekata, zato sto se podrazumeva
    /// da je odgovornost za unistavanje SentryField objekta na DCELL strukturi
    /// kojoj to polje pripada.
    ///
    ~TriangleLookupNode()
    {}
};

namespace SentryVertexColoring
{
    ///
    /// Enumeracija boje za upotrebu u tri-bojenju algoritma.
    /// Osim tri definisane boje, potrebno nam je i "neodredjeno" stanje, pa je ukupan broj definisanih boja 4.
    /// Vrednosti boja su stepeni dvojke, radi lakšeg izračunavanja u algoritmu tribojenja.
    ///
    enum Color
    {
        White = 0,
        Red = 1,
        Green = 2,
        Blue = 4
    };
}

///
/// \brief The SentryVertex struct
/// Struktura koja omogućava čuvanje informacija o čvoru (odnosno ćošku) prostorije.
///
struct SentryVertex
{
    ///
    /// \brief coords
    /// Pokazivač na objekat sa koordinatama čvora u ravni iscrtavanja.
    ///
    QPoint* coords;

    ///
    /// \brief color
    /// Trenutna boja temena.
    /// Do završetka triangulacije, sva temena imaju belu boju.
    ///
    SentryVertexColoring::Color color;

    ///
    /// \brief SentryVertex
    /// Konstruktor strukture.
    ///
    /// \param coords
    /// Koordinate čvora u ravni iscrtavanja.
    ///
    SentryVertex(QPoint* coords)
    {
        this->coords = new QPoint(coords->x(), coords->y());
        this->color = SentryVertexColoring::Color::White;
    }

    ///
    /// Destruktor klase.
    ///
    ~SentryVertex()
    {
        delete coords;
    }

    ///
    /// \brief operator QString
    /// Operator konverzije u QString.
    /// Potreban je za svrhe debagovanja i testiranja koda,
    /// da bi mogla biti izvršena integracija sa QDebug() mehanizmom.
    ///
    operator QString() const;
};

///
/// \brief The SentryHalfEdge struct
/// Struktura koja omogućava čuvanje informacija o polu-ivici triangulacije.
///
struct SentryHalfEdge
{
    ///
    /// \brief origin
    /// Pokazivač na čvor sa kojim počinje ova polu-ivica.
    ///
    SentryVertex *origin;

    ///
    /// \brief twin
    /// Pokazivač na polu-ivicu sa kojom ova ivica čini celokupnu ivicu.
    ///
    SentryHalfEdge *twin;

    ///
    /// \brief incidentFace
    /// Pokazivač na polje kojem je data polu-ivica unutrašnja polu-ivica.
    ///
    SentryField *incidentFace;

    ///
    /// \brief next
    /// Pokazivač na sledeću unutrašnju polu ivicu
    /// Pokazivač na prethodnu nije potreban, zato što se u ovom slučaju uvek radi isključivo o trouglovima,
    /// pa u najgorem slučaju imamo potrebu da dva puta potražimo "next".
    ///
    SentryHalfEdge *next;

    SentryHalfEdge()
        : origin(nullptr),
          twin(nullptr),
          incidentFace(nullptr),
          next(nullptr)
    {}
};

///
/// \brief The SentryField struct
/// Struktura koja omogućava čuvanje informacija o poljima (trouglovima) triangulacije.
struct SentryField
{
    ///
    /// \brief outerComponent
    /// Pokazivač na neku spoljnu polu-ivicu.
    /// U specijalnom slučaju neograničene oblasti, ovaj pokazivač je jednak nullptr.
    ///
    SentryHalfEdge *outerComponent;

    ///
    /// \brief innerComponent
    /// Pokazivač na untrašnju polu-ivicu.
    /// U opštem slučaju, potrebno je imati vektor koji čuva podatke o ivicama "rupa",
    /// međutim, za ovaj problem, tako nešto nije potrebno jer soba nema "rupe", prema definiciji problema.
    ///
    SentryHalfEdge *innerComponent;

    ///
    /// \brief lookupNode
    /// Pokazivač na triangle lookup node u stablu kojim se lociraju trouglovi.
    /// Ovaj pokazivač omogućava brzu komunikaciju između stabla i DCELL strukture.
    /// Pri destrukciji polja, unistava se i ovaj objekat, nezavisno od strukture kojoj pripada.
    /// Zato je jako bitno da svakom polju odgovara iskljucivo jedan cvor.
    ///
    TriangleLookupNode *lookupNode;

    ///
    /// \brief SentryField
    /// Konstruktor klase.
    ///
    SentryField()
        :outerComponent(nullptr),innerComponent(nullptr),lookupNode(nullptr)
    {}

    ///
    /// \brief ~SentryField
    /// Destruktor klase.
    ///
    ~SentryField();

    ///
    /// \brief operator QString
    /// Operator konverzije u QString.
    /// Potreban je za svrhe debagovanja i testiranja koda,
    /// da bi mogla biti izvršena integracija sa QDebug() mehanizmom.
    ///
    operator QString() const;
};

///
/// \brief The SentryDcell class
/// Struktura koja objedinjuje DCELL i koja je odgovorna za brisanje elemenata koji joj pripadaju nakon prestanka upotrebe.
///
struct SentryDcell
{
    ///
    /// \brief SentryDcell
    /// Konstruktor DCELL strukture.
    ///
    SentryDcell()
    {}

    ///
    /// \brief ~SentryDcell
    /// Destruktor klase.
    ///
    ~SentryDcell();

    ///
    /// \brief lvReal
    /// Realno teme ogranicavajuceg trougla;
    ///
    const QPoint* lvReal;

    ///
    /// \brief _unlimitedField
    /// Neograničeno polje strukture.
    /// Pošto se ostala polja čuvaju u podeljenim nizovima polja koja trenutno učestvuju u triangulaciji i polja koja su nekada učestvovala,
    /// odvajamo posebnu promeljivu za čuvanje neograničenog polja, koje nam je potrebno tokom cele triangulacije,
    /// ali ne pripada samoj triangulaciji za potrebe obilazaka i bojenja polja.
    ///
    SentryField *_unlimitedField;

    ///
    /// \brief _vertices
    /// Svi čvorovi trenutne triangulacije.
    ///
    std::vector<SentryVertex*> _vertices;

    ///
    /// \brief _edges
    /// Sve ivice trenutne triangulacije.
    ///
    std::vector<SentryHalfEdge*> _edges;

    ///
    /// \brief _fields
    /// Sva polja trenutne triangulacije.
    ///
    std::vector<SentryField*> _fields;

    ///
    /// \brief _deletedEdges
    /// Sve ivice koje su nekada bile deo triangulacije.
    ///
    std::vector<SentryHalfEdge*> _deletedEdges;

    ///
    /// \brief _deletedFields
    /// Sva polja koja su nekada bila deo triangulacije.
    ///
    std::vector<SentryField*> _deletedFields;

    ///
    /// \brief deleteEdge
    /// Prebacuje ivicu iz spiska trenutnih u spisak obrisanih.
    ///
    /// \param edge
    /// Ivica koja treba da se izbriše.
    ///
    void deleteEdge(SentryHalfEdge *edge);

    ///
    /// \brief deleteField
    /// Prebacuje trougao iz spiska trenutnih u spisak obrisanih.
    ///
    /// \param triangle
    /// Trougao koji treba obrisati.
    ///
    void deleteField(SentryField * triangle);

    ///
    /// \brief operator QString
    /// Operator konverzije u QString.
    /// Potreban je za svrhe debagovanja i testiranja koda,
    /// da bi mogla biti izvršena integracija sa QDebug() mehanizmom.
    ///
    operator QString() const;
};

///
/// \brief The SentryPlacementUtils class
/// Pomoćna klasa koja sadrži statičke metode koji su neophodni za izvršavanje algoritma.
/// Izdvojena je kao posebna klasa, da bi omogućila lakše unit-testiranje pojedinih metoda,
/// kao i bolju preglednost međuzavisnosti delova koda.
///
class SentryPlacementUtils
{
    public:
        ///
        /// \brief P1_POINT_COORDS
        /// Vrednost koordinata prve simboličke tačke.
        /// Uvek treba da bude postavljena na vrednost koju ne može imati nijedna druga
        /// "realna" tačka triangulacije.
        ///
        static constexpr int P1_POINT_COORDS = -1;

        ///
        /// \brief P2_POINT_COORDS
        /// Vrednost koordinata prve simboličke tačke.
        /// Uvek treba da bude postavljena na vrednost koju ne može imati nijedna druga
        /// "realna" tačka triangulacije.
        ///
        static constexpr int P2_POINT_COORDS = -2;

        ///
        /// \brief isSymbolic
        /// Metod kojim se proverava da li je dato teme simboličko.
        ///
        /// \param pVertex
        /// Teme koje treba proveriti.
        ///
        /// \return
        /// Tačno ili netačno, u zavisnosti od osobina trenutnog temena.
        ///
        static bool isSymbolic(SentryVertex *pVertex);

        ///
        /// \brief isSymbolic
        /// Metod kojim se proverava da li je tačka koja ima date koordinate simbolička.
        ///
        /// \param pPoint
        /// Koordinate za koje treba izvršiti proveru.
        ///
        /// \return
        /// Tačno ili netačno, u zavisnosti od datih koordinata.
        ///
        static bool isSymbolic(QPoint *pPoint);

        ///
        /// \brief isP1
        /// Proverava da li date koordinate određuju simboličku tačku "P1".
        ///
        /// \param pPoint
        /// Koordinate za koje treba izvršiti proveru.
        ///
        /// \return
        /// Tačno ili netačno, u zavisnosti od datih koordinata.
        ///
        static bool isP1(const QPoint *pPoint);

        ///
        /// \brief isP2
        /// Proverava da li date koordinate određuju simboličku tačku "P2".
        ///
        /// \param pPoint
        /// Koordinate za koje treba izvršiti proveru.
        ///
        /// \return
        /// Tačno ili netačno, u zavisnosti od datih koordinata.
        ///
        static bool isP2(const QPoint *pPoint);

        ///
        /// \brief makeP1Symbolic
        /// Kreira simboličku tačku "P1".
        /// Ovaj metod treba koristiti za kreiranje simboličke tačke,
        /// da bi se izbegla potencijalna buduća nepoklapanja u definisanju fizičke reprezentacije
        /// imaginarnih tačaka.
        ///
        /// \return
        /// Tačka koja može da se koristi u algoritmu.
        ///
        static QPoint makeP1Symbolic();

        ///
        /// \brief makeP2Symbolic
        /// Kreira simboličku tačku "P2".
        /// Ovaj metod treba koristiti za kreiranje simboličke tačke,
        /// da bi se izbegla potencijalna buduća nepoklapanja u definisanju fizičke reprezentacije
        /// imaginarnih tačaka.
        ///
        /// \return
        /// Tačka koja može da se koristi u algoritmu.
        ///
        static QPoint makeP2Symbolic();

        ///
        /// \brief lexGreater
        /// Metod za leksikografsko poređenje koordinata dva data temena.
        ///
        /// \param a
        /// Prvo teme.
        ///
        /// \param b
        /// Drugo teme.
        ///
        /// \return
        /// Tačno, ukoliko je prvo teme leksikografski veće od drugog.
        ///
        static bool lexGreater(SentryVertex *a, SentryVertex *b);

        ///
        /// \brief lexGreater
        /// Metod za leksikografsko poređenje datih koordinata.
        ///
        /// \param a
        /// Prvo teme.
        ///
        /// \param b
        /// Drugo teme.
        ///
        /// \return
        /// Tačno, ukoliko je prvo teme leksikografski veće od drugog.
        ///
        static bool lexGreater(const QPoint *a, const QPoint *b);

        ///
        /// \brief getChildIndex
        /// Metod za određivanje indeksa deteta čvora stabla pretrage,
        /// kojem pripada data tačka.
        ///
        /// \param currentNode
        /// Čvor stabla pretrage.
        ///
        /// \param newPoint
        /// Tačka koja se dodaje u triangulaciju.
        ///
        /// \return
        /// Broj u intervalu [0,2] koji označava indeks deteta datog čvora.
        ///
        static int getChildIndex(TriangleLookupNode *currentNode, QPoint *newPoint);

        ///
        /// \brief triangleContainsPoint
        /// Proverava da li trougao koji trenutno posmatra dati čvor stabla pretrage
        /// sadrži datu tačku.
        ///
        /// \param currentNode
        /// Čvor stabla pretrage za koji se vrši provera.
        ///
        /// \param newPoint
        /// Tačka za koju se vrši provera.
        ///
        /// \return
        /// Vraća tačno, ukoliko se data tačka nalazi u trouglu definisanom čvorom stabla pretrage
        /// (uključujući i ivice datog trougla).
        ///
        static bool triangleContainsPoint(TriangleLookupNode *currentNode, QPoint *newPoint);

        ///
        /// \brief onTheLeftSideOfAB
        /// Metod koji određuje da li se data tačka nalazi sa leve strane usmerene duži AB.
        /// Ovaj metod nije deo neke od zajedničkih klasa, zato što pokriva i slučajeve operacija sa simboličkim tačkama.
        ///
        /// \param pointToCheck
        /// Tačka za koju treba izvršiti proveru.
        ///
        /// \param pointA
        /// Prva tačka usmerene duži.
        ///
        /// \param pointB
        /// Druga tačka usmerene duži.
        ///
        /// \return
        /// Tačno, ukoliko su svi uslovi ispunjeni.
        ///
        static bool onTheLeftSideOfAB(QPoint *pointToCheck, QPoint *pointA, QPoint *pointB);

        ///
        /// \brief alreadyLegal
        /// Proverava da li je data ivica već legalna u odnosu na četvorougao definisan njenim temenima,
        /// datim temenom i njegovim suprotnim temenom.
        ///
        /// \param newVertex
        /// Teme koje je novo dodato u triangulaciju, i koje je moglo da ugrozi legalnost date ivice.
        ///
        /// \param edgeToFlip
        /// Ivica čiju legalnost treba proveriti.
        ///
        /// \return
        /// Tačno, ukoliko je ivica već legalna prema pravilima algoritma.
        ///
        static bool alreadyLegal(SentryVertex *newVertex, SentryHalfEdge *edgeToFlip);

        ///
        /// \brief counterClockwise
        /// Proverava da li je data trojka tačaka orijentisana u matematički pozitivnom smeru.
        ///
        /// \param p1
        /// Prva tačka.
        ///
        /// \param p2
        /// Druga tačka.
        ///
        /// \param p3
        /// Treća tačka.
        ///
        /// \return
        /// Tačno, ukoliko je uslov ispunjen.
        ///
        static bool counterClockwise(QPoint *p1, QPoint *p2, QPoint *p3);

        ///
        /// \brief onTheTriangleEdge
        /// Metod koji proverava da li se dato teme nalazi tačno na ivici datog trougla.
        ///
        /// \param n
        /// Novo teme koje se dodaje u triangulaciju.
        ///
        /// \param triangle
        /// Trougao za koji treba proveriti da li se novo teme nalazi na nekoj od njegovih ivica.
        ///
        /// \return
        /// Vraća pokazivač na ivicu kojoj novo teme pripada, ukoliko pripada nekoj ivici.
        /// Vraća nullptr, ukoliko tačka pripada strogoj unutrašnjosti ili spoljašnosti trougla.
        ///
        static SentryHalfEdge *onTheTriangleEdge(const SentryVertex *n, const SentryField *triangle);

        ///
        /// \brief edgeContainsVertex
        /// Proverava da li data ivica triangulacije sadrži dato teme u svojoj unutrašnjosti.
        ///
        /// \param edge
        /// Ivica za koju se vrši provera.
        ///
        /// \param vertex
        /// Teme za koje se vrši provera.
        ///
        /// \return
        /// Tačno, ukoliko je uslov ispunjen.
        ///
        static bool edgeContainsVertex(const SentryHalfEdge *edge, const SentryVertex *vertex);

        ///
        /// \brief normalizeCoords
        /// Pomoćni metod za iscrtavanje imaginarnih tačaka.
        /// U slučaju regularne tačke, vraća njenu kopiju,
        /// dok u slučaju imaginarne tačke vraća tačku koja ima dovoljno velike koordinate da ne ometa crtež.
        ///
        /// \param p
        /// Tačka čije koordinate treba normalizovati.
        ///
        /// \param highestPointY
        /// Najveća Y koordinata koja se trenutno nalazi na crtežu.
        ///
        /// \return
        /// Kopiju tačke sa odgovarajućim osobinama.
        ///
        static QPoint normalizeCoords(const QPoint *p, const int highestPointY);

        ///
        /// \brief getQtColor
        /// Pomoćni metod za dohvatanje boje iz QT framework-a
        /// koja odgovara boji kojom je obojeno teme triangulacije.
        ///
        /// \param pv
        /// Teme triangulacije.
        ///
        /// \return
        /// Boja kojom je dato teme obojeno.
        ///
        static QColor getQtColor(SentryVertex *pv);

        ///
        /// \brief colorize
        /// Metod koji boji dato polje po principima oportunističkog tri-bojenja.
        /// Sva temena koja su trenutno neobojena, boje se nekom od tri boje,
        /// tako da se izbegnu ponavljanja boja što je više moguće.
        ///
        /// \param pField
        /// Polje koje treba obojiti.
        ///
        /// \return
        /// Vraća tačno, ukoliko je došlo do bojenja barem jednog temena.
        /// Ova povratna vrednost se može koristiti da bi se sprečilo beskonačno kretanje po mreži triangulacije,
        /// već da se dalje obrađuju susedi samo onih temena, kod kojih je bilo makar neke aktivnosti.
        ///
        static bool colorize(SentryField *pField);

        ///
        /// \brief getUnusedColor
        /// Iterira kroz temena datog polja, i nalazi prvu boju koja do sada nije korišćena.
        /// S obzirom da je broj potencijalnih boja tri, a da se radi o trouglovima,
        /// uvek je moguće naći do sada nekorišćenu boju.
        ///
        /// \param pField
        /// Polje u kojem treba naći koja boja do sada nije bila u upotrebi.
        ///
        /// \return
        /// Boja koja do sada nije bila u upotrebi na datom polju.
        ///
        static SentryVertexColoring::Color getUnusedColor(SentryField *pField);

        ///
        /// \brief getUncoloredVertex
        /// Pronalazi prvo teme koje do sada nije obojeno u datom polju.
        ///
        /// \param pField
        /// Polje u kojem treba pronaći neobojeno teme.
        ///
        /// \return
        /// Vraća pokazivač na prvo neobojeno teme ili nullptr ukoliko su sva temena u polju obojena.
        ///
        static SentryVertex *getUncoloredVertex(SentryField *pField);

    private:
        ///
        /// \brief SentryPlacementUtils
        /// Konstruktor klase.
        /// Pošto je namena ove klase da definiše samo statičke metode,
        /// konstruktor je "zabranjen" time što je deklarisan kao privatni i nije definisan.
        ///
        SentryPlacementUtils();
};

///
/// \brief The SentryPlacement class
/// Klasa koja implementira algoritam za postavljanje čuvara.
/// Glavni algoritam se oslanja na kreiranje Delone triangulacije i zatim 3-bojenje dobijenih trouglova
///     Ovaj algoritam ne daje uvek tačno rešenje iz dva razloga:
///     - ivice triangulacije se ne moraju poklapati sa ivicama određenog prostora
///     - tri-bojenje ne mora uvek da postoji, pa je neophodno koristiti "relaksirano" tri-bojenje, u kojem.
/// Naivni algoritam je probabilistički, sa slučajnim postavljanjem čuvara i proverom da li su svi ćoškovi pokriveni.
///     On uvek daje tačno rešenje, ali posle potencijalno beskonačno vremena.
///
class SentryPlacement : public AlgorithmBase
{
public:
    ///
    /// \brief SentryPlacement
    /// Konstruktor klase.
    ///
    /// \param pRenderer
    /// Alat za iscrtvanje.
    ///
    /// \param delayMs
    /// Broj milisekundi čekanja izmedju iteracija u interaktivnom modu.
    ///
    /// \param fileName
    /// Ime ulaznog fajla sa podacima potrebnim algoritmu.
    ///
    /// \param isNaive
    /// Oznaka koja skreće izvršavanja algoritma na "naivnu putanju", ukoliko je postavljena na true.
    ///
    /// \param timeoutSeconds
    /// Maksimalan broj sekundi dat za izvršavanje kritičnih sekcija naivnog algoritma.
    /// Ukoliko je potrošeno više sekundi nego što je dozvoljeno, algoritam se prekida pre završetka.
    ///
    /// \param numberOfPoints
    /// Broj temena (ćoškova) prostorije, ukoliko se od algoritma očekuje da sam izgeneriše slučajan poligon.
    ///
    SentryPlacement(
            QWidget *pRenderer,
            int delayMs,
            std::string fileName,
            bool isNaive,
            double timeoutSeconds,
            int numberOfPoints = DEFAULT_POINTS_NUM);

    ///
    /// \brief ~SentryPlacement
    /// Destruktor klase.
    ///
    ~SentryPlacement();

    ///
    /// \brief MIN_DIMENSION
    /// Najmanji broj tačaka koji je dozvoljen pri testiranju performansi (neinteraktivni mod).
    ///
    static constexpr int MIN_DIMENSION = 4;

    ///
    /// \brief STEP_SIZE
    /// Veličina koraka za ispitivanje performansi u neinteraktivnom modu.
    /// Svaka sledeća iteracija testa će imati ulazne parametre veće za ovu vrednost.
    ///
    static constexpr int STEP_SIZE = 4;

    ///
    /// \brief MAX_DIMENSION
    /// Maksimalna veličina ulaza koja je dozvoljena pri testiranju performansi.
    ///
    static constexpr int MAX_DIMENSION = 200;

    ///
    /// \brief X_MAX_CHART_VAL
    /// Maksimalna vrednost koja se prikazuje na grafikonu za testiranje performansi.
    ///
    static constexpr int X_MAX_CHART_VAL = 200;

    // AlgorithmBase interface
public:
    ///
    /// \brief runAlgorithm
    /// Metod nasleđen iz AlgorithmBase.
    /// Omogućava pokretanje glavnog algoritma.
    /// Konkretna implementacija u ovoj klasi pokreće naivni algoritam,
    /// ukoliko je _isNaive postavljeno na true.
    ///
    void runAlgorithm();

    ///
    /// \brief drawAlgorithm
    /// Metod nasleđen iz AlgorithmBase.
    /// Omogućava iscrtavanje trenutnog stanja u svakoj iteraciji
    /// (tj. pri svakom pozivanju odgovarajućeg metoda za update).
    ///
    /// \param painter
    /// Alat za iscrtavanje.
    ///
    void drawAlgorithm(QPainter &painter) const;

    ///
    /// \brief runNaiveAlgorithm
    /// Metod nasleđen iz AlgorithmBase.
    /// Omogućava pokretanje naivnog algoritma.
    ///
    void runNaiveAlgorithm();

    ///
    /// \brief isCompleted
    /// Vraća vrednost oznake, da li je prethodno pokretanje algoritma izvršeno do kraja bez grešaka.
    /// Ova vrednost se ne poništava na kraju algoritma, da bi bilo moguće izvršiti unit-testiranje koda.
    ///
    bool isCompleted() const;

private:

    ///
    /// \brief SENTRY_DIAMETER
    /// Veličina prikaza čuvara pri iscrtavanju.
    ///
    static const int SENTRY_DIAMETER = 10;

    ///
    /// \brief VERTEX_DIAMETER
    /// Veličina čvora triangulacije pri iscrtavanju.
    ///
    static const int VERTEX_DIAMETER = 5;

    ///
    /// \brief STARTING_RANDOM_POINTS_CACHE_SIZE
    /// Pocetna veličina keša za slučajne tačke.
    /// Pri svakom novom pokretanju algoritma korišćenjem
    /// iste klase treba resetovati keš na početnu veličinu.
    ///
    static const int STARTING_RANDOM_POINTS_CACHE_SIZE = 4;

    ///
    /// \brief _roomCorners
    /// Ulazni parametar - koordinate ćoškova prostorije, navedene redosledom prema susedstvu.
    ///
    std::vector<QPoint> _roomCorners;

    ///
    /// \brief _timeoutSeconds
    /// Maksimalan broj sekundi dozvoljen za izvršavanje kritičnih sekcija naivnog algoritma.
    ///
    const double _timeoutSeconds;

    ///
    /// \brief _isNaive
    /// Ukoliko se postavi na tačno, glavni algoritam će biti preusmeren na naivni algoritam.
    ///
    const bool _isNaive;

    ///
    /// \brief _completed
    /// Označava da je algoritam završen.
    ///
    bool _completed = false;

    ///
    /// \brief _verticesToInsert
    /// Kolekcija koordinata čvorova koje tek treba uneti u triangulaciju.
    ///
    std::vector<QPoint> _verticesToInsert;

    ///
    /// \brief _triangulation
    /// Pokazivač na DCELL strukturu koja čuva trenutno stanje triangulacije.
    ///
    SentryDcell *_triangulation = nullptr;

    ///
    /// \brief _triangleLookupTreeRoot
    /// Pokazivač na drvo pretrage tačaka.
    /// Ova struktura je u tesnoj vezi sa DCELL strukturom.
    ///
    TriangleLookupNode *_triangleLookupTreeRoot = nullptr;

    ///
    /// \brief _currentPoint
    /// Trenutna tačka koja se obrađuje u triangulaciji.
    /// Ovaj podatak se koristi u iscrtavanju.
    ///
    QPoint* _currentPoint = nullptr;

    ///
    /// \brief _edgeToHighlight
    /// Pokazivač na ivicu koja se trenutno obrađuje, i koja treba da bude podebljana.
    /// Ovaj podatak se koristi u iscrtavanju.
    ///
    SentryHalfEdge *_edgeToHighlight = nullptr;

    ///
    /// \brief _newlyAddedEdgeVertexX
    /// Pokazivač na teme novo-dodate ivice triangulacije.
    /// Ovaj podatak se koristi u iscrtavanju.
    ///
    SentryVertex *_newlyAddedEdgeVertexX = nullptr;

    ///
    /// \brief _newlyAddedEdgeVertexY
    /// Pokazivač na teme novo-dodate ivice triangulacije.
    /// Ovaj podatak se koristi u iscrtavanju.
    ///
    SentryVertex *_newlyAddedEdgeVertexY = nullptr;

    ///
    /// \brief _currentLookupNode
    /// Pokazivač na čvor u drvetu pretrage, na kojem smo trenutno pozicionirani.
    /// Ovaj podatak se koristi u iscrtavanju.
    ///
    TriangleLookupNode *_currentLookupNode = nullptr;

    ///
    /// \brief _includeSymbolicLines
    /// U zavisnosti od vrednosti ove promenljive, biće ili neće biti prikazivane
    /// linije koje su razapete izmedju veštačkih (simboličkih ili ograničavajućih) tačaka.
    ///
    bool _includeSymbolicLines = true;

    ///
    /// \brief _boundingPointX
    /// Najveća X koordinata koja može biti prikazana/korišćena u algoritmu.
    ///
    int _boundingPointX = _pRenderer == nullptr? NO_RENDERER_MAX_X : _pRenderer->width();

    ///
    /// \brief _boundingPointY
    /// Najveća Y koordinata koja može biti prikazana/korišćena u algoritmu.
    ///
    int _boundingPointY = _pRenderer == nullptr? NO_RENDERER_MAX_Y : _pRenderer->height();

    ///
    /// \brief _selectedSentries
    /// Lista trenutno odabranih čuvara.
    ///
    std::list<QPoint> _selectedSentries;

    ///
    /// \brief _consumedSeconds
    /// Broj sekundi potrošen tokom izvršavanja algoritma.
    /// Ova metrika je potrebna, radi ograničavanja vremena izvršavanja naivnog algoritma,
    /// koji se može izvršavati potencijalno beskonačno dugo.
    ///
    double _consumedSeconds = 0.0;

    ///
    /// \brief _randomPointsCache
    /// Vektor za čuvanje pre-generisanih slučajnih tačaka.
    /// Radi optimizacije algoritma, slučajne tačke se ne generišu jedna po jedna,
    /// već u većim količinama, i smeštaju se u ovaj vektor.
    /// Vektor se po potrebi uvećava.
    ///
    std::vector<QPoint> _randomPointsCache;

    ///
    /// \brief _randomPointsCacheSize
    /// Početna veličina vektora za smeštanje slučajnih tačaka.
    ///
    int _randomPointsCacheSize = STARTING_RANDOM_POINTS_CACHE_SIZE;

    ///
    /// \brief hasNoImaginaryOrBoundingPoint
    /// Metod koji proverava da li dato polje ima veštačku tačku među svojim temenima.
    /// Koristi se kada je potrebno izbeći neku operaciju u zavisnosti od "imaginarnosti" temena.
    /// Ovaj metod mora biti ne-statički, zato što rezultat koji daje zavisi od trenutne ivice prozora.
    ///
    /// \param pField
    /// Polje za koje je potrebno izvršiti proveru.
    ///
    /// \return
    /// Tačno, ukoliko ne postoji nijedna veštačka tačka među temenima.
    ///
    bool hasNoImaginaryOrBoundingPoint(SentryField *pField);

    ///
    /// \brief initTriangulation
    /// Metod za inicijalizaciju triangulacije, koja čini deo glavnog algoritma.
    /// Alocira osnovne strukture i postavlja invarijante algoritma.
    ///
    void initTriangulation();

    ///
    /// \brief initDcell
    /// Inicijalizuje DCELL strukturu koja se koristi u glavnom algoritmu.
    /// Postavlja prvi sveobuhvatni trougao, koji se sastoji od jedne ekstremne ograničavajuće tačke
    /// i dve imaginarne tačke.
    ///
    void initDcell();

    ///
    /// \brief insertVertexToTriangulation
    /// Ubacuje novo teme u triangulaciju, određujući trougao, odnosno ivicu kojoj novo teme pripada,
    /// a zatim pozivajući odgovarajući metod za konkretno unošenje temena u DCELL i strukturu pretrage.
    ///
    /// \param pNewVertexCoords
    /// Koordinate novog temena koje treba da bude uključeno u triangulaciju.
    ///
    void insertVertexToTriangulation(QPoint *pNewVertexCoords);

    ///
    /// \brief insertVertexOnEdge
    /// Unosi novo teme triangulacije na ivicu postojeće triangulacije.
    /// Ovo se postiže brisanjem dva do tada postojeća trougla, a dodavanjem četiri nova,
    /// tako da se novo teme nađe na preseku dijagonala četvorougla koji je pogođen promenom.
    ///
    /// \param edgeContainingPoint
    /// Ivica koja sadrži novo teme.
    ///
    /// \param n
    /// Novo teme koje treba dodati u triangulaciju.
    ///
    void insertVertexOnEdge(SentryHalfEdge *edgeContainingPoint, SentryVertex *n);

    ///
    /// \brief insertVertexInInterior
    /// Unosi novo teme triangulacije unutar postojećeg polja.
    /// Ovo se postiže uklanjanjem jednog polja i dodavanjem tri nova polja,
    /// tako da se novo teme poveže sa svakim temenom polja kojem pripada.
    ///
    /// \param f
    /// Polje kojem pripada teme.
    ///
    /// \param n
    /// Novo teme koje treba dodati u triangulaciju.
    ///
    void insertVertexInInterior(SentryField *f, SentryVertex *n);

    ///
    /// \brief doThreeColoring
    /// Metod koji vrši "best-effort" tri-bojenje temena triangulacije.
    /// U Delone triangulaciji, budući da nije moguće ispoštovati ivice prostora,
    /// može se desiti da strogo tri-bojenje ne postoji.
    /// Ova varijanta tri-bojenja ne baca nikakvu grešku u slučaju da se u nekom trouglu nađu dva temena iste boje,
    /// već pokušava da oboji što je više temena moguće.
    ///
    void doThreeColoring();

    ///
    /// \brief selectFinalSentries
    /// Metod za odabir čuvara u glavnom algoritmu.
    /// Podrazumeva se da je tri-bojenje obavljeno.
    /// Čuvari se biraju tako da budu svi iste boje, i to one boje koja je najmanje zastupljena.
    /// U slučaju strogog tri-bojenja, dobijamo raspored čuvara koji je svakako dovoljan da pokrije čitav prostor.
    /// U našem tri-bojenju, može da se desi da neki deo prostora ne bude pokriven, zbog sečenja ivica triangulacije
    /// sa ivicama prostora koji se analizira.
    ///
    void selectFinalSentries();

    ///
    /// \brief legalizeEdges
    /// Metod za legalizaciju ivica, koje mogu biti ugrožene dodavanjem novog temena u triangulaciju.
    /// Pri dodavanju novog temena, neke ivice koje su do tada bile legalne, ne moraju više biti.
    /// Po potrebi se izvršava okretanje ivica, tako da se dobije "legalnija" raspodela.
    /// Pri svakom okretanju, nove ivice mogu biti ugrožene, pa se i one obrađuju.
    /// Budući da se sa svakom transformacijom povećava ukupan zbir unutrašnjih uglova trougla triangulacije,
    /// ivice koje treba legalizovati se vremenom iscrpljuju, tako da ne može doći do beskonačne petlje.
    ///
    /// \param newPoint
    /// Tačka koja je poslednja uneta, i koja može ugroziti legalnost ivica triangulacije.
    ///
    /// \param refEdgesToLegalize
    /// Kolekcija ivica koje treba legalizovati.
    /// Metodu se prosleđuje nekoliko ivica koje treba proveriti, a zatim se kolekcija koristi unutar metoda
    /// za dodavanje i analizu i drugih ivica.
    ///
    void legalizeEdges(SentryVertex *newPoint, std::stack<SentryHalfEdge*> &refEdgesToLegalize);

    ///
    /// \brief getFirstNonImaginaryField
    /// Pomoćni metod, korišćen u toku bojenja čvorova.
    /// Dohvata prvo dostupno polje iz triangulacije, čije nijedno teme nije veštačko.
    ///
    /// \return
    /// Pokazivač na prvo polje koje zadovoljava uslove ili nullptr.
    /// Osim u triangulacijama sa veoma malim brojem temena, uvek bi trebalo da postoji makar jedno
    /// polje koje zadovoljava uslov.
    ///
    SentryField *getFirstNonImaginaryField();

    ///
    /// \brief isSymbolicOrBounding
    /// Metod za proveru da li je teme veštačko.
    ///
    /// \param pVertex
    /// Teme koje treba proveriti.
    ///
    /// \return
    /// Tačno ili netačno, u zavisnosti od ispunjenosti uslova.
    ///
    bool isSymbolicOrBounding(SentryVertex *pVertex) const;

    ///
    /// \brief isSymbolicOrBounding
    /// Metod za proveru da li date koordinate mogu pripadati veštačkom temenu.
    ///
    /// \param pPoint
    /// Koordinate za koje treba izvršiti proveru.
    ///
    /// \return
    /// Tačno ili netačno, u zavisnosti od ispunjenosti uslova.
    ///
    bool isSymbolicOrBounding(QPoint *pPoint) const;

    ///
    /// \brief drawSentries
    /// Metod za iscrtavanje odabranih čuvara.
    ///
    /// \param painter
    /// Objekat kojim se vrši iscrtavanje na ekranu.
    ///
    void drawSentries(QPainter &painter) const;

    ///
    /// \brief drawEdgeToHighlight
    /// Metod za iscrtavanje ivice koja se trenutno obrađuje u okviru algoritma legalizacije,
    /// odnosno koju treba obojiti drugom bojom, da bi se bolje videla na ekranu.
    ///
    /// \param painter
    /// Objekat kojim se vrši iscrtavanje na ekranu.
    ///
    void drawEdgeToHighlight(QPainter &painter) const;

    ///
    /// \brief drawSentriesCountText
    /// Metod za iscrtavanje teksta koji prati broj izabranih čuvara.
    ///
    /// \param painter
    /// Objekat kojim se vrši iscrtavanje na ekranu.
    ///
    void drawSentriesCountText(QPainter &painter) const;

    ///
    /// \brief drawRoom
    /// Metod za iscrtavanje ograničenog prostora u kojem treba rasporediti čuvare.
    ///
    /// \param painter
    /// Objekat kojim se vrši iscrtavanje na ekranu.
    ///
    void drawRoom(QPainter &painter) const;

    ///
    /// \brief drawTriangulationLines
    /// Metod za iscrtavanje granica trenutno definisane mreže triangulacije.
    ///
    /// \param painter
    /// Objekat kojim se vrši iscrtavanje na ekranu.
    ///
    void drawTriangulationLines(QPainter &painter) const;

    ///
    /// \brief drawCurrentPoint
    /// Metod za bojenje tačke koja se trenutno obrađuje.
    ///
    /// \param painter
    /// Objekat kojim se vrši iscrtavanje na ekranu.
    ///
    void drawCurrentPoint(QPainter &painter) const;

    ///
    /// \brief drawCurrentLookupNode
    /// Metod za iscrtavanje linija trenutnog trougla u kojem se nalazi tačka koju dodajemo.
    ///
    /// \param painter
    /// Objekat kojim se vrši iscrtavanje na ekranu.
    ///
    void drawCurrentLookupNode(QPainter &painter) const;

    ///
    /// \brief clearRunData
    /// Čisti ostatke prethodnog izvršavanja.
    /// Ovaj metod je neophodan, zato što trenutna implementacija glavnog programa podrazumeva
    /// da se instanca algoritma koristi više puta uzastopno, pa se ne možemo oslanjati na destruktor.
    ///
    void clearRunData();

    ///
    /// \brief getNextRandomPoint
    /// Vraća slučajno odabranu tačku sa koordinatama koje upadaju u opseg iscrtavanja.
    /// \return
    ///
    QPoint getNextRandomPoint();

    ///
    /// \brief getRandomSentry
    /// Metod koji omogućava generisanje slučajnog čuvara unutar datog poligona.
    /// Metod nije pouzdan, u smislu da postoje granični slučajevi kada generisani čuvar nije zapravo u poligonu,
    /// ali je verovatnoća za tako nešto izuzetno mala i ne narušava krajnju korektnost rešenja.
    /// \return
    /// Generisana tačka.
    ///
    QPoint getRandomSentry();

    ///
    /// \brief updateVisibleCorners
    /// Funkcija koja osvežava spisak trenutno vidljivih ćoškova koriscenjem novog stražara u naivnom algoritmu.
    /// \param sentry
    /// Novi stražar.
    /// \param visibleCorners
    /// Spisak trenutno vidljivih čoškova.
    ///
    void updateVisibleCorners(const QPoint& sentry, std::set<QPoint, CmpQPoint> &visibleCorners);
};

#endif // GA04_SENTRYPLACEMENT_H
