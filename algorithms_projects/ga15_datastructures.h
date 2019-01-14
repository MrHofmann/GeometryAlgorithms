#ifndef GA15_DATASTRUCTURES_H
#define GA15_DATASTRUCTURES_H

#include "utils.h"
#include "algorithms_practice/ga04_dcel.h"
#include <iostream>
#include <QPoint>
#include <QLine>

#define EPS 0.05

///
/// \brief The vertex struct            - struktura koja definise cvor grafa
///
struct vertex{
    ///
    /// \brief _v                       - ime cvora
    ///
    std::string _v;
    ///
    /// \brief _coordinates             - polozaj cvora u dekartovoj ravni
    ///
    QPoint _coordinates;
    ///
    /// \brief _obstacleVert            - pokazivac na teme prepreke koje odgovara cvoru grafa
    ///
    DCELVertex *_obstacleVert;
    ///
    /// \brief _edges                   - niz koji predstavlja grane do susednih cvorova
    ///
    std::vector<std::pair<double, vertex*> > _edges;

    ///
    /// \brief _prev                    - pokazivac na prethodne cvorove u izracunatom najkracem putu
    ///
    std::vector<vertex*> _prev;
    ///
    /// \brief _visited                 - indikator koji pokazuje da li je cvor posecen prilikom pretrage
    ///
    bool _visited;
    ///
    /// \brief _dist                    - udaljenost od pocetnog cvora u pretrazi
    ///
    double _dist;
    ///
    /// \brief _hx                      - euklidsko rastojanje do odredista
    ///
    double _hx;

    ///
    /// \brief vertex                   - konstruktor strukture cvor
    /// \param coordinates              - koordinate polozaja u dekartovoj ravni
    /// \param v                        - pokazivac na teme prepreke koje odgovara cvoru grafa
    ///
    vertex(QPoint coordinates, DCELVertex *v)
        : _coordinates(coordinates), _obstacleVert(v), _visited(false),
          _dist(std::numeric_limits<double>::max()), _hx(std::numeric_limits<double>::max())
    {
        _v = "v_" + std::to_string(coordinates.x()) + "," + std::to_string(coordinates.y());
        _prev.push_back(nullptr);
    }
};

///
/// \brief The Graph class              - klasa koja definise graf
///
class Graph{
public:
    ///
    /// \brief Graph                    - podrazumevani konstruktor
    ///
    Graph();
    ///
    /// \brief addVertex                - funkcija koja dodaje cvor grafu
    /// \param p                        - tacka koja definise koordinate cvora
    /// \param obstacle                 - pokazivac na teme prepreke
    ///
    void addVertex(const QPoint &p, DCELVertex *obstacle);
    ///
    /// \brief addEdge                  - funkcija koja dodaje granu izmedju dva cvora u grafu
    /// \param from                     - cvor iz kog izlazi grana
    /// \param to                       - cvor u koji ulazi grana
    /// \param weight                   - tezina grane
    ///
    void addEdge(QPoint from, QPoint to, double weight);
    ///
    /// \brief vertices                 - geter koji vraca strukturu koja cuva sve cvorove grafa
    /// \return                         - struktura koja cuva sve cvorove grafa
    ///
    std::map<QPoint, vertex *> vertices() const;

private:
    ///
    /// \brief _vertices                - struktura koja cuva sve cvorove grafa
    ///
    std::map<QPoint, vertex*>   _vertices;
};

///
/// \brief The searchType enum          - definicija tipova pretrage
///
enum searchType {DIJKSTRA, A_STAR};

///
/// \brief The EventQueueVertex struct  - struktura koja opisuje dogadjaj
///
struct EventQueueVertex
{
    ///
    /// \brief event                    - tacka koja odgovara dogadjaju
    ///
    QPoint event;
    ///
    /// \brief v                        - cvor grafa koje odgovara dogadjaju
    ///
    vertex *v;
    ///
    /// \brief sweepLine                - polozaj brisuce prave
    ///
    QLine sweepLine;
};

