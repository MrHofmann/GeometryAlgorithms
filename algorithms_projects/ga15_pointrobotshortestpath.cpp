#include "ga15_pointrobotshortestpath.h"
#include <queue>
#include <QLine>
#include <fstream>

PointRobotShortestPath::PointRobotShortestPath(QWidget *pRender, int delayMs, std::string fileName, searchType search, QPoint start, QPoint end)
    : AlgorithmBase(pRender, delayMs), _obstacles(fileName), _search(search), _pStart(start), _pEnd(end),
      _eventQueue(), _statusQueue(StatusQueueCompare(&_sweepLine)), _searchQueue(SearchQueueCompare(&_search))
{
    if(start != end)
    {
        _pStart = start;
        _pEnd = end;
    }
    else if(fileName.find("ga15_obstacles00.off") != std::string::npos)
    {
        _pStart = QPoint(90, 330);
        _pEnd = QPoint(550, 200);
    }
    else if(fileName.find("ga15_obstacles01.off") != std::string::npos)
    {
        _pStart = QPoint(0, 0);
        _pEnd = QPoint(950,400);
    }
    else if(fileName.find("ga15_obstacles02.off") != std::string::npos)
    {
        _pStart = QPoint(90, 200);
        _pEnd = QPoint(600, 190);
    }
    else if(fileName.find("ga15_obstacles03") != std::string::npos)
    {
        _pStart = QPoint(400, 200);
        _pEnd = QPoint(600, 200);
    }
    else if(fileName.find("ga15_obstacles04") != std::string::npos)
    {
        _pStart = QPoint(50, 250);
        _pEnd = QPoint(970, 200);
    }
    else
    {
        _pStart = QPoint(0, 0);
        _pEnd = QPoint(1000, 500);
    }

    //INICIJALIZACIJA GRAFA VIDLJIVOSTI O(nlogn)
    std::vector<DCELVertex*> vertices = _obstacles.vertices();
    for(unsigned i=0; i<vertices.size(); i++) //O(n)
    {
        DCELVertex *v1 = vertices[i];
        DCELVertex *v2 = v1->incidentEdge()->next()->origin();
        QPoint p1 = v1->coordinates();
        QPoint p2 = v2->coordinates();

        _lineSegments.push_back(QLine(p1, p2));
        _visibilityGraph.addVertex(p1, v1); //O(logn)
    }
    _visibilityGraph.addVertex(_pStart, nullptr);
    _visibilityGraph.addVertex(_pEnd, nullptr);
}

PointRobotShortestPath::PointRobotShortestPath(QWidget *pRender, int delayMs, std::string fileName, searchType search, int size)
    : AlgorithmBase(pRender, delayMs), _obstacles(generateObstacleGrid(size, size)), _search(search),
      _eventQueue(), _statusQueue(StatusQueueCompare(&_sweepLine)), _searchQueue(SearchQueueCompare(&_search))
{
    _pStart = QPoint(10, 35);
    _pEnd = QPoint(485, 485);

    //INICIJALIZACIJA GRAFA VIDLJIVOSTI O(nlogn)
    std::vector<DCELVertex*> vertices = _obstacles.vertices();
    for(unsigned i=0; i<vertices.size(); i++) //O(n)
    {
        DCELVertex *v1 = vertices[i];
        DCELVertex *v2 = v1->incidentEdge()->next()->origin();
        QPoint p1 = v1->coordinates();
        QPoint p2 = v2->coordinates();

        _lineSegments.push_back(QLine(p1, p2));
        _visibilityGraph.addVertex(p1, v1); //O(logn)
    }
    _visibilityGraph.addVertex(_pStart, nullptr);
    _visibilityGraph.addVertex(_pEnd, nullptr);
}


void PointRobotShortestPath::runAlgorithm()
{
    if(pointInsidePolygon(_pStart))
        std::cout << "START POINT IS INSIDE OBSTACLE" << std::endl;
    else if(pointInsidePolygon(_pEnd))
        std::cout << "END POINT IS INSIDE OBSTACLE" << std::endl;
    else
    {
        improvedVisibilityGraph();
        naiveShortestPath();

        _eventQueue.clear();
        _statusQueue.clear();
    }

    emit animationFinished();
}

