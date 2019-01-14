#include "ga01_sweepline.h"

SweepLine::SweepLine(QWidget *pRenderer, int delayMs, std::string filename)
    :AlgorithmBase{pRenderer, delayMs}, _yPos{0}
{
    if(filename == "")
        _points = generateRandomPoints();
    else
        _points = readPointsFromFile(filename);
}

void SweepLine::runAlgorithm()
{
    while (_yPos < _pRenderer->height()) {
        _yPos+=5;
        AlgorithmBase_updateCanvasAndBlock();
    }

    emit animationFinished();
}

void SweepLine::drawAlgorithm(QPainter &painter) const
{
    QPen p = painter.pen();
    p.setColor(Qt::green);
    painter.setPen(p);
    painter.drawLine(0, _yPos, _pRenderer->width(), _yPos);

    p.setWidth(5);
    for(const QPoint& pt : _points)
    {
        if(pt.y() < _yPos)
            p.setColor(Qt::red);
        else
            p.setColor(Qt::blue);


        painter.setPen(p);
        painter.drawPoint(pt);
    }
}

void SweepLine::runNaiveAlgorithm()
{

}
