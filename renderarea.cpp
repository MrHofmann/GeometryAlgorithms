#include "renderarea.h"
#include <iostream>

#include "mainwindow.h"
#include "algorithmbase.h"

RenderArea::RenderArea(QWidget *parent)
    : QWidget(parent), _pAlgorithmBase{nullptr}
{}

void RenderArea::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.drawRect(0,0, width()-1, height()-1);

    if (_pAlgorithmBase)
    {
        _pAlgorithmBase->drawAlgorithm(painter);
    }
    else
    {
        //painter.fillRect(0,0,width(), height(), Qt::white);
    }
}

void RenderArea::setPAlgorithmBase(AlgorithmBase *pAlgorithmBase)
{
    _pAlgorithmBase = pAlgorithmBase;
}
