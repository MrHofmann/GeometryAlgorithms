#ifndef GA04_DCELDEMO_H
#define GA04_DCELDEMO_H

#include "algorithmbase.h"
#include "ga04_dcel.h"

class DCELDemo : public AlgorithmBase
{
public:
    DCELDemo(QWidget *pRenderer, int delayMs, std::string filename);

private:
    DCEL _polygon;

    // AlgorithmBase interface
public:
    void runAlgorithm();
    void drawAlgorithm(QPainter &painter) const;
    void runNaiveAlgorithm();
};

#endif // GA04_DCELDEMO_H
