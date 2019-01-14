#ifndef GA05_TRIANGULATION_H
#define GA05_TRIANGULATION_H

#include <set>

#include "algorithmbase.h"
#include "ga04_dcel.h"
#include "utils.h"

/* Funktor za poredjenje dva dogadjaja */
struct VertexCompare
{
    bool operator()(const DCELVertex* lhs, const DCELVertex* rhs) const
    {
        if(lhs->coordinates().y() != rhs->coordinates().y())
            return lhs->coordinates().y() < rhs->coordinates().y();
        else
            return lhs->coordinates().x() < rhs->coordinates().x();
    }
};

/* Funktor za poredjenje dva dogadjaja */
struct TmpEdgeCompare
{
    bool operator()(const DCELHalfEdge* lhs, const DCELHalfEdge* rhs) const
    {
        if(lhs->origin()->coordinates().y() != rhs->origin()->coordinates().y())
            return lhs->origin()->coordinates().y() < rhs->origin()->coordinates().y();
        else
            return lhs->origin()->coordinates().x() < rhs->origin()->coordinates().x();
    }
};

/* Funktor za poredjenje dve grane */
struct EdgeCompare
{
private:
    double* _ypos;
public:
    EdgeCompare(double * ypos);

    bool operator()(const DCELHalfEdge* lhs, const DCELHalfEdge* rhs) const
    {
        QLineF sweepLine = QLineF(0, *_ypos, 2000, *_ypos);
        QPointF i1, i2;
        QLineF l1(lhs->origin()->coordinates(), lhs->twin()->origin()->coordinates());
        QLineF l2(rhs->origin()->coordinates(), rhs->twin()->origin()->coordinates());
        utils::lineIntersection(sweepLine, l1, &i1);
        utils::lineIntersection(sweepLine, l2, &i2);
        return i1.x() < i2.x();
    }
};

class Triangulation : public AlgorithmBase
{
public:
    Triangulation(QWidget *pRenderer, int delayMs, std::string filename);

    // AlgorithmBase interface
public:
    void runAlgorithm();
    void drawAlgorithm(QPainter &painter) const;
    void runNaiveAlgorithm();

    double sweepLinePos() const;

private:
    DCEL _polygon;
    std::set<DCELVertex*, VertexCompare> _eventQueue;
    std::set<DCELHalfEdge*, EdgeCompare> _statusQueue;
    double _sweepLinePos;
};

#endif // GA05_TRIANGULATION_H
