/*
Autor:
Godina: 2018
Kratak opis problema:
*/

#ifndef GA00_EMPTYPROJECT_H
#define GA00_EMPTYPROJECT_H

#include "algorithmbase.h"

class EmptyProject : public AlgorithmBase
{
public:
    EmptyProject(QWidget *pRenderer, int delayMs);

    // AlgorithmBase interface
public:
    void runAlgorithm();
    void drawAlgorithm(QPainter &painter) const;
    void runNaiveAlgorithm();
};

#endif // GA00_EMPTYPROJECT_H
