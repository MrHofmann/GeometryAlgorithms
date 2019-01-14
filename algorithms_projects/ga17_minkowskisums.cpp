#include "ga17_minkowskisums.h"

#include <QPainter>
#include <fstream>
#include "math.h"
#include <iostream>
#include <iterator>
#include <stdlib.h>
#include <algorithm>
#include "utils.h"

MinkowskiSums::MinkowskiSums(QWidget *pRenderer, int delayMs, std::string fileName, int n, int m)
    :AlgorithmBase (pRenderer, delayMs)
{
    //Array for text input
    std::vector<QLineF> lines;
    //Arrays for generated input
    std::vector<QLineF> lineA;
    std::vector<QLineF> lineB;

    if(fileName == ""){
        _n = n;
        _m = m;
        //No polygons
        if(_n < MINV || _m < MINV){
            _status = AlgorithmStatus::INVALID_INPUT;
        }
        else{
            _indicator = 0;
            _randomHullA = regularPolygon(n, xOff1, yOff1);
            _randomHullB = regularPolygon(m, xOff2, yOff2);
            lineA = transformHull(_randomHullA);
            lineB = transformHull(_randomHullB);
            _firstHull = convertLineToEdge(lineA);
            _secondHull = convertLineToEdge(lineB);
            QPointF offset(xOffR, yOffR);
            _origin = offset;
        }
    }
    else{
        lines = readPolygonsFromFile(fileName);
        _indicator = 1;
        //Split input into separate polygons
        std::vector<QLineF> p(lines.begin(), lines.begin() + _nSize);
        std::vector<QLineF> q(lines.begin() + _nSize, lines.end());

        //Instantiating hulls
        _firstHull = convertLineToEdge(p);
        _secondHull = convertLineToEdge(q);
    }

    //Shift array
    _firstHull = shiftVector(_firstHull);
    _secondHull = shiftVector(_secondHull);

}

void MinkowskiSums::runAlgorithm()
{
    if(_status == AlgorithmStatus::INVALID_INPUT){
        emit animationFinished();
        return;
    }
    //Instantiating iterators for both polygons
    auto first1 = _firstHull.begin();
    auto last1 = _firstHull.end();
    auto first2 = _secondHull.begin();
    auto last2 = _secondHull.end();
    //result iterator
    auto d_result = std::back_inserter(_resultHull);
    //iterator for duplicate polygons needed for nice drawing animation
    auto d_p = std::back_inserter(_p);
    auto d_q = std::back_inserter(_q);

    int i=0;
    for (; first1 != last1; ++d_result) {
        //In case if second hull is empty
        if (first2 == last2) {
            std::copy(first1, last1, d_result);
            std::copy(first1, last1, d_p);
            break;
        }
        //Ordering edges by ascending angle
        else if (first2->getTheta() < first1->getTheta()) {
                *d_result = *first2;
                *d_q = *first2;
                ++first2;
                translateVector(i);
                AlgorithmBase_updateCanvasAndBlock();
         }
         else {
                *d_result = *first1;
                *d_p = *first1;
                ++first1;
                translateVector(i);
                AlgorithmBase_updateCanvasAndBlock();
         }
        i++;
    }
    //In case if first hull is empty
    std::copy(first2, last2, d_result);
    std::copy(first2, last2, d_q);
    for(;i<_resultHull.size();){
        translateVector(i);
        i++;
    }
    AlgorithmBase_updateCanvasAndBlock();

    //Converts vector<Edge> to vector<QLineF>, for test usage because comparison with QLineF is easier
    _compareHull = convertEdgeToLine(_resultHull);

    emit animationFinished();
}

void MinkowskiSums::drawAlgorithm(QPainter &painter) const
{

    QPen p = painter.pen();
    p.setColor(Qt::red);
    painter.setPen(p);

    if(_indicator == 0){
        painter.drawText(xOff1, yOff1, "P");
    }

    //Drawing first convex hull
    for(Edge line: _firstHull){
        painter.drawLine(line.getEdge());
    }
    p.setWidth(BOLDPEN);
    painter.setPen(p);
    for(Edge line: _p){
        painter.drawLine(line.getEdge());
    }

    p.setColor(Qt::blue);
    p.setWidth(PEN);
    painter.setPen(p);

    if(_indicator == 0){
        painter.drawText(xOff2, yOff2, "Q");
    }

    //Drawing second convex hull
    for(Edge line: _secondHull){
        painter.drawLine(line.getEdge());
    }
    p.setWidth(BOLDPEN);
    painter.setPen(p);
    for(Edge line: _q){
        painter.drawLine(line.getEdge());
    }

    p.setColor(Qt::darkMagenta);
    p.setWidth(BOLDPEN);
    painter.setPen(p);
    if(_indicator == 0){
        painter.drawText(xCen, yCen, "R");
    }

    //Drawing resulting convexHull
    for(QLineF line: _convexHull){
        painter.drawLine(line);
    }
}