void PointRobotShortestPath::drawAlgorithm(QPainter &painter) const
{
    painter.fillRect(0, 0, 1000, 600, Qt::SolidPattern);

    QPen pen = painter.pen();
    pen.setColor(Qt::red);
    pen.setWidth(5);

    painter.setPen(pen);

    painter.drawPoint(_pStart);
    painter.drawPoint(_pEnd);

    for(int i=0; i<_obstacles.edges().size(); i+=2)
    {
        painter.drawLine(_obstacles.edges()[i]->origin()->coordinates(),
                         _obstacles.edges()[i]->next()->origin()->coordinates());
    }

    pen.setWidth(0.5);
    pen.setColor(Qt::green);
    painter.setPen(pen);

    painter.drawLine(_sweepLine);

    std::map<QPoint, vertex*> verts = _visibilityGraph.vertices();
    std::map<QPoint, vertex*>::iterator it;
    for(it=verts.begin(); it!=verts.end(); it++)
    {
        std::vector<std::pair<double, vertex*> > edges = it->second->_edges;
        std::vector<std::pair<double, vertex*> >::iterator jt;
        for(jt=edges.begin(); jt!=edges.end(); jt++)
        {
            painter.drawLine(it->second->_coordinates, jt->second->_coordinates);
        }
    }

    pen.setWidth(2);
    pen.setColor(Qt::yellow);
    painter.setPen(pen);
    for(it=verts.begin(); it!=verts.end(); it++)
        if(it->second->_visited)
        {
            painter.drawPoint(it->second->_coordinates);
            if(it->second->_prev[0] != nullptr)
                painter.drawLine(it->second->_coordinates, it->second->_prev[0]->_coordinates);
        }

    pen.setWidth(5);
    pen.setColor(Qt::blue);
    painter.setPen(pen);
    for(unsigned i=0; i<_shortestPath.size(); i++)
        if(_shortestPath[i]->_prev[0] != nullptr)
            painter.drawLine(_shortestPath[i]->_coordinates, _shortestPath[i]->_prev[0]->_coordinates);

    painter.drawPoint(_pStart);
    painter.drawPoint(_pEnd);
}

void PointRobotShortestPath::runNaiveAlgorithm()
{
        naiveVisibilityGraph();
        naiveShortestPath();
}


