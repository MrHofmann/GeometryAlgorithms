#include <QPainter>
#include <QDebug>

#include "ga05_quickhull.h"
#include "utils.h"

//-------------------------------------------------------------
// QuickHull
//-------------------------------------------------------------
QuickHull::QuickHull(QWidget *pRenderer, int delayMs, std::string filename, int inputSize)
    : ConvexHull(pRenderer, delayMs, filename, inputSize),
      _findingMaxPointIndicator(false)
{}

//-------------------------------------------------------------
// findHull
//-------------------------------------------------------------
bool QuickHull::findHull(const QPoint &p1, const QPoint &p2, const std::vector<const QPoint*> &points, bool canDeleteLine)
{
    _minPoint = &p1;
    _maxPoint = &p2;
    double maxDistance = 0;         // maksimalno rastojanje tacke od prave
    double currentDistance = 0;     // trenutno rastojanje tacke od prave
    int maxPointIndex = NOT_FOUND;  // indeks pronadjene najudaljenije tacke od prave
    bool hasPoints = false;         // indikator koji oznacava da li je skup tacaka koji se razmatra prazan ili neprazan

    // pronalazenje najudaljenije tacke od prave
    _findingMaxPointIndicator = true;
    for (unsigned int i = 0; i < points.size(); ++i)
    {
        hasPoints = true;
        _pointToHighlight = points[i];
        currentDistance = utils::calculateDistanceFromLine(*points[i], p1, p2);

        if (maxDistance < currentDistance)
        {
            maxDistance = currentDistance;
            maxPointIndex = i;
        }

        updateCanvas();
        if (_destroyAnimation)
            return false;
    }
    _findingMaxPointIndicator = false;

    // izlaz iz rekurzije
    if (hasPoints == false)
        return false;

    // dodavanje tacke koja je najudaljenija od prave u konveksni omotac
    _convexHull.push_back(*points[maxPointIndex]);

    // brisanje ivica koje vise nisu deo trenutnog konveksnog omotaca
    if (canDeleteLine)
    {
        for (unsigned int i = 0; i < _pointDrawVector.size(); ++i)
        {
            if (_pointDrawVector[i].first == p1 && _pointDrawVector[i].second == p2)
            {
                _pointDrawVector.erase(_pointDrawVector.begin() + i);
                break;
            }
        }
    }

    // iscrtavanje ivica trenutnog konveksnog omotaca
    _pointDrawVector.push_back(std::make_pair(p1, *points[maxPointIndex]));
    updateCanvas();
    if (_destroyAnimation)
        return false;

    _pointDrawVector.push_back(std::make_pair(*points[maxPointIndex], p2));
    updateCanvas();
    if (_destroyAnimation)
        return false;

    // particionisanje skupa tacaka u zavisnosti od njihove pozicije
    std::vector<const QPoint*> leftSidePoints;
    std::vector<const QPoint*> rightSidePoints;
    std::vector<const QPoint*> pointsOnLeftLine;
    std::vector<const QPoint*> pointsOnRightLine;
    for (const QPoint *p : points)
    {
        if (*p == *points[maxPointIndex])
            continue;

        short side1 = utils::getPointSide(*p, p1, *points[maxPointIndex]);
        short side2 = utils::getPointSide(*p, *points[maxPointIndex], p2);

        if (side1 == LEFT_SIDE)
        {
            leftSidePoints.push_back(p);
        }
        else if (side2 == LEFT_SIDE)
        {
            rightSidePoints.push_back(p);
        }
        else if (side1 == ON_THE_LINE)
        {
            pointsOnLeftLine.push_back(p);
        }
        else if (side2 == ON_THE_LINE)
        {
            pointsOnRightLine.push_back(p);
        }
    }

    // rekurzivni poziv za levu stranu
    bool leftSideStatus = findHull(p1, *points[maxPointIndex], leftSidePoints, true);
    if (_destroyAnimation)
        return false;

    // rekurzivni poziv za desnu stranu
    bool rightSideStatus = findHull(*points[maxPointIndex], p2, rightSidePoints, true);
    if (_destroyAnimation)
        return false;

    // potencijalno dodavanje tacaka koje leze na granici konveksnog omotaca
    if (leftSideStatus == false)
        for (const QPoint *p : pointsOnLeftLine)
            _convexHull.push_back(*p);

    if (rightSideStatus == false)
        for (const QPoint *p : pointsOnRightLine)
            _convexHull.push_back(*p);

    return hasPoints;
}

