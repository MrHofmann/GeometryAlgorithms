/*
Autor: Vojislav Stanković
Godina: 2018
Kratak opis problema:
Određivanje i vizualizacija suma Minkovskog dva konveksna poligona u ravni
u složenosti O(m+n) gde su m i n broj temena poligona.
*/


#ifndef GA17_MINKOWSKISUMS_H
#define GA17_MINKOWSKISUMS_H

#include "algorithmbase.h"
#include "algorithms_practice/ga02_grahamscan.h"

//radius of a circle on which a regular polygon is drawn
#define RAD     (100)
//coordinates of the center of the first polygon
#define xOff1   (200)
#define yOff1   (150)
//coordinates of the center of the first polygon
#define xOff2   (200)
#define yOff2   (400)
//offset needed for result polygon
#define xOffR   (500)
#define yOffR   (450)
//position of resulting polygon name
#define xCen    (550)
#define yCen    (250)
//angle offset for regular polygon
#define ANGLE   (2*M_PI)
#define MIN_ANG (0)
//Minimum number of vertices that polygon can have
#define MINV    (3)
//Bold pen width
#define BOLDPEN (5)
//Regular pen width
#define PEN     (1)
//Origin point coordinates
#define xOrigin (0)
#define yOrigin (0)

//Edge of a polygon
class Edge{
public:
    Edge(QLineF edge);

    //getters and setters
    QLineF getEdge() const;
    void setEdge(const QLineF edge);
    qreal getTheta() const;
    void setTheta(const qreal theta);
private:
    //line component of the edge
    QLineF _edge;
    //polar angle of the edge
    qreal _theta;

};


class MinkowskiSums : public AlgorithmBase
{
public:
    MinkowskiSums(QWidget *pRenderer, int delayMs, std::string fileName = "", int n = DEFAULT_POINTS_NUM, int m = DEFAULT_POINTS_NUM);

    // AlgorithmBase interface
public:

    enum AlgorithmStatus {INVALID_INPUT};

    void runAlgorithm();
    void drawAlgorithm(QPainter &painter) const;
    void runNaiveAlgorithm();
    //Reading polygons from file
    std::vector<QLineF> readPolygonsFromFile(std::string fileName = "");
    std::vector<QPointF> readConvexSetFromFile(std::string fileName = "");
    //converting line to edge
    std::vector<Edge> convertLineToEdge(std::vector<QLineF> lines);
    std::vector<QLineF> convertEdgeToLine(std::vector<Edge> edges);
    std::vector<Edge> shiftVector(std::vector<Edge> edges);
    void translateVector(int i);
    std::vector<QPointF> generateRandom(int inputSize);
    std::vector<QLineF> transformHull(std::vector<QPointF> hull);

    //getters and setters
    std::vector<QLineF> getResult() const;
    std::vector<QPointF> getNaiveResult() const;
    int getM() const;
    int getN() const;
    AlgorithmStatus getStatus() const;
    std::vector<QLineF> getLines();
    std::vector<QPointF> regularPolygon(int n, qreal xOff, qreal yOff);

private:

    AlgorithmStatus _status;

    //Arrays for random input
    std::vector<QPointF> _randomHullA;
    std::vector<QPointF> _randomHullB;

    //Arrays that are directly participating in algorithm
    std::vector<Edge> _firstHull;
    std::vector<Edge> _secondHull;
    std::vector<Edge> _resultHull;
    std::vector<QLineF> _compareHull;
    std::vector<QLineF> _naiveHull;

    //Arrays needed for smooth drawing animation
    std::vector<Edge> _p;
    std::vector<Edge> _q;
    std::vector<QLineF> _convexHull;

    //Convex result set for naive algorithm
    std::vector<QPointF> _convexSet;

    //Input size
    int _n;
    int _m;
    //Input file size
    int _nSize;
    int _mSize;
    int _indicator;

    //Origin point for drawing resulting convex hull
    QPointF _origin;
};




#endif // GA17_MINKOWSKISUMS_H