///
/// \brief The EventQueueAngleComp struct   - funktor koji uporedjuje dva dogadjaja
///
struct EventQueueAngleComp
{
    ///
    /// \brief operator ()              - operator poredjenja dva dogadjaja
    /// \param lhs                      - levi operand
    /// \param rhs                      - desni operand
    /// \return                         - tacno ako je levi manji od desnog, netacno inace
    ///
    bool operator()(const EventQueueVertex lhs, const EventQueueVertex rhs) const
    {
        QPoint p1 = lhs.sweepLine.p1();
        QPoint q1 = lhs.event;
        QPoint q2 = rhs.event;
        double pi = atan2(0, -1);

        double angle1 = atan2(p1.y() - q1.y(), p1.x() - q1.x());
        double angle2 = atan2(p1.y() - q2.y(), p1.x() - q2.x());

        if(angle1 == pi)
            angle1 = -pi;
        if(angle2 == pi)
            angle2 = -pi;

        if(angle1 != angle2)
            return angle1 < angle2;
        else
            return utils::distance2(p1,q1) < utils::distance2(p1,q2);

    }
};

///
/// \brief The StatusQueueCompare struct    - funktor koji uporedjuje dve duzi u statusu
///
struct StatusQueueCompare
{
private:
    ///
    /// \brief sweepLine                    - pokazivac na brisucu pravu
    ///
    QLine *sweepLine;
public:
    ///
    /// \brief StatusQueueCompare           - konstruktor funktora
    /// \param l                            - pokazivac na brisucu pravu
    ///
    StatusQueueCompare(QLine *l)
    {
        sweepLine = l;
    }    
    ///
    /// \brief operator ()                  - operator koji uporedjuje dve duzi
    /// \param lhs                          - levi operand
    /// \param rhs                          - desni operand
    /// \return                             - tacno ako je presek levog na manjem rastojanju od preseka desnog
    ///
    bool operator()(const QLineF &lhs, const QLineF &rhs) const
    {
        QPoint p = sweepLine->p1();
        QLineF l = QLineF(*sweepLine);
        QPointF i1, i2;

        utils::lineIntersection2(l, lhs, &i1);
        utils::lineIntersection2(l, rhs, &i2);

//        std::cout << "STATUS" << std::endl;
//        std::cout << "-----------------------------------------------" << std::endl;
//        std::cout << "sweep line: " << l.x1() << ", " << l.y1() << "-----" << l.x2() << ", " << l.y2() << std::endl;
//        std::cout << "l1: " << lhs.x1() << ", " << lhs.y1() << "-----" << lhs.x2() << ", " << lhs.y2() << std::endl;
//        std::cout << "l2: " << rhs.x1() << ", " << rhs.y1() << "-----" << rhs.x2() << ", " << rhs.y2() << std::endl;
//        std::cout << "intersections: " << i1.x() << ", " << i1.y() << "; " << i2.x() << ", " << i2.y() << std::endl;
//        std::cout << "distances: " << utils::distance2(p,i1) << "; " << utils::distance2(p, i2) << std::endl;
//        std::cout << "-----------------------------------------------" << std::endl << std::endl;

//        if(utils::distance2(p,i1) == utils::distance2(p, i2))
//            std::cout << "EQUAL: " << utils::distance2(p,i1) << " and " << utils::distance2(p,i2) << std::endl;
//        else
//            std::cout << "NOT EQUAL: " << utils::distance2(p,i1) << " and " << utils::distance2(p,i2) << std::endl << std::endl;

        return utils::distance2(p,i1) < utils::distance2(p, i2);
    }
};

///
/// \brief The SearchQueueCompare struct    - funktor za uporedjivanje dva cvora u redu sa prioritetom
///
struct SearchQueueCompare
{
private:
    ///
    /// \brief search                   - pokazivac na tip pretrage
    ///
    searchType *search;
public:
    ///
    /// \brief SearchQueueCompare       - konstruktor funktora
    /// \param s                        - pokazivac na tip pretrage
    ///
    SearchQueueCompare(searchType *s)
    {
        search = s;
    }
    ///
    /// \brief operator ()              - operator koji uporedjuje dva elementa reda sa prioritetom
    /// \param v1                       - levi operand
    /// \param v2                       - desni operand
    /// \return                         - tacno ako je levi bitniji od desnog
    ///
    bool operator () (const vertex *v1, const vertex *v2) const
    {
        switch(*search)
        {
            case DIJKSTRA:
                return v1->_dist > v2->_dist;
            case A_STAR:
                return v1->_dist+v1->_hx > v2->_dist+v2->_hx;
            default:
                return v1->_dist > v2->_dist;
        }
    }
};

#endif // GA15_DATASTRUCTURES_H
