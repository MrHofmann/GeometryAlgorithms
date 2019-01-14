#include "ga00_drawpolygon.h"

DrawPolygon::DrawPolygon(QWidget *pRenderer, int delayMs, std::string filename)
    :AlgorithmBase{pRenderer, delayMs}, _n(0)
{
    if(filename == "")
        _points = generateRandomPoints();
    else
        _points = readPointsFromFile(filename);
}

void DrawPolygon::runAlgorithm()
{
    while(_n++ != _points.size())
    {
        AlgorithmBase_updateCanvasAndBlock();
    }

    emit animationFinished();
}

void DrawPolygon::drawAlgorithm(QPainter &painter) const
{
    QPen p = painter.pen();
    p.setColor(Qt::red);
    painter.setPen(p);

    painter.drawPolygon(_points.data(), _n);
}

void DrawPolygon::runNaiveAlgorithm()
{

}
