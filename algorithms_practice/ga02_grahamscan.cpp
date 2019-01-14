#include "ga02_grahamscan.h"

#include <functional>

#include "utils.h"
#include <iostream>

GrahamScan::GrahamScan(QWidget *pRenderer, int delayMs, std::string filename, int inputSize, std::vector<QPoint> points)
    : ConvexHull(pRenderer, delayMs, filename, inputSize, points)
{
    _firstIsMinY = true;
    _firstIsMaxY = true;
    _firstIsMinX = true;

    //resavanje problema duplikata tacaka
    std::sort(_points.begin(), _points.end(), [](const QPoint &p1, const QPoint &p2){
        if ( p1.x() < p2.x() ){
            return true;
        }
        else if (p1.x() == p2.x() && p1.y() < p2.y() ){
            return true;
        }
        else{
            return false;
        }
    });
    auto it = std::unique(_points.begin(), _points.end());
    _points.resize(std::distance(_points.begin(), it));
}

void GrahamScan::runAlgorithm()
{
    //Pronalazak najdesnije tacke
    _firstPoint = _points[0];
    for(QPoint p: _points)
    {
        if(p.x() > _firstPoint.x() || (p.x() == _firstPoint.x() && p.y() > _firstPoint.y()))
        {
            _firstPoint = p;
        }
    }
    AlgorithmBase_updateCanvasAndBlock();

    for(QPoint p:_points){
        if(p.y() < _firstPoint.y())
            _firstIsMinY = false;
        if(p.y() > _firstPoint.y())
            _firstIsMaxY = false;
        if(p.x() < _firstPoint.x())
            _firstIsMinX = false;
    }

    //zelim sad da izbacim ovu prvu tacku iz ovih tacaka
    for(int i = 0; i < _points.size(); i++){
        if(_firstPoint == _points[i]){
            _points[i] = _points[_points.size()-1];
            _points.pop_back();
            break;
        }
    }


    //Sortiranje tacaka po uglu sa _firstPoint i pozitivnim krajem x ose
    // I nacin:
    //std::sort(_points.begin(), _points.end(), std::bind(&ConvexHull::compare, this, std::placeholders::_1, std::placeholders::_2));

    //II nacin:
    std::sort(_points.begin(), _points.end(), [&](const QPoint& lhs, const QPoint& rhs){return compare(lhs, rhs);});
    AlgorithmBase_updateCanvasAndBlock();

    //Pronalazak konveksnog omotaca
    _convexHull.push_back(_firstPoint);

    //ako se u _points nalazi samo jedna tacka, onda je to tacka _firstPoint i ona je ubacena u konv. omotac
    if(_points.size() == 0){
        //zbog konzisentnosti se ubacuje prva tacka na kraj (ciklicni raspored temena u konv. omotacu)
        _convexHull.push_back(_firstPoint);
        emit animationFinished();
        return;
    }

    AlgorithmBase_updateCanvasAndBlock();

    int m=1, k;
    if(_points[0] != _firstPoint)
    {
        _convexHull.push_back(_points[0]);
        k=1;
    }
    else
    {
        _convexHull.push_back(_points[1]);
        k=2;
    }

    AlgorithmBase_updateCanvasAndBlock();
    for(;k < _points.size(); k++)
    {
        _convexHull.push_back(_points[k]);
        AlgorithmBase_updateCanvasAndBlock();

        while(m > 0 && utils::negativeOrientation(_convexHull[m-1], _convexHull[m], _points[k], true))
        {
            // Brise se m:
            // Brisemo k, brisemo m, vracamo k
            // Ovo radimo samo zbog lepse vizuelizacije
            _convexHull.pop_back();
            _convexHull.pop_back();
            _convexHull.push_back(_points[k]);
            m--;
            AlgorithmBase_updateCanvasAndBlock();
        }
        m++;
    }

    //Na kraju dodajemo prvu tacku na kraj, opet samo zbog iscrtavanja
    _convexHull.push_back(_firstPoint);
    AlgorithmBase_updateCanvasAndBlock();

    emit animationFinished();
}

void GrahamScan::drawAlgorithm(QPainter &painter) const
{
    QPen p = painter.pen();
    p.setWidth(1);
    p.setColor(QColor::fromRgb(128, 128, 128));
    painter.setPen(p);

    //Iscrtavamo sve tacke i povezujemo ih sa prvom tackom
    int i = 1;
    for(QPoint p: _points)
    {
        //double angle = atan2(_firstPoint.y()-p.y(), _firstPoint.x()-p.x());
        //painter.drawText(p, QString::number(i));
       // painter.drawLine(p, _firstPoint);
        painter.drawPoint(p);
        painter.drawLine(_firstPoint, p);
        i++;
    }

    //Najdesniju tacku iscrtavamo zutom bojom
    p.setColor(Qt::yellow);
    p.setWidth(10);
    painter.setPen(p);
    painter.drawPoint(_firstPoint);

    //Crvenom bojom iscrtavamo trenutni sadrzaj omotaca
    p.setColor(Qt::red);
    p.setWidth(3);
    painter.setPen(p);
    int vel = _convexHull.size() - 1;
    for(int i = 0; i<vel; i++)
    {
        painter.drawLine(_convexHull[i], _convexHull[i+1]);
    }
}

void GrahamScan::runNaiveAlgorithm()
{
    ConvexHull::runNaiveAlgorithm();
}


void GrahamScan::popFromHullLastPoint()
{
    _convexHull.pop_back();
}

bool GrahamScan::compare(const QPoint &p1, const QPoint &p2)
{
    double angle1 = atan2(_firstPoint.y() - p1.y(), _firstPoint.x() - p1.x());
    double angle2 = atan2(_firstPoint.y() - p2.y(), _firstPoint.x() - p2.x());

    if(angle1 != angle2)
        return angle1 < angle2;
    /*ako su tacke horizontalne, a y koordinata je minimalna, ali ne ujedno i maksimalna
    //to je slucaj --------
    //             |
    //jer se krecem suprotno od kazaljke na satu (na crtezu, u smeru kazaljke), ali nije i slucaj -------,
    //tad treba da idem s desna na levo i da kupim tacke
    //drugi slucaj je slucaj:
    //    |
    // ---
    // opet isti smer treba da uzmem, ali nije slucaj | jer tad treba da uzmem odozdo na gore tacke */
    else if((angle1 == 0 && _firstIsMinY && !_firstIsMaxY ) || (angle1 > 0 && !_firstIsMinX))
        return utils::distance(p1, _firstPoint) > utils::distance(p2, _firstPoint);
    else
        return utils::distance(p1, _firstPoint) < utils::distance(p2, _firstPoint);
}

std::vector<QPoint> GrahamScan::convexHull() const
{
    return _convexHull;
}

std::vector<QPoint> GrahamScan::convexHullTest() const
{
    return ConvexHull::convexHullTest();
}
