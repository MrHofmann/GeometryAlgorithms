/*
Autor: Anja Bukurov 1082/2016
Godina: 2017/18
Kratak opis problema: Dato je n kruznica u ravni. Potrebno je odrediti sve presecne tacke.
*/

#ifndef GA10_CIRCLEINTERSECTION_H
#define GA10_CIRCLEINTERSECTION_H

#include "algorithmbase.h"
#include "utils.h"
#include <set>
#include <fstream>
#include <iostream>
#include <algorithm>

enum EType{UPPER, LOWER};

/* comparator for intersectiong points
 * sorting points by their x-coordinate ascending
 * then by their y-coordinate ascending
 */
struct PointComparator
{
    bool operator()(const QPoint lhs, const QPoint rhs) const
    {
        if(lhs.x() != rhs.x())
            return lhs.x() < rhs.x();
        else
            return lhs.y() < rhs.y();
    }
};


class CircleI
{
public:
    // constructors
    CircleI(const QPoint p, int r);
    CircleI(int x, int y, int r);


    // getters and setters
    QPoint center() const;
    void setCenter(const QPoint &center);
    int radius() const;
    void setRadius(int radius);

    // checks if this circle is equal to circle c by comparing their centers and radius
    bool equal(const CircleI c) const;

    // checks if this circle is intersecting with circle c and returns number of intersections
    int intersecting(const CircleI c) const;

    // using function intersecting it determines if to compute intersection or not
    // if there are intersections they are computed and inserted in intersectionSet
    void intersections(const CircleI c, std::set<QPoint, PointComparator> *intersectionSet);

private:
    QPoint _center;
    int _radius;
};


class EventPoint
{
public:
    // constructors
    EventPoint(const EType et, const QPoint p, const CircleI c);

    // getters and setters
    EType type() const;
    void setType(const EType &type);
    QPoint point() const;
    void setPoint(const QPoint &point);
    CircleI circle() const;
    void setCircle(const CircleI &circle);

private:
    // event can be upper or lower point
    EType _type;

    // event point
    QPoint _point;

    // circle that contains this event point
    CircleI _circle;
};

/* comparator for event points
 * first sorting points by their y-coordinate
 * then by their x-coordinate
 * then by their type - so that upper event comes first
 * then by their circles- radius then center
 */
struct EventPointComparator
{
    bool operator()(const EventPoint lhs, const EventPoint rhs) const
    {
        if(lhs.point().y() != rhs.point().y())
            return lhs.point().y() < rhs.point().y();
        else if(lhs.point().x() != rhs.point().x())
            return lhs.point().x() < rhs.point().x();
        else if(lhs.type() != rhs.type())
            return lhs.type() < rhs.type();
        else if(lhs.circle().radius() != rhs.circle().radius())
            return lhs.circle().radius() < rhs.circle().radius();
        else if(lhs.circle().center().x() != rhs.circle().center().x())
            return lhs.circle().center().x() < rhs.circle().center().x();
        else
            return lhs.circle().center().y() < rhs.circle().center().y();
    }
};

/* comparator for circles
 * sorting circles by their most left point
 * first by their x-coordinate
 * then by their y-coordinate
 * then by their radius
 */
struct CircleComparator {
    bool operator()(const CircleI lhs, const CircleI rhs) const
    {
        // sorting circles by their most left point
        if(lhs.center().x() - lhs.radius() != rhs.center().x() - rhs.radius())
            return lhs.center().x() - lhs.radius() < rhs.center().x() - rhs.radius();
        else if(lhs.center().y() - lhs.radius() != rhs.center().y() - rhs.radius())
            return lhs.center().y() - lhs.radius() < rhs.center().y() - rhs.radius();
        else
            return lhs.radius() < rhs.radius();
    }
};

class CircleIntersection : public AlgorithmBase
{
public:
    // constructor
    CircleIntersection(QWidget* pRenderer, int delayMs, std::string filename = "", int circlesNum = DEFAULT_POINTS_NUM);

    // function that generates random circles that fit the window
    std::vector<CircleI> generateRandomCircles(int circlesNum);

    // function that reads circles from file
    std::vector<CircleI> readCirclesFromFile(std::string fileName);

    // AlgorithmBase interface
public:
    void runAlgorithm();
    void drawAlgorithm(QPainter &painter) const;
    void runNaiveAlgorithm();

    // getters and setters
    std::set<QPoint, PointComparator> intersections() const;
    void setIntersections(const std::set<QPoint, PointComparator> &intersections);
    std::set<QPoint, PointComparator> intersectionsNaive() const;
    void setIntersectionsNaive(const std::set<QPoint, PointComparator> &intersectionsNaive);
    void setCircles(const std::vector<CircleI> &circles);

private:
    // input is stored in this vector and it represents n circles
    std::vector<CircleI> _circles;

    // intersection computed in function runAlgorithm are stored in this set
    std::set<QPoint, PointComparator> _intersections;

    // intersections computed by Naive algorithm are stored in this set
    std::set<QPoint, PointComparator> _intersectionsNaive;

    // set of event points
    std::set<EventPoint, EventPointComparator> _events;

    // set of circles intersected by sweep line
    std::set<CircleI, CircleComparator> _status;

    // position of sweep line
    int _y;
};



#endif // GA10_CIRCLEINTERSECTION_H
