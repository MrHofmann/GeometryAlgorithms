#include <QPainter>
#include <QDebug>
#include "ga05_incrementalinsertion.h"
#include "utils.h"


//-------------------------------------------------------------
// IncrementalInsertion
//-------------------------------------------------------------
IncrementalInsertion::IncrementalInsertion(QWidget *pRenderer, int delayMs, std::string filename, int inputSize)
    :  ConvexHull(pRenderer, delayMs, filename, inputSize),
      _xPositionOfSweepline(0), _numberOfProcessedPoints(0)
{}

//-------------------------------------------------------------
// runAlgorithm
//-------------------------------------------------------------
void IncrementalInsertion::runAlgorithm()
{
    // provera broja tacaka
    if (_points.size() < 3)
    {
        emit animationFinished();
        return;
    }

    // sortiranje tacaka
    std::sort(_points.begin(), _points.end(), [&](const QPoint& lhs, const QPoint& rhs){ return utils::compareForLexicographicSort(lhs, rhs); });
    // eliminacija duplikata
    auto it = std::unique(_points.begin(), _points.end());
    _points.resize(std::distance(_points.begin(), it));

    // kreiranje skupa kompleksnih tacaka
    for (const QPoint &p : _points)
        _complexPoints.push_back(Point(p));

    // prikazivanje tacaka na platnu
    AlgorithmBase_updateCanvasAndBlock();

    // odredjivanje strane na kojoj se nalazi treca tacka, u odnosu na pravu koja prolazi kroz prve dve tacke
    short side = utils::getPointSide(_complexPoints[2].getValue(), _complexPoints[0].getValue(), _complexPoints[1].getValue());

    if (side == LEFT_SIDE)          // treca tacka se nalazi sa leve strane
    {
        makeTriangle(&_complexPoints[0], &_complexPoints[1], &_complexPoints[2]);
    }
    else if (side == RIGHT_SIDE)    // treca tacka se nalazi sa desne strane
    {
        makeTriangle(&_complexPoints[0], &_complexPoints[2], &_complexPoints[1]);
    }
    else if (side == ON_THE_LINE)    // sve tri tacke su kolinearne
    {
        makeTriangle(&_complexPoints[0], &_complexPoints[1], &_complexPoints[2]);
        _collinearIndicator = true;
    }

    // azuriranje broja obradjenih tacaka
    _numberOfProcessedPoints = 3;

    // obrada preostalih tacaka
    for (unsigned int i = 3; i < _complexPoints.size(); ++i)
    {
        // azuriranje broja obradjenih tacaka i pozicije brisuce prave
        ++_numberOfProcessedPoints;
        _xPositionOfSweepline = _complexPoints[i].getValue().x();

        // inicijalizacija gornje i donje potporne tacke
        _topPoint = &_complexPoints[i-1];
        _bottomPoint = _topPoint;
        _currentPoint = &_complexPoints[i];

        // obrada speijalnog slucaja gde su tacke kolinearne
        if (_collinearIndicator)
        {
            // odredjivanje strane na kojoj se nalazi tacka koju trenutno obradjujemo,
            // u odnosu na pravu koja prolazi kroz prethodne dve tacke
            short side = utils::getPointSide(_complexPoints[i].getValue(), _complexPoints[i-1].getValue(), _complexPoints[i-1].getPrevPoint()->getValue());

            if (side == ON_THE_LINE)        // tacka koju trenutno obradjujemo lezi na pravoj
            {
                _topPoint = _topPoint->getNextPoint();
            }
            else if (side == LEFT_SIDE)     // tacka koju trenutno obradjujemo se nalazi sa leve strane
            {
                reverseList();
                _collinearIndicator = false;
            }
            else if (side == RIGHT_SIDE)    // tacka koju trenutno obradjujemo se nalazi sa desne strane
            {
                _collinearIndicator = false;
            }
        }

        // trazenje gornje potporne prave
        while (utils::getPointSide(_complexPoints[i].getValue(), _topPoint->getValue(), _topPoint->getNextPoint()->getValue()) == RIGHT_SIDE)
        {
            _topPoint = _topPoint->getNextPoint();
            AlgorithmBase_updateCanvasAndBlock();
        }

        // trazenje donje potporne prave
        while (utils::getPointSide(_complexPoints[i].getValue(), _bottomPoint->getValue(), _bottomPoint->getPrevPoint()->getValue()) == LEFT_SIDE)
        {
            _bottomPoint = _bottomPoint->getPrevPoint();
            AlgorithmBase_updateCanvasAndBlock();
        }

        // azuriranje trenutnog konveksnog omotaca
        _complexPoints[i].setNextPoint(_topPoint);
        _complexPoints[i].setPrevPoint(_bottomPoint);
        _topPoint->setPrevPoint(&_complexPoints[i]);
        _bottomPoint->setNextPoint(&_complexPoints[i]);

        AlgorithmBase_updateCanvasAndBlock();
    }

    // brisanje potpornih linija, potpornih tacaka i brisuce prave sa platna
    _topPoint = _bottomPoint = nullptr;
    _xPositionOfSweepline = 0;
    AlgorithmBase_updateCanvasAndBlock();

    // kreiranje vektora u kom se nalaze tacke koje pripadaju konveksnom omotacu
    const Point *startPoint = &(_complexPoints[0]);
    _convexHull.push_back(startPoint->getValue());
    const Point *currentPoint = startPoint;
    while (startPoint->getValue() != currentPoint->getNextPoint()->getValue())
    {
        currentPoint = currentPoint->getNextPoint();
        _convexHull.push_back(currentPoint->getValue());
    }

    // kraj animacije
    emit animationFinished();
}

