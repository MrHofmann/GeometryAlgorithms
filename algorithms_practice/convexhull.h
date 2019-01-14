#ifndef CONVEXHULL_H
#define CONVEXHULL_H

#include "algorithmbase.h"

#include <set>
#include <functional>

class ConvexHull : public AlgorithmBase
{
public:
    ConvexHull(QWidget *pRenderer, int delayMs, std::string filename = "", int inputSize = DEFAULT_POINTS_NUM, std::vector<QPoint> points = std::vector<QPoint>());

public:
    void runNaiveAlgorithm();
    std::vector<QPoint> convexHullTest() const;

protected:
    /* Ulazni podaci */
    std::vector<QPoint> _points;

private:
    /* Najdesnija tacka */
    QPoint _firstPoint;
    /* Konveksni omotac izracunat algoritmom grube sile */
    std::vector<QPoint> _convexHullTest;
    bool compare(const QPoint& p1, const QPoint& p2);
};

#endif // CONVEXHULL_H