//IZRACUNAVANJE GRAFA VIDLJIVOSTI (O(n^3))
void PointRobotShortestPath::naiveVisibilityGraph()
{
    std::map<QPoint, vertex*> vertices = _visibilityGraph.vertices();
    for(auto it = vertices.begin(); it!=vertices.end(); it++)
    {
        for(auto jt = std::next(it); jt!=vertices.end(); jt++)
        {
            bool add = true;
            QPoint p1 = it->second->_coordinates;
            QPoint p2 = jt->second->_coordinates;

            //PROVERA DA LI DUZ p1p2 SECE UNUTRASJOST PREPREKE LOKALNO KOD p1
            vertex *v = it->second;
            if(v->_obstacleVert != nullptr)
            {
                QPoint vp = v->_obstacleVert->incidentEdge()->prev()->origin()->coordinates();
                QPoint vn = v->_obstacleVert->incidentEdge()->next()->origin()->coordinates();

                double angle1 = atan2(p1.y() - vp.y(), p1.x() - vp.x());
                double angle2 = atan2(p1.y() - vn.y(), p1.x() - vn.x());
                double angle3 = atan2(p1.y() - p2.y(), p1.x() - p2.x());

                if(angle1 > angle2)
                {
                    if(angle3 > angle2 && angle3 < angle1)
                        add = false;
                }
                else if(angle1 < angle2)
                {
                    if(!(angle3 >= angle1 && angle3 <= angle2))
                        add = false;
                }
            }

            std::vector<DCELHalfEdge*> edges = _obstacles.edges();
            for(int i=0; i<edges.size(); i+=2)
            {
                QPointF in;
                DCELHalfEdge *e = edges[i];

                QPoint q1 = e->origin()->coordinates();
                QPoint q2 = e->twin()->origin()->coordinates();

                //AKO JE p1p2 IVICA PREPREKE
                if((p1 == q1 && p2 == q2) || (p1 == q2 && p2 == q1))
                {
                    _visibilityGraph.addEdge(it->first, jt->first, utils::distance2(p1, p2));
                    _visibilityGraph.addEdge(jt->first, it->first, utils::distance2(p2, p1));
                    AlgorithmBase_updateCanvasAndBlock();
                    add = false;
                    break;
                }
                if (p1 == q1 || p2 == q1 || p1 == q2 || p2 == q2)
                    continue;
                if(utils::lineIntersection2(QLineF(p1, p2), QLineF(q1, q2), &in))
                {
                    //AKO DUZ p1p2 SECE NEKA IVICA PREPREKE ALI NE U TEMENU
                    if(p1 != q1 && p2 != q1 && p1 != q2 && p2 != q2)
                    {
                        if(in != q1 && in != q2)
                        {
                            int o1 = utils::orientation(p1, p2, q1);
                            int o2 = utils::orientation(p1, p2, q2);
                            if(o1 != 0 && o2 != 0)
                            {
                                add = false;
                                break;
                            }
                        }
                        else if(in == q1)
                        {
                            QPoint q0 = e->prev()->origin()->coordinates();
                            int o1 = utils::orientation(p1, p2, q0);
                            int o2 = utils::orientation(p1, p2, q2);

                            if(o1 != o2 && o1 != 0 && o2 != 0)
                            {
                                add = false;
                                break;
                            }
                        }
                        else if(in == q2)
                        {
                            QPoint q3 = e->next()->next()->origin()->coordinates();
                            int o1 = utils::orientation(p1, p2, q1);
                            int o2 = utils::orientation(p1, p2, q3);

                            if(o1 != o2 && o1 != 0 && o2 != 0)
                            {
                                add = false;
                                break;
                            }
                        }
                    }
                }
            }

            if(add)
            {
                _visibilityGraph.addEdge(it->first, jt->first, utils::distance2(p1, p2));
                _visibilityGraph.addEdge(jt->first, it->first, utils::distance2(p2, p1));
                AlgorithmBase_updateCanvasAndBlock();
            }
        }
    }
}
//IZRACUNAVANJE NAJKRACEG PUTA IZMEDJU DVE TACKE (O(n^2))
void PointRobotShortestPath::naiveShortestPath()
{
    //INCIJALIZACIJA REDA CVOROVA I VREDNOSTI HEURISTIKE
    std::map<QPoint, vertex*> queue = _visibilityGraph.vertices();
    for(auto it=queue.begin(); it!=queue.end(); it++)
        it->second->_hx = utils::distance(it->second->_coordinates, _pEnd);

    queue.find(_pStart)->second->_dist = 0;
    while(!queue.empty())
    {
        //U ZAVISNOSTI OD TIPA PRETRAGE IZABERI SLEDECI CVOR KOJI SE OBRADJUJE
        auto it = queue.begin();
        if(_search == DIJKSTRA)
        {
            for(auto jt=queue.begin(); jt!=queue.end(); jt++)
                if(jt->second->_visited == false  &&  jt->second->_dist < it->second->_dist)
                    it = jt;
        }
        else if(_search == A_STAR)
        {
            for(auto jt=queue.begin(); jt!=queue.end(); jt++)
                if(jt->second->_visited == false  &&  jt->second->_dist+jt->second->_hx < it->second->_dist+it->second->_hx)
                    it = jt;
        }

        //ZA SVAKOG SUSEDA AKO JE POTREBNO AZURIRAJ TRENUTNI NAJKRACI PUT
        for(unsigned i=0; i<it->second->_edges.size(); i++)
        {
            double alt = it->second->_edges[i].first;
            vertex *v = it->second->_edges[i].second;
            if(v->_visited == false)
            {
                alt += it->second->_dist;
                if(alt < v->_dist)
                {
                    v->_dist = alt;
                    v->_prev.clear();
                    v->_prev.push_back(it->second);
                }
                else if(alt == v->_dist)
                    v->_prev.push_back(it->second);
            }
        }

        //OZNACI TEKUCI CVOR I OBRISI GA IZ REDA
        it->second->_visited = true;
        AlgorithmBase_updateCanvasAndBlock();
        if(it->second->_coordinates == _pEnd)
            break;
        else
            queue.erase(it);
    }

    //KONSTRUKCIJA NAJKRACEG PUTA
    vertex *v = queue.find(_pEnd)->second;
    if(v->_prev[0] != nullptr)
    {
        do
        {
            _shortestPath.push_back(v);
            v = v->_prev[0];
            AlgorithmBase_updateCanvasAndBlock();
        }while(v != nullptr);
    }
}