//-------------------------------------------------------------
// runAlgorithm
//-------------------------------------------------------------
void QuickHull::runAlgorithm()
{
    // provera broja tacaka
    if (_points.size() < 3)
    {
        emit animationFinished();
        return;
    }

    // pronalazenje tacke sa najmanjom x koordinatom
    std::vector<QPoint>::iterator minPoint = std::min_element(_points.begin(), _points.end(), utils::compareForLexicographicSort);
    // pronalazenje tacke sa najvecom x koordinatom
    std::vector<QPoint>::iterator maxPoint = std::max_element(_points.begin(), _points.end(), utils::compareForLexicographicSort);
    // dodavanje pronadjenih tacaka u konveksni omotac
    _convexHull.push_back(*minPoint);
    _convexHull.push_back(*maxPoint);
    _pointDrawVector.push_back(std::make_pair(*maxPoint, *minPoint));

    // particionisanje skupa tacaka u zavisnosti od njihove pozicije
    std::vector<const QPoint*> topSidePoints;
    std::vector<const QPoint*> bottomSidePoints;
    std::vector<const QPoint*> onTheLinePoints;
    for (const QPoint &p : _points)
    {
        if (p == *minPoint || p == *maxPoint)
            continue;

        short side = utils::getPointSide(p, *minPoint, *maxPoint);

        if (side == TOP_SIDE)
        {
            topSidePoints.push_back(&p);
        }
        else if (side == BOTTOM_SIDE)
        {
            bottomSidePoints.push_back(&p);
        }
        else if (side == ON_THE_LINE)
        {
            onTheLinePoints.push_back(&p);
        }
    }

    // rekurzivni poziv za gornji podskup tacaka
    bool topSideStatus = findHull(*minPoint, *maxPoint, topSidePoints, true);
    if (_destroyAnimation)
        return;

    // rekurzivni poziv za donji podskup tacaka
    bool bottomSideStatus = findHull(*maxPoint, *minPoint, bottomSidePoints, topSidePoints.size() == 0 ? false : true);
    if (_destroyAnimation)
        return;

    // potencijalno dodavanje tacaka koje leze na granici konveksnog omotaca
    if (topSideStatus == false || bottomSideStatus == false)
        for (const QPoint *p : onTheLinePoints)
            _convexHull.push_back(*p);

    // kraj animacije
    emit animationFinished();
}

//-------------------------------------------------------------
// drawAlgorithm
//-------------------------------------------------------------
void QuickHull::drawAlgorithm(QPainter &painter) const
{
    // podesavanje painter-a
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPen p = painter.pen();

    // iscrtavanje trenutnog konveksnog omotaca
    p.setWidth(2);
    p.setColor(Qt::darkBlue);
    painter.setPen(p);
    for (unsigned int i = 0; i < _pointDrawVector.size(); i++)
        painter.drawLine(_pointDrawVector[i].first, _pointDrawVector[i].second);

    // animiranje trazenja najudaljenije tacke
    if (_findingMaxPointIndicator == true)
    {
        p.setWidth(2);
        p.setColor(Qt::darkGray);
        p.setStyle(Qt::DotLine);
        painter.setPen(p);
        painter.drawLine(utils::getPointOnLine(_pointToHighlight, _minPoint, _maxPoint), *_pointToHighlight);

        p.setWidth(15);
        p.setCapStyle(Qt::RoundCap);
        p.setColor(Qt::red);
        painter.setPen(p);
        painter.drawPoint(*_pointToHighlight);
    }

    // iscrtavanje tacaka i polaznog skupa
    p.setWidth(7);
    p.setCapStyle(Qt::RoundCap);
    p.setColor(Qt::darkRed);
    painter.setPen(p);
    for (const QPoint &pt : _points)
        painter.drawPoint(pt);
}

//-------------------------------------------------------------
// runNaiveAlgorithm
//-------------------------------------------------------------
void QuickHull::runNaiveAlgorithm()
{
    ConvexHull::runNaiveAlgorithm();
}

//-------------------------------------------------------------
// getConvexHull
//-------------------------------------------------------------
std::vector<QPoint> QuickHull::getConvexHull() const
{
    return _convexHull;
}

//-------------------------------------------------------------
// getConvexHullTest
//-------------------------------------------------------------
std::vector<QPoint> QuickHull::getConvexHullTest() const
{
    return ConvexHull::convexHullTest();
}

//-------------------------------------------------------------
// updateCanvas
//-------------------------------------------------------------
void QuickHull::updateCanvas()
{
    AlgorithmBase_updateCanvasAndBlock();
}
