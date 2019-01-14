#include "ga02_rotatingcalipers.h"
#include "utils.h"
#include <math.h>

#include <iostream>


RotatingCalipers::RotatingCalipers(QWidget* pRenderer, int delayMs, std::string filename, int inputSize)
     :ConvexHull(pRenderer, delayMs, filename, inputSize)
{
    if(filename == "")
        _points = generateRandomPoints();
    else
        _points = readPointsFromFile(filename);
}


/*racuna povrsinu pravougaonika*/
double RotatingCalipers::calculateArea(std::vector<QPoint> rectangle)
{
    //P = a * b
    //a = sqrt((x1 - x2)^2 + (y1 - y2)^2)
    double p1 = pow(rectangle[0].x() - rectangle[1].x(), 2);
    double p2 = pow(rectangle[0].y() - rectangle[1].y(), 2);

    double a = sqrt(p1 + p2);

    double q1 = pow(rectangle[1].x() - rectangle[2].x(), 2);
    double q2 = pow(rectangle[1].y() - rectangle[2].y(), 2);

    double b = sqrt(q1 + q2);

    return a * b;
}


/*trazi najmanji ugao za koji cemo rotirati calipere*/
double RotatingCalipers::findSmallestAngle(Caliper c1, Caliper c2, Caliper c3, Caliper c4)
{
    double a1, a2, a3, a4;

    //za svaki racunamo ugao
    a1 = c1.calculateAngle();
    a2 = c2.calculateAngle();
    a3 = c3.calculateAngle();
    a4 = c4.calculateAngle();

    //trazimo najmanji
    if(a1 <= a2 && a1 <= a3 && a1 <= a4)
    {
        return a1;
    }
    else if(a2 <= a3 && a2 < a4)
    {
        return a2;
    }
    else if(a3 <= a4)
    {
        return a3;
    }
    else
    {
        return a4;
    }

}


void RotatingCalipers::runAlgorithm()
{
    //ako imamo manje od 3 tacke, izlaz je prazan
    if (_points.size() < 3)
    {
       emit animationFinished();
       return;
    }

    /*Trazenje konveksnog omotaca da nam njim primenimo rotatin calipers*/
    GrahamScan* g_hull = new GrahamScan(_pRenderer, 0, "", 0, _points);
    g_hull->runAlgorithm();
    AlgorithmBase_updateCanvasAndBlock();
    _convexHull = g_hull->convexHull();
    AlgorithmBase_updateCanvasAndBlock();


    /*Racunanje 4 krajnje tacke za inicijalizaciju*/
    _x_min = _convexHull[0];
    _x_max = _convexHull[0];
    _y_min = _convexHull[0];
    _y_max = _convexHull[0];
    _index_down = 0;
    _index_up = 0;
    _index_right = 0;
    _index_left = 0;
    for(unsigned i = 0; i < _convexHull.size(); i++)
    {
        if(_convexHull[i].x() < _x_min.x())
        {
            _x_min = _convexHull[i];
            _index_left = i;
        }
        if(_convexHull[i].x() > _x_max.x())
        {
            _x_max = _convexHull[i];
            _index_right = i;
        }
        if(_convexHull[i].y() < _y_min.y())
        {
            _y_min = _convexHull[i];
            _index_up = i;
        }
        if(_convexHull[i].y() > _y_max.y())
        {
            _y_max = _convexHull[i];
            _index_down = i;
        }
    }


    //cuvamo trenutni pravougaonik, vektor tacaka, i skup svih pravougaonika, vektor vektora tacaka
    std::vector<QPoint> rectangle;
    std::vector <std::vector<QPoint>> rectangles;

    //inicijalizujemo uglove pocetnih calipera tako da su paralelni sa x i y osom
    Caliper caliper_left = Caliper(_convexHull, _index_left, 270);
    Caliper caliper_right = Caliper(_convexHull, _index_right, 90);
    Caliper caliper_up = Caliper(_convexHull, _index_up, 0);
    Caliper caliper_down = Caliper(_convexHull, _index_down, 180);

    while(caliper_up._angle < 90.0)
    {
        A = caliper_left.findIntersectionPoint(caliper_up);
        B = caliper_right.findIntersectionPoint(caliper_up);
        C = caliper_right.findIntersectionPoint(caliper_down);
        D = caliper_left.findIntersectionPoint(caliper_down);

        rectangle.push_back(A);
        rectangle.push_back(B);
        rectangle.push_back(C);
        rectangle.push_back(D);

        _AB = QLine(A, B);
        _BC = QLine(B, C);
        _CD = QLine(C, D);
        _DA = QLine(D, A);
        AlgorithmBase_updateCanvasAndBlock();

        rectangles.push_back(rectangle);

        //nadji najmanji ugao, rotiraj
        double smallestAngle = findSmallestAngle(caliper_down, caliper_up, caliper_left, caliper_right);
        caliper_down.rotate(smallestAngle);
        caliper_up.rotate(smallestAngle);
        caliper_left.rotate(smallestAngle);
        caliper_right.rotate(smallestAngle);
    }

    //nadjemo najmanji pravougaonik
    std::vector<QPoint> minBoundingRectangle;
    double minBoundingRectangleSurface = LONG_LONG_MAX;
    for(auto rectangle : rectangles)
    {
        double surfaceArea = calculateArea(rectangle);

        if(surfaceArea < minBoundingRectangleSurface)
        {
            minBoundingRectangleSurface = surfaceArea;
            minBoundingRectangle = rectangle;
        }
    }
    AB = QLine(minBoundingRectangle[0], minBoundingRectangle[1]);
    BC = QLine(minBoundingRectangle[1], minBoundingRectangle[2]);
    CD = QLine(minBoundingRectangle[2], minBoundingRectangle[3]);
    DA = QLine(minBoundingRectangle[3], minBoundingRectangle[0]);

    AlgorithmBase_updateCanvasAndBlock();

    emit animationFinished();
}