void PointRobotShortestPath::improvedVisibilityGraph()
{
    //NAPREDNI ALGORITAM ZA RACUNANJE GRAFA VIDLJIVOSTI                             O(n^2*logn)
    //--------------------------------------------------------------------------------------------
    std::map<QPoint, vertex*> vertices = _visibilityGraph.vertices();
    for(auto it=vertices.begin(); it!=vertices.end(); it++) //..........................O(n)
    {
        bool prevVis = false;
        vertex* prevVert = nullptr;
        std::vector<vertex *> visibleVertices;

        //ODREDJIVANJE INICIJALNOG POLOZAJA BRISUCE PRAVE
        //-------------------------------------------------------------------------------------
        QPoint p1 = it->second->_coordinates;
        _sweepLine = QLine(p1.x(), p1.y(), 10000, p1.y());
        AlgorithmBase_updateCanvasAndBlock();
        //-------------------------------------------------------------------------------------

        //SORTIRANJE TACAKA DOGADJAJA PO UGLU SA BRISUCOM PRAVOM                    O(nlogn)
        //-------------------------------------------------------------------------------------
        initDataQueues(it->second);
        //-------------------------------------------------------------------------------------


        //ODREDJIVANJE SVIH VIDLJIVIH CVOROVA IZ TEKUCEG CVORA                      O(nlogn)
        //-------------------------------------------------------------------------------------
        while(!_eventQueue.empty())//...............................................O(n)
        {
            EventQueueVertex eventVertex = *_eventQueue.begin();
            _eventQueue.erase(_eventQueue.begin()); //..................................O(logn)

            QPoint p2 = eventVertex.event;
            vertex *v = eventVertex.v;

            updateSweepLine(p1, p2);
//            AlgorithmBase_updateCanvasAndBlock();

            //U SLUCAJU TEMENA PREPREKE STATUS SE AZURIRA...............................O(logn)
            if(v->_obstacleVert != nullptr)
                updateStatusQueue(v);

            //PROVERA VIDLJIVOSTI TEMENA p2 IZ p1.......................................O(logn)
            if(isVisible(it->second, v, prevVert, prevVis))
            {
                double weight = utils::distance(p1, p2);
                _visibilityGraph.addEdge(p1, p2, weight);
                visibleVertices.push_back(v);
                prevVis = true;
                AlgorithmBase_updateCanvasAndBlock();
            }
            else
                prevVis = false;

            prevVert = v;
        }
        _sweepLine = QLine(0, 0, 0, 0);
        //-------------------------------------------------------------------------------------
    }
    //--------------------------------------------------------------------------------------------

}
//NE RADI JER std::priority_queue NE MOZE DA ODRZAVA PRIORITET I AZURNOST U ISTO VREME
void PointRobotShortestPath::improvedShortestPath()
{
    std::map<QPoint, vertex*> vertices = _visibilityGraph.vertices();
    for(auto it=vertices.begin(); it!=vertices.end(); it++)
    {
        it->second->_hx = utils::distance(it->second->_coordinates, _pEnd);
        _searchQueue.push(it->second);
    }

    vertices.find(_pStart)->second->_dist = 0;
    while(!_searchQueue.empty())//O(n)
    {
        vertex *v = _searchQueue.top();
        _searchQueue.pop();                     //O(logn) ZA std::set
        for(unsigned i=0; i<v->_edges.size(); i++)//O(n)
        {
            double alt = v->_edges[i].first;
            vertex *w = v->_edges[i].second;
            if(w->_visited == false)
            {
                alt += v->_dist;
                if(alt < w->_dist)
                {
                    w->_dist = alt;
                    w->_prev.clear();
                    w->_prev.push_back(v);
                    //PRISUP I BRSANJE ELEMENTA + UBACIVANJE U RED ZA std::set O(logn)
                }
                else if(alt == w->_dist)
                    w->_prev.push_back(v);
            }
        }

        v->_visited = true;
        AlgorithmBase_updateCanvasAndBlock();
        if(v->_coordinates == _pEnd)
            break;
    }

    std::cout << std::endl << "SHORTEST PATH:" << std::endl;
    vertex *v = vertices.find(_pEnd)->second;
    unsigned len = 0;
    do
    {
        if(v->_prev[0] != nullptr)
            len += v->_dist - v->_prev[0]->_dist;

        std::cout << v->_v << std::endl;
        _shortestPath.push_back(v);
        AlgorithmBase_updateCanvasAndBlock();
        v = v->_prev[0];
    }while(v != nullptr);
    std::cout << "length: " << len << std::endl;
}


