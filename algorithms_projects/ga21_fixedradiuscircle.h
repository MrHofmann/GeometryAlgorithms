/*
Autor: Luka Živanović 1086/2017
Godina: 2017
Kratak opis problema: Dato je n tačka u ravni i prečnik kruga. Potrebno je odrediti maksimalni broj tačaka
                      koje se nalaze u proizvoljno pozicioniranom krugu datog prečnika.
*/

#ifndef GA021_FIXEDRADIUSCIRCLE_H
#define GA021_FIXEDRADIUSCIRCLE_H

#include <QWidget>
#include <QPainter>

#include "utils.h"

#include "algorithmbase.h"

#define ENTER true
#define LEAVE false

class FixedRadiusCircle : public AlgorithmBase
{
public:
    FixedRadiusCircle(QWidget *pRenderer, int delayMs, int radius, std::string filename, int inputSize = DEFAULT_POINTS_NUM);

public:
    void runAlgorithm();
    void drawAlgorithm(QPainter &painter) const;
    void runNaiveAlgorithm();

    void setPoints(const std::vector<QPoint> &points);
    int result() const;

private:
    int _radius;
    std::vector<QPoint> _points;

    QPoint _currentPoint;
    QPoint _referentPoint;

    QPoint _currentCircle;
    QPoint _currentMaxCircle;
    QPoint _globalMaxCircle;

    int _globalMaxCount;
    int _currentCount;
    int _currentMaxCount;

    double _currentAngle;

    bool _enterLeave;
};

#endif // GA021_FIXEDRADIUSCIRCLE_H