void RotatingCalipers::drawAlgorithm(QPainter &painter) const
{
    QPen p = painter.pen();
    p.setWidth(5);
    p.setColor(Qt::black);
    painter.setPen(p);

    //Iscrtavanje svih tacaka
    for(QPoint p: _points)
       painter.drawPoint(p);

    p.setColor(Qt::green);
    p.setWidth(1);
    painter.setPen(p);

    painter.drawPolygon(_convexHull.data(), _convexHull.size());

    p.setColor(Qt::red);
    p.setWidth(6);
    painter.setPen(p);
    painter.drawPoint(_x_min);
    p.setColor(Qt::red);
    p.setWidth(6);
    painter.setPen(p);
    painter.drawPoint(_x_max);
    p.setColor(Qt::red);
    p.setWidth(6);
    painter.setPen(p);
    painter.drawPoint(_y_min);
    p.setColor(Qt::red);
    p.setWidth(6);
    painter.setPen(p);
    painter.drawPoint(_y_max);

    p.setColor(Qt::black);
    p.setWidth(1);
    painter.setPen(p);
    painter.drawLine(_AB);
    painter.drawLine(_BC);
    painter.drawLine(_CD);
    painter.drawLine(_DA);

    p.setColor(Qt::green);
    p.setWidth(6);
    painter.setPen(p);
    painter.drawPoint(A);
    painter.drawPoint(B);
    painter.drawPoint(C);
    painter.drawPoint(D);

    p.setColor(Qt::blue);
    p.setWidth(1);
    painter.setPen(p);
    painter.drawLine(AB);
    painter.drawLine(BC);
    painter.drawLine(CD);
    painter.drawLine(DA);
}


void RotatingCalipers::drawGraham(const std::vector<QPoint>& points, QPainter &painter, int r, int g, int b) const
{
    QPen p = painter.pen();
    p.setWidth(1);
    p.setColor(QColor(r, g, b));
    painter.setPen(p);

    QPainterPath path;
    path.moveTo(points[0]);

    int sizeOfHull = points.size()-1;
    for(int i = 0; i < sizeOfHull; i++)
    {
        path.lineTo(points[i+1]);
    }
    path.lineTo(points[0]);
    painter.drawLine(points[sizeOfHull],points[0]);
    painter.fillPath(path, QBrush(QColor(r,g,b,140)));
}

void RotatingCalipers::runNaiveAlgorithm()
{

}

std::vector<QPoint> RotatingCalipers::convexHull() const
{
    return _convexHull;
}