void PointRobotShortestPath::updateSweepLine(const QPoint p1, const QPoint p2)
{
    QPoint p3;
    if(p1.x() == p2.x())
    {
        if(p1.y() < p2.y())
            p3 = QPoint(p1.x(), 1000);
        else
            p3 = QPoint(p1.x(), -1000);
    }
    else
    {
        double k = (double)(p2.y()-p1.y())/(p2.x()-p1.x());
        double n = p2.y()-k*p2.x();

        if(p1.x() < p2.x())
            p3 = QPoint(1000, 1000*k + n);
        else
            p3 = QPoint(-1000, -1000*k + n);
    }
    _sweepLine = QLine(p1, p3);
}

void PointRobotShortestPath::initDataQueues(vertex *w)
{
    QPoint p1 = w->_coordinates;
    std::map<QPoint, vertex*> vertices = _visibilityGraph.vertices();
    auto it = vertices.find(p1);

    _eventQueue.clear();
    _statusQueue.clear();
    for(auto jt=vertices.begin(); jt!=vertices.end(); jt++)//......................O(n)
    {
        if(jt != it)
        {
            QPoint p0 = jt->first;
            vertex *v = jt->second;
            _eventQueue.insert({p0, v, _sweepLine});//.......................O(logn)
        }
    }

    //BRISUCA PRAVA OBILAZI JEDAN KRUG, OVAJ DEO JE NEOPHODAN PRE NEGO STO KRENE ALGORITAM
    for(auto jt=_eventQueue.begin(); jt!=_eventQueue.end(); jt++)
    {
        EventQueueVertex eventVertex = *jt;
        QPoint p2 = eventVertex.event;
        vertex *v = eventVertex.v;
        updateSweepLine(p1, p2);

        //U SLUCAJU TEMENA PREPREKE STATUS SE AZURIRA...............................O(logn)
        if(v->_obstacleVert != nullptr)
            updateStatusQueue(v);
    }
}

