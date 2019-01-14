#ifndef GA00_DRAWPOLYGON_H
#define GA00_DRAWPOLYGON_H

#include <QWidget>

#include "algorithmbase.h"

class DrawPolygon : public AlgorithmBase
{
public:
    DrawPolygon(QWidget *pRenderer, int delayMs, std::string filename = "");


    // AlgorithmBase interface
public:
    void runAlgorithm();
    void drawAlgorithm(QPainter &painter) const;
    void runNaiveAlgorithm();

private:
    std::vector<QPoint> _points;
    int _n; 
};

#endif // GA00_DRAWPOLYGON_H