void MinkowskiSums::runNaiveAlgorithm()
{
    if(_status == AlgorithmStatus::INVALID_INPUT){
        return;
    }
    //Naive algorythm adds every vector from first polygon
    //to every vector of second polygon
    for(Edge p: _firstHull){
        qreal xp = p.getEdge().x2() - p.getEdge().x1();
        qreal yp = p.getEdge().y2() - p.getEdge().y1();
        for(Edge q: _secondHull){
            qreal xq = q.getEdge().x2() - q.getEdge().x1();
            qreal yq = q.getEdge().y2() - q.getEdge().y1();
            qreal x = xp + xq;
            qreal y = yp + yq;
            QPointF r(x, y);
            _convexSet.push_back(r);
        }
    }
}

//Makes lines out of points
std::vector<QLineF> MinkowskiSums::transformHull(std::vector<QPointF> hull){

    std::vector<QLineF> line;
    for(int i=0; i< hull.size()-1; i++){
        line.emplace_back(hull.at(i), hull.at(i+1));
    }

    return line;
}

//function for creating regular polygon
std::vector<QPointF> MinkowskiSums::regularPolygon(int n, qreal xOff, qreal yOff){
   qreal x, y;
   std::vector<QPointF> points;

   for(int i=0; i<n; i++){
       x = RAD * cos(ANGLE*i/n) + xOff;
       y = RAD * sin(ANGLE*i/n) + yOff;
       points.emplace_back(x, y);
   }
   x = RAD * cos(MIN_ANG) + xOff;
   y = RAD * sin(MIN_ANG) + yOff;
   points.emplace_back(x, y);
   return points;
}


//Translates vector by a given offset
void MinkowskiSums::translateVector(int i){
    QPointF offset(_origin.x() - _resultHull.at(i).getEdge().x1(), _origin.y() - _resultHull.at(i).getEdge().y1());
    QLineF translated = _resultHull.at(i).getEdge().translated(offset);
    _origin = translated.p2();
   _convexHull.push_back(translated);
}

//Shifting array so that it becomes ascending
std::vector<Edge> MinkowskiSums::shiftVector(std::vector<Edge> edges){
    std::rotate(edges.begin(),
                std::min_element(edges.begin(), edges.end(),
                                 [](const Edge& lhs, const Edge& rhs){
                                    return lhs.getTheta() < rhs.getTheta();}),
                edges.end());

    return edges;
}

//Returns array with lines that represent two convex polygons
std::vector<QLineF> MinkowskiSums::readPolygonsFromFile(std::string fileName){

    std::ifstream inputFile(fileName);
    std::vector<QLineF> lines;
    int n, m, p, q, x1, y1, x2, y2;
   //Reading size of both polygons
    inputFile >> n >> m;
    _nSize = n;
    _mSize = m;
    inputFile >> p >> q;
    QPointF origin(p, q);
    _origin = origin;
    while(inputFile >> x1 >> y1 >> x2 >> y2 )
    {
        QLineF line(x1, y1, x2, y2);
        lines.emplace_back(line);
    }
    return lines;
}

//Constructor for edge of a polygon
Edge::Edge(QLineF edge)
{
    _edge = edge;
    _theta = atan2(edge.y2()-edge.y1(), edge.x2()-edge.x1());
}

//Converting simple line to Edge
std::vector<Edge> MinkowskiSums::convertLineToEdge(std::vector<QLineF> lines)
{
    std::vector<Edge> edges;
    for(QLineF line: lines){
        Edge edge(line);
        edges.emplace_back(edge);
    }

    return edges;
}

//Converts vector<Edge> to vector<QLineF>
std::vector<QLineF> MinkowskiSums::convertEdgeToLine(std::vector<Edge> edges){
    std::vector<QLineF> lines;
    for(Edge edge: edges){
        lines.emplace_back(edge.getEdge());
    }

    return lines;
}

QLineF Edge::getEdge() const{
    return _edge;
}

void Edge::setEdge(const QLineF edge){
    _edge = edge;
}

qreal Edge::getTheta() const{
    return _theta;
}

void Edge::setTheta(const qreal theta){
    _theta = theta;
}

std::vector<QLineF> MinkowskiSums::getResult() const{
    return _compareHull;
}

std::vector<QPointF> MinkowskiSums::getNaiveResult() const{
    return _convexSet;
}

int MinkowskiSums::getN() const{
    return _n;
}

int MinkowskiSums::getM() const{
    return _m;
}

MinkowskiSums::AlgorithmStatus MinkowskiSums::getStatus() const{
    return _status;
}
