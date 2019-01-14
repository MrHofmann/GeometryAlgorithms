#ifndef GA03_LINESEGMENTINTERSECTION_H
#define GA03_LINESEGMENTINTERSECTION_H

#include <set>

#include "algorithmbase.h"
#include "utils.h"
#include "ga03_datastructures.h"

class LineSegmentIntersection : public AlgorithmBase
{
public:
    LineSegmentIntersection(QWidget *pRenderer, int delayMs, std::string filename = "", int dimension = 15);

    // AlgorithmBase interface
public:
    void runAlgorithm();
    void drawAlgorithm(QPainter &painter) const;
    void runNaiveAlgorithm();

private:
    std::vector<QLineF> _lineSegments;
    std::set<EventQueueNode, EventQueueComp> _eventQueue;
    std::set<QLineF*, StatusQueueComp> _statusQueue;

    double _sweepLineY;
    std::vector<QPointF> _intersections;

    void deleteFromStatus(QLineF* l);
};

#endif // GA03_LINESEGMENTINTERSECTION_H