void PointRobotShortestPath::updateStatusQueue(vertex *v)
{
    QPoint p = v->_coordinates;

    if(v->_obstacleVert->incidentEdge()->prev() == nullptr)
        std::cout << v->_coordinates.x() << ", " << v->_coordinates.y()
                  << " previous edge is null" << std::endl;
    else
    {
        QPoint pp = v->_obstacleVert->incidentEdge()->prev()->origin()->coordinates();

        //AKO JE PRETHODNO TEME U SMERU KAZALJKE NA SATU ONDA DODAJ IVICU, INACE BRISI
        if(utils::orientation(_sweepLine.p1(), _sweepLine.p2(), pp) < 0)
            _statusQueue.insert(QLineF(pp, p));
        else
            deleteFromStatus(QLine(pp, p));
    }

    if(v->_obstacleVert->incidentEdge()->next() == nullptr)
        std::cout << v->_coordinates.x() << ", " << v->_coordinates.y()
                  << " next edge is null" << std::endl;
    else
    {
        QPoint pn = v->_obstacleVert->incidentEdge()->next()->origin()->coordinates();

        //AKO JE SLEDECE TEME U SMERU KAZALJKE NA SATU ONDA DODAJ IVICU, INACE BRISI
        if(utils::orientation(_sweepLine.p1(), _sweepLine.p2(), pn) < 0)
            _statusQueue.insert(QLine(p, pn));
        else
            deleteFromStatus(QLine(p, pn));
    }
}
//NE RADI, BAG JE NEGDE U STATUSOVOM FUNKTORU
void PointRobotShortestPath::deleteFromStatus2(QLine l)
{
    //CUVANJE TRENUTNOG POLOZAJA BRISUCE PRAVE
    QLine sweepTmp = _sweepLine;
    if(utils::orientation(_sweepLine.p1(), _sweepLine.p2(), l.p1()) > 0)
        updateSweepLine(_sweepLine.p1(), l.p1());
    else
        updateSweepLine(_sweepLine.p1(), l.p2());

    //AKO SE DUZ SA TRAZENIM PRESEKOM NALAZI U STATUSU
    auto it = _statusQueue.find(l);
    if(it != _statusQueue.end())
    {
        //AKO JE TO UPRAVO TRAZENA DUZ ONDA BRISI, INACE PROVERI PRETHODNU I SLEDECU
        auto itn = std::next(it);
        if(*it == l)
        {
            _statusQueue.erase(it);
        }
        else if(it != _statusQueue.begin())
        {
            auto itp = std::prev(it);
            if(*itp == l)
            {
                _statusQueue.erase(itp);
            }
            else if(itn != _statusQueue.end())
            {
                if(*itn == l)
                    _statusQueue.erase(itn);
            }
        }
        else if(itn != _statusQueue.end())
        {
            if(*itn == l)
                _statusQueue.erase(itn);
        }
        else
            std::cout << "STRANICA NE POSTOJI U STATUSU 1" << std::endl;
    }
    else
        std::cout << "STRANICA NE POSTOJI U STATUSU 2" << std::endl;    

    updateSweepLine(sweepTmp.p1(), sweepTmp.p2());
}
//RADI U LINEARNOM VREMENU, POTREBNO JE LOGARITAMSKO
void PointRobotShortestPath::deleteFromStatus(QLine l)
{
    for(auto it = _statusQueue.begin(); it != _statusQueue.end(); ++it)
    {
        if(*it == l)
        {
            _statusQueue.erase(it);
            break;
        }
    }
}
//RADI, ZAVISI OD ORIJENTACIJE IVICA
bool PointRobotShortestPath::isVisible(const vertex *v, const vertex *w, vertex *prevVert, bool prevVis)
{
    //AKO SE PRETHODNO TEME/DOGADJAJ NE NALAZI NA ISTOJ PRAVOJ KAO I TEKUCE, ILI
    //TEKUCE TEME/DOGADJAJ JE PRVI U NIZU TADA AKO SE p0,p2 I PRVA DUZ U STATUSU
    //SEKU ONDA p2 NIJE VIDLJIVO IZ p1 INACE JESU

    //AKO SE TEKUCE TEME p2 I PRETHODNO TEME p1 NALAZE NA ISTOJ POLUPRAVOJ,
    //TADA AKO PRETHODNO TEME NIJE VIDLJIVO ONDA NIJE NI TEKUCE A
    //INACE AKO DUZ p1,p2 SECE NEKU DUZ IZ STATUSA
    //ONDA TEKUCE TEME p2 NIJE VIDLJIVO INACE JESTE

    QPoint p0 = v->_coordinates;
    QPoint p2 = w->_coordinates;

    if(_statusQueue.empty())
        return true;

    //AKO DUZ vw SECE UNUTRASNJOST POLIGONA LOKALNO KOD v
    if(v->_obstacleVert != nullptr)
    {
        QPoint vp = v->_obstacleVert->incidentEdge()->prev()->origin()->coordinates();
        QPoint vn = v->_obstacleVert->incidentEdge()->next()->origin()->coordinates();

        double angle1 = atan2(p0.y() - vp.y(), p0.x() - vp.x());
        double angle2 = atan2(p0.y() - vn.y(), p0.x() - vn.x());
        double angle3 = atan2(p0.y() - p2.y(), p0.x() - p2.x());

        if(angle1 > angle2)
        {
            if(angle3 > angle2 && angle3 < angle1)
                return false;
        }
        else if(angle1 < angle2)
        {
            if(!(angle3 >= angle1 && angle3 <= angle2))
                return false;
        }
    }

    //AKO JE TEKUCI PRVI DOGADJAJ U NIZU ILI DOGADJAJI NISU KOLINEARNI
    if(prevVert == nullptr || utils::orientation(_sweepLine.p1(), _sweepLine.p2(), prevVert->_coordinates) != 0)
    {
        QPointF i;
        QLineF l1 = QLine(p0, p2);
        QLineF l2 = *_statusQueue.begin();
        if(utils::lineIntersection2(l1, l2, &i) && i!=p2)
            return false;
        else
            return true;
    }
    //AKO SU OBA NA BRISUCOJ PRAVOJ
    else
    {
        //AKO PRETHDONI NIJE VIDLJIV
        if(prevVis == false)
            return false;

        //AKO JE PRETHODNI TEME PREPREKE
        if(prevVert->_obstacleVert != nullptr)
        {
            QPoint pp = prevVert->_obstacleVert->incidentEdge()->prev()->origin()->coordinates();
            QPoint pn = prevVert->_obstacleVert->incidentEdge()->next()->origin()->coordinates();

            int o1 = utils::orientation(p0, p2, pp);
            int o2 = utils::orientation(p0, p2, pn);

            if(o1 != o2 && o1 != 0 && o2 != 0)
                return false;
        }

        QPoint p1 = prevVert->_coordinates;
        QPointF ip;
        QPointF in;
        bool ret = true;

        auto l = _statusQueue.insert(QLine(p1, p2));
        auto ln = std::next(l);
        if(l != _statusQueue.begin())
        {
            auto lp = std::prev(l);
            if(utils::lineIntersection2(*l, *lp, &ip) && ip != p1 && ip != p2)
                ret = false;
        }
        if(ln != _statusQueue.end())
        {
            if(utils::lineIntersection2(*l, *ln, &in) && in != p1 && in != p2)
                ret = false;
        }

        _statusQueue.erase(l);
        return ret;
    }
}

