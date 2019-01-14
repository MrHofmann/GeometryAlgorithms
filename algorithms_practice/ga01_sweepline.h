#ifndef GA01_SIMULATESWEEPLINE_H
#define GA01_SIMULATESWEEPLINE_H

#include "algorithmbase.h"

#include <vector>

class SweepLine : public AlgorithmBase
{
public:
    SweepLine(QWidget* pRenderer, int delayMs, std::string filename = "");

    // AlgorithmBase interface
public:
    void runAlgorithm();
    void drawAlgorithm(QPainter &painter) const;
    void runNaiveAlgorithm();

private:
    int _yPos;
    std::vector<QPoint> _points;
};

#endif // GA01_SIMULATESWEEPLINE_H
