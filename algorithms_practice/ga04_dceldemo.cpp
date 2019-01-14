#include "ga04_dceldemo.h"
#include "ga04_dcel.h"

DCELDemo::DCELDemo(QWidget *pRenderer, int delayMs, std::string filename)
    : AlgorithmBase (pRenderer, delayMs), _polygon(filename)
{
}

void DCELDemo::runAlgorithm()
{
    AlgorithmBase_updateCanvasAndBlock();
    emit animationFinished();
}

void DCELDemo::drawAlgorithm(QPainter &painter) const
{
    QPen pen = painter.pen();

    for(int i=0; i<_polygon.edges().size(); i+=2)
    {

        pen.setColor(Qt::yellow);
        painter.setPen(pen);
        painter.drawLine(_polygon.edges()[i]->origin()->coordinates(),
                         _polygon.edges()[i]->next()->origin()->coordinates());


        pen.setColor(Qt::red);
        painter.setPen(pen);
        painter.drawLine(_polygon.edges()[i]->twin()->origin()->coordinates().x() + 5,
                         _polygon.edges()[i]->twin()->origin()->coordinates().y() + 5,
                         _polygon.edges()[i]->twin()->next()->origin()->coordinates().x() - 5,
                         _polygon.edges()[i]->twin()->next()->origin()->coordinates().y() + 5);
   }

    int curr_num = 1;
    painter.setBrush(Qt::red);
    painter.setPen(Qt::white);
    for(DCELVertex* v: _polygon.vertices())
    {
       painter.drawEllipse(v->coordinates(), 10, 10);
       painter.drawText(v->coordinates().x() - 4, v->coordinates().y() + 4, QString::fromStdString(std::to_string(curr_num)));
       curr_num++;
    }
}

void DCELDemo::runNaiveAlgorithm()
{

}