std::string PointRobotShortestPath::generateObstacleGrid(unsigned rows, unsigned cols) const
{
    std::string path = "ga15_obstacleGrid.off";
    std::ofstream out(path);
    out << "OFF" << std::endl << std::endl;
    out << rows*cols*4 << " " << rows*cols << " " << rows*cols*4 << std::endl << std::endl;

    double squareSize = 0.1;
    double distance = 0.1;
    for(unsigned i=0; i<rows; i++)
    {
        for(unsigned j=0; j<cols; j++)
        {
            double v_x, v_y;

            v_x = -1 + (squareSize + distance)*j;
            v_y = -1 + (squareSize + distance)*i;
            out << " " << v_x << " " << v_y << " " << 0 << std::endl;

            v_x = -1 + (squareSize + distance)*j + squareSize;
            v_y = -1 + (squareSize + distance)*i;
            out << " " << v_x << " " << v_y << " " << 0 << std::endl;

            v_x = -1 + (squareSize + distance)*j + squareSize;
            v_y = -1 + (squareSize + distance)*i + squareSize;
            out << " " << v_x << " " << v_y << " " << 0 << std::endl;

            v_x = -1 + (squareSize + distance)*j;
            v_y = -1 + (squareSize + distance)*i + squareSize;
            out << " " << v_x << " " << v_y << " " << 0 << std::endl << std::endl;
        }

        out << std::endl << std::endl << std::endl << std::endl;
    }

    for(unsigned i=0; i<rows*cols*4; i++)
    {
        if(i%4 == 0)
            out << 4  << " ";

        out << i << " ";

        if((i+1)%4 == 0)
            out << std::endl;
    }

    return path;
}


