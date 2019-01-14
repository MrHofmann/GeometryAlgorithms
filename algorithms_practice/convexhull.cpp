#include "convexhull.h"

#include <functional>

#include "utils.h"
#include <iostream>

ConvexHull::ConvexHull(QWidget *pRenderer, int delayMs, std::string filename, int inputSize, std::vector<QPoint> points)
    :AlgorithmBase (pRenderer, delayMs)
{
   /* if(filename == "")
        _points = generateRandomPoints(inputSize);
    else
        _points = readPointsFromFile(filename); */

    if(filename == "" && points.empty())
        _points = generateRandomPoints(inputSize);
    else if(filename != "")
        _points = readPointsFromFile(filename);
    else
        for(int i = 0; i < points.size(); i++)
            _points.push_back(points[i]);
}

void ConvexHull::runNaiveAlgorithm()
{
    // check number of points
    if (_points.size() < 3)
    {
        emit animationFinished();
        return;
    }

    //Pronalazak najdesnije tacke
    _firstPoint = _points[0];
    for(QPoint p: _points)
    {
        if(p.x() > _firstPoint.x() || (p.x() == _firstPoint.x() && p.y() > _firstPoint.y()))
        {
            _firstPoint = p;
        }
    }

    int i, j, k;
    int pointsNum = _points.size();
    bool valid;

    for(i=0; i<pointsNum; i++)
    {
        for(j=0; j<pointsNum; j++)
        {
            if(i!=j)
            {
                valid = true;
                for(k=0; k<pointsNum; k++)
                {
                    if(k == i || k == j)
                        continue;
                    if(utils::negativeOrientation(_points[i], _points[j], _points[k], true))
                    {   valid = false;
                        break;
                    }
                }
                if(valid)
                {
                    _convexHullTest.push_back(_points[i]);
                    _convexHullTest.push_back(_points[j]);
                }
            }
        }
    }
    //Sortiranje po uglu
    std::sort(_convexHullTest.begin(), _convexHullTest.end(), [&](const QPoint& lhs, const QPoint& rhs){return compare(lhs, rhs);});

    //Ukloni duplikate
    auto it = std::unique(_convexHullTest.begin(), _convexHullTest.end());
    _convexHullTest.resize(std::distance(_convexHullTest.begin(), it));

    //Ukloni prvu tacku iz sredine i dodaj je na kraj
    auto f = std::find(_convexHullTest.begin(), _convexHullTest.end(), _firstPoint);
    if(f != _convexHullTest.end())
        _convexHullTest.erase(f);
    _convexHullTest.push_back(_firstPoint);
    AlgorithmBase_updateCanvasAndBlock();
}

bool ConvexHull::compare(const QPoint &p1, const QPoint &p2)
{
    double angle1 = atan2(_firstPoint.y() - p1.y(), _firstPoint.x() - p1.x());
    double angle2 = atan2(_firstPoint.y() - p2.y(), _firstPoint.x() - p2.x());

    if(angle1 != angle2)
        return angle1 < angle2;
    else
        return utils::distance(p1, _firstPoint) < utils::distance(p2, _firstPoint);
}

std::vector<QPoint> ConvexHull::convexHullTest() const
{
    return _convexHullTest;
}
