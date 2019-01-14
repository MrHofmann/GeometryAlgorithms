#include "ga00_emptyproject.h"

#include <QPainter>

EmptyProject::EmptyProject(QWidget *pRenderer, int delayMs)
    :AlgorithmBase (pRenderer, delayMs)
{

}

void EmptyProject::runAlgorithm()
{
    emit animationFinished();
}

void EmptyProject::drawAlgorithm(QPainter &painter) const
{
    (void)painter;
}

void EmptyProject::runNaiveAlgorithm()
{

}