std::vector<vertex *> PointRobotShortestPath::shortestPath() const
{
    return _shortestPath;
}

double PointRobotShortestPath::shortestPathLength() const
{
    if(_shortestPath.size() == 0)
        return 0;

    double length = 0;
    for(unsigned i=0; i<_shortestPath.size()-1; i++)
        length += utils::distance2(_shortestPath[i]->_coordinates, _shortestPath[i+1]->_coordinates);

    return length;
}

//NE RADI ZA POLIGONE SA DIJAGONALAMA, RADI U LINEARNOM VREMENU.
bool PointRobotShortestPath::pointInsidePolygon(QPointF p1)
{
    unsigned intersecs = 0;
    std::vector<DCELVertex*> vertices = _obstacles.vertices();
    for(unsigned i=0; i<vertices.size(); i++)
    {
        DCELHalfEdge *e = vertices[i]->incidentEdge();
        QPointF q1 = e->origin()->coordinates();
        QPointF q2 = e->next()->origin()->coordinates();
        QPointF p2 = QPointF(100000, p1.y());
        QPointF in;

        if(utils::lineContainsPoint2(QLineF(q1, q2), p1))
            return false;

        QLineF l = QLineF(p1, p2);
        if(utils::lineIntersection2(l, QLineF(q1, q2), &in))
        {
            QPoint q3 = e->prev()->origin()->coordinates();
            QPoint q4 = e->next()->next()->origin()->coordinates();
            if(utils::lineContainsPoint2(l, q1) && utils::lineContainsPoint2(l, q2))
            {
                int o1 = utils::orientation(l.p1(), l.p2(), q3);
                int o2 = utils::orientation(l.p1(), l.p2(), q4);
                if(o1 != o2 && o1 != 0 && o2 != 0)
                    intersecs++;
            }
            else if(in == q1)
            {
                int o1 = utils::orientation(l.p1(), l.p2(), q2);
                int o2 = utils::orientation(l.p1(), l.p2(), q3);
                if(o1 != o2 && o1 != 0 && o2 != 0)
                    intersecs++;
            }
            else if(in == q2)
            {
                int o1 = utils::orientation(l.p1(), l.p2(), q1);
                int o2 = utils::orientation(l.p1(), l.p2(), q4);
                if(o1 != o2 && o1 != 0 && o2 != 0)
                    intersecs++;
            }
            else
                intersecs++;
        }
    }

    return intersecs%2 != 0;
}


/*********************************************************************
 *                         DATASTRUCTURES                            *
 *********************************************************************/
bool operator < (QPointF p1, QPointF p2)
{
    return (p1.x()==p2.x())? (p1.y()<p2.y()):(p1.x()<p2.x());
}

bool operator < (QLineF l1, QLineF l2)
{
    return (l1.p1() == l2.p1())? (l1.p2()<l2.p2()):(l1.p1()<l2.p1());
}


Graph::Graph()
{

}

void Graph::addVertex(const QPoint &p, DCELVertex *obstacle)
{
    std::map<QPoint, vertex*>::iterator i = _vertices.find(p);
    if(i == _vertices.end())
    {
        vertex *v = new vertex(p, obstacle);
        _vertices[p] = v;
    }
    else
        std::cout << "Vertex " << i->second->_v << " already exists." << std::endl;
}

void Graph::addEdge(QPoint from, QPoint to, double weight)
{
    vertex *f = (_vertices.find(from)->second);
    vertex *t = (_vertices.find(to)->second);
    std::pair<double, vertex*> edge = std::make_pair(weight, t);
    f->_edges.push_back(edge);
}

std::map<QPoint, vertex *> Graph::vertices() const
{
    return _vertices;
}
