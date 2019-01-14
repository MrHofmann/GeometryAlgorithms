/*
Autor: Strahinja Milojević
Godina: 2018
Kratak opis problema:
Implementacija i vizualizacija Saterlend-Hodžmanovog algoritma
za klipovanje poligona, odnosno određivanje dela glavnog poligona
koji se nalazi unutar (konveksnog) klip poligona.
*/


#ifndef GA20_SUTHERLANDHODGMAN_H
#define GA20_SUTHERLANDHODGMAN_H

#include <algorithmbase.h>
#include "algorithms_practice/ga04_dcel.h"
#include "algorithms_practice/ga04_dceldemo.h"
#include <vector>

using namespace std;

class SutherlandHodgman : public AlgorithmBase
{
    // AlgorithmBase interface
public:
    SutherlandHodgman(QWidget *pRender, int delayMs, std::string fileName, int n, int m);

    void runAlgorithm();
    void drawAlgorithm(QPainter &painter) const;
    void runNaiveAlgorithm();

    vector<DCELVertex*> outputList;

private:
    DCEL* input;
    DCELField* subjectPolygon;
    DCELField* clipPolygon;
    vector<DCELVertex*> subjectPolygonVertices;
    vector<DCELVertex*> clipPolygonVertices;

    vector<DCELHalfEdge*> subjectPolygonEdges;
    vector<DCELHalfEdge*> clipPolygonEdges;

    DCELHalfEdge* clipEdgeTest;

    ///
    /// \brief ComputeIntersection
    /// \param a
    /// \param b
    /// \param edge
    /// \return intersection of lines (a,b) and edge (or null if lines are parallel)
    ///
    DCELVertex* ComputeIntersection(DCELVertex* a, DCELVertex* b, DCELHalfEdge* edge);

    ///
    /// \brief IsInFrontOf
    /// \param v
    /// \param edge
    /// \return true if vertex v is in front of edge 'edge'
    ///
    bool IsInFrontOf(DCELVertex* v, DCELHalfEdge* edge);

    ///
    /// \brief IsPolygonConvex
    /// \param poly
    /// \return true if polygon poly is convex
    ///
    bool IsPolygonConvex(vector<DCELVertex*>& poly);

    ///
    /// \brief onSegment
    /// \param p
    /// \param q
    /// \param r
    /// \return true if q is on segment [p,r]
    ///
    bool onSegment(DCELVertex* p, DCELVertex* q, DCELVertex* r);

    ///
    /// \brief orientation
    /// \param p
    /// \param q
    /// \param r
    /// \return 0, if p, q and r are colinear
    ///         1, if orientation is clockwise
    ///         2, if orientation is counterclockwise
    ///
    int orientation(DCELVertex* p, DCELVertex* q, DCELVertex* r);

    ///
    /// \brief doIntersect
    /// \param p1
    /// \param q1
    /// \param p2
    /// \param q2
    /// \return true if [p1, q1] and [p2, q2] intersect
    ///
    bool doIntersect(DCELVertex* p1, DCELVertex* q1, DCELVertex* p2, DCELVertex* q2);

    ///
    /// \brief isInside
    /// \param p
    /// \param polygon
    /// \return true if vertex p is inside polygon
    ///
    bool isInside( DCELVertex* p, vector<DCELVertex*> polygon);

    ///
    /// \brief IsOnLineSegment
    /// \param point
    /// \param start
    /// \param end
    /// \return true if point is on line segment [start, end]
    ///
    bool IsOnLineSegment(DCELVertex* point, DCELVertex* start, DCELVertex* end);

    ///
    /// \brief ComputeIntersection1
    /// \param e1
    /// \param e2
    /// \return intersection of halfedges e1 and e2 (not lines e1 and e2!)
    ///
    DCELVertex* ComputeIntersection1(DCELHalfEdge* e1, DCELHalfEdge* e2);


};

#endif // GA20_SUTHERLANDHODGMAN_H