//-------------------------------------------------------------
// drawAlgorithm
//-------------------------------------------------------------
void IncrementalInsertion::drawAlgorithm(QPainter &painter) const
{
    // podesavanje painter-a
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPen p = painter.pen();

    // iscrtavanje brisuce prave
    p.setWidth(1);
    p.setColor(Qt::darkGray);
    painter.setPen(p);
    painter.drawLine(_xPositionOfSweepline, 0, _xPositionOfSweepline, _pRenderer->height());

    // iscrtavanje trenutnog konveksnog omotaca
    p.setWidth(2);
    p.setColor(Qt::darkBlue);
    painter.setPen(p);
    const Point *currentPoint = &(_complexPoints[0]);
    if (_numberOfProcessedPoints > 1)
    {
        for (unsigned int i = 0; i < _numberOfProcessedPoints; ++i)
        {
            painter.drawLine(currentPoint->getValue(), currentPoint->getNextPoint()->getValue());
            currentPoint = currentPoint->getNextPoint();
        }
    }

    // iscrtavanje potpornih pravih
    if (_topPoint != nullptr && _bottomPoint != nullptr)
    {
        // iscrtavanje pravougaonika koji pokriva tacke koje trenutno nisu obradjene
        QRect r(QPoint(_xPositionOfSweepline+5, 0), QPoint(2000, 2000));
        painter.fillRect(r, QBrush(QColor(200, 200, 200, 100)));

        // iscrtavanje labele
        p.setColor(Qt::black);
        painter.setPen(p);
        painter.drawText(QPoint(_xPositionOfSweepline, 20), QString("   sweep line   "));

        // iscrtavanje gornje potporne tacke
        p.setWidth(18);
        p.setCapStyle(Qt::RoundCap);
        p.setColor(Qt::green);
        painter.setPen(p);
        painter.drawPoint(_topPoint->getValue());

        // iscrtavanje gornje potporne prave
        p.setWidth(2);
        p.setColor(Qt::green);
        p.setStyle(Qt::DotLine);
        painter.setPen(p);
        painter.drawLine(_currentPoint->getValue(), _topPoint->getValue());

        // iscrtavanje labele
        p.setColor(Qt::black);
        painter.setPen(p);
        painter.drawText((_topPoint->getValue() + _currentPoint->getValue())/2, QString("   top support   "));

        // iscrtavanje donje potporne tacke
        p.setWidth(18);
        p.setCapStyle(Qt::RoundCap);
        p.setColor(Qt::red);
        painter.setPen(p);
        painter.drawPoint(_bottomPoint->getValue());

        // iscrtavanje donje potporne prave
        p.setWidth(2);
        p.setColor(Qt::red);
        p.setStyle(Qt::DotLine);
        painter.setPen(p);
        painter.drawLine(_currentPoint->getValue(), _bottomPoint->getValue());

        // iscrtavanje labele
        p.setColor(Qt::black);
        painter.setPen(p);
        painter.drawText((_bottomPoint->getValue() + _currentPoint->getValue())/2, QString("   bottom support   "));
    }

    // iscrtavanje pocetnog skupa tacaka
    p.setWidth(7);
    p.setCapStyle(Qt::RoundCap);
    p.setColor(Qt::darkRed);
    painter.setPen(p);
    for (const QPoint &pt : _points)
    {
        painter.drawPoint(pt);
    }
}

//-------------------------------------------------------------
// runNaiveAlgorithm
//-------------------------------------------------------------
void IncrementalInsertion::runNaiveAlgorithm()
{
    ConvexHull::runNaiveAlgorithm();
}

//-------------------------------------------------------------
// makeTriangle
//-------------------------------------------------------------
void IncrementalInsertion::makeTriangle(Point *p1, Point *p2, Point *p3)
{
    p1->setNextPoint(p2);
    p1->setPrevPoint(p3);

    p2->setNextPoint(p3);
    p2->setPrevPoint(p1);

    p3->setNextPoint(p1);
    p3->setPrevPoint(p2);
}

//-------------------------------------------------------------
// reverseList
//-------------------------------------------------------------
void IncrementalInsertion::reverseList()
{
    Point *current = &_complexPoints[0];
    Point *temp;

    do
    {
        temp = current->getNextPoint();
        current->setNextPoint(current->getPrevPoint());
        current->setPrevPoint(temp);
        current = temp;
    } while (current != &_complexPoints[0]);
}

//-------------------------------------------------------------
// getConvexHull
//-------------------------------------------------------------
std::vector<QPoint> IncrementalInsertion::getConvexHull() const
{
    return _convexHull;
}

//-------------------------------------------------------------
// getConvexHullTest
//-------------------------------------------------------------
std::vector<QPoint> IncrementalInsertion::getConvexHullTest() const
{
    return ConvexHull::convexHullTest();
}
