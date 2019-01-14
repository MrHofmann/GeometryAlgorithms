#include "ga18_smallestenclosingdisk.h"

#include <limits>
#include <QDebug>
#include <algorithm>
#include <algorithms_practice/ga02_grahamscan.h>
#include <random>

#include "utils.h"


namespace
{
    const double pi__ = 3.1415926535898;
}


MinEnclosingCircle::MinEnclosingCircle(
        QWidget *pRenderer, int delayMs, std::string filename, int inputSize )
        :AlgorithmBase (pRenderer, delayMs), n(0), phase(0), InsertPointIndex(0),
          ph2InsertPointIndex(0), ph3InsertPointIndex(0),
          ph2MinCircle(QPointF(-1,-1), -1, std::numeric_limits<double>::max()),
          ph3MinCircle(QPointF(-1,-1), -1, std::numeric_limits<double>::max()),
          minCircle(QPointF(-1,-1), -1, std::numeric_limits<double>::max())
{
    std::vector<QPoint> pointInt;
    if(filename == "")
        pointInt = generateRandomPointsF(inputSize);
    else
        pointInt = readPointsFromFile(filename);
    for ( auto p : pointInt){
        points.emplace_back(p);
    }
}


MinEnclosingCircle::MinEnclosingCircle(
        QWidget *pRenderer, int delayMs, std::vector<QPointF> newPoints )
        :AlgorithmBase (pRenderer, delayMs), n(0), phase(0), InsertPointIndex(0),
          ph2InsertPointIndex(0), ph3InsertPointIndex(0),
          ph2MinCircle(QPointF(-1,-1), -1, std::numeric_limits<double>::max()),
          ph3MinCircle(QPointF(-1,-1), -1, std::numeric_limits<double>::max()),
          minCircle(QPointF(-1,-1), -1, std::numeric_limits<double>::max())
{
    for ( auto p : newPoints){
        points.emplace_back(p);
    }
}


void MinEnclosingCircle::runAlgorithm()
{
    AlgorithmBase_updateCanvasAndBlock();
    if ( !minDisk() ){
        return;
    }

    AlgorithmBase_updateCanvasAndBlock();

    emit animationFinished();
}


void MinEnclosingCircle::drawAlgorithm(QPainter &painter) const
{
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);

    static QColor greenTint = QColor(128,255,200,20);
    static QColor ph2GreenTint = QColor(128,233,233,30);
    static QColor ph3GreenTint = QColor(128,200,255,40);

    changePen(painter, phase*phase*phase*4, Qt::black);
    static QPointF center = QPointF(QPointF(_pRenderer->width()/2, _pRenderer->height()/2));
    painter.drawEllipse(center, _pRenderer->width()/2 + 30, _pRenderer->width()/2 + 30);

    if ( phase == 0 ){
        changePen(painter, 3, Qt::black);
        for ( int i = 0; i < points.size(); ++i){
            painter.drawPoint(points[i]);
        }
    }
    else if (phase == 1 ){
        drawDisk( painter, minCircle, greenTint, 2, Qt::black );

        // draw all unprocessed points
        drawFirstNPoints( painter, points.size(), 3, Qt::gray);

        // draw processed points
        drawFirstNPoints( painter, InsertPointIndex, 5, Qt::black);

        // draw the point which is being inserted
        drawInsertedPoint(painter, InsertPointIndex, minCircle);
        if ( InsertPointIndex == 1 ){
            drawInsertedPoint(painter, 0, minCircle);
        }

    }
    else if (phase == 2 ){
        drawDisk( painter, ph2MinCircle, ph2GreenTint, 2, Qt::black );

        // draw all unprocessed points
        drawFirstNPoints( painter, InsertPointIndex, 3, Qt::gray);

        // draw processed points
        drawFirstNPoints( painter, ph2InsertPointIndex, 5, Qt::black);

        // draw the point which is being inserted
        drawInsertedPoint(painter, ph2InsertPointIndex, ph2MinCircle);

        // draw fixed point
        changePen(painter, 6, Qt::blue);
        painter.drawPoint(points[InsertPointIndex]);

    }
    else if (phase == 3 ){
        drawDisk( painter, ph3MinCircle, ph3GreenTint, 2, Qt::black );

        // draw all points
        drawFirstNPoints( painter, ph2InsertPointIndex, 3, Qt::gray);

        //draw other points that were inserted
        drawFirstNPoints( painter, ph3InsertPointIndex, 5, Qt::black);

        // draw the point which is being inserted
        if ( ph3InsertPointIndex >= 0 ){
            drawInsertedPoint(painter, ph3InsertPointIndex, ph3MinCircle);
        }

        // draw fixed points
        changePen(painter, 6, Qt::blue);
        painter.drawPoint(points[InsertPointIndex]);
        painter.drawPoint(points[ph2InsertPointIndex]);
    }
}


void MinEnclosingCircle::drawDisk( QPainter &painter, Circle disk, QColor brushTint, int penWidth, QColor penColor) const
{
    changePen( painter, penWidth, penColor );
    painter.setBrush(brushTint);
    painter.drawEllipse(disk.center, disk.r, disk.r);
    painter.setBrush(Qt::white);
}


void MinEnclosingCircle::drawFirstNPoints( QPainter &painter, int n, int penWidth, QColor penColor ) const
{
    if( n < 0 ){
        return;
    }
    std::size_t lastPoint = static_cast<std::size_t>(n);

    changePen( painter, penWidth, penColor );
    for ( std::size_t i = 0; i < lastPoint; ++i){
        painter.drawPoint(points[i]);
    }
}


void MinEnclosingCircle::drawInsertedPoint( QPainter &painter, std::size_t pointIndex, Circle disk,
                        int penWidth, QColor inCol, QColor outCol) const
{
    if ( disk.containsPoint(points[pointIndex]) ){
        changePen(painter, penWidth, inCol);
    }
    else{
        changePen(painter, penWidth, outCol);
    }
    painter.drawPoint(points[pointIndex]);

    QPen p = painter.pen();
    p.setWidth(1);
    painter.setPen(p);
    painter.drawLine(disk.center, points[pointIndex]);
}


void MinEnclosingCircle::changePen( QPainter &painter, int penWidth, QColor penColor) const
{
    QPen p = painter.pen();
    p.setColor(penColor);
    p.setWidth(penWidth);
    painter.setPen(p);
}


bool MinEnclosingCircle::minDisk( )
{
    clearResult();
    srand (time(NULL));
    std::random_shuffle(points.begin(), points.end());
    phase = 1;
    if ( points.size() < 2 ){
        return false;
    }
    minCircle = Circle(points[0], points[1]);
    InsertPointIndex = 1;

    if (points.size() == 2){
        drawMe_updateCanvasAndBlock();
        return true;
    }

    drawMe_updateCanvasAndBlock();

    for ( std::size_t i = 2; i < points.size(); ++i ){
        phase = 1;
        InsertPointIndex = i;
        drawMe_updateCanvasAndBlock();

        if ( !minCircle.containsPoint(points[InsertPointIndex]) ){
            if ( !minDisk(i) ){
                return false;
            }
            minCircle = ph2MinCircle;
            phase = 1;
            drawMe_updateCanvasAndBlock();
        }
    }

    return true;
}


bool MinEnclosingCircle::minDisk( unsigned int p1_index )
{
    phase = 2;
    std::random_shuffle(points.begin(), points.begin()+p1_index);

    ph2MinCircle = Circle(points[p1_index], points[0]);
    ph2InsertPointIndex = 0;
    drawMe_updateCanvasAndBlock();

    for ( std::size_t i = 1; i < p1_index; ++i ){
        phase = 2;
        ph2InsertPointIndex = i;
        drawMe_updateCanvasAndBlock();

        if ( !ph2MinCircle.containsPoint(points[ph2InsertPointIndex]) ){
            if ( !minDisk(p1_index, i) ){
                return false;
            }
            ph2MinCircle = ph3MinCircle;
            phase = 2;
            drawMe_updateCanvasAndBlock();
        }
    }

    return true;
}


bool MinEnclosingCircle::minDisk( unsigned int p1_index, unsigned int p2_index )
{
    phase = 3;
    std::random_shuffle(points.begin(), points.begin()+p2_index);

    ph3MinCircle = Circle(points[p1_index], points[p2_index]);
    ph3InsertPointIndex = -1;
    drawMe_updateCanvasAndBlock();

    for ( std::size_t i = 0; i < p2_index; ++i ){
        phase = 3;
        ph3InsertPointIndex = i;
        drawMe_updateCanvasAndBlock();

        if ( !ph3MinCircle.containsPoint(points[ph3InsertPointIndex]) ){
            ph3MinCircle = Circle(points[p1_index], points[p2_index], points[i]);
            drawMe_updateCanvasAndBlock();
        }
    }

    return true;
}


void MinEnclosingCircle::runNaiveAlgorithm()
{
    clearResult();
    std::vector<QPoint> grahPoints = std::vector<QPoint>();
    for ( auto i : points ){
        grahPoints.emplace_back(QPoint(i.x(), i.y()));
    }
    GrahamScan grahAlg(nullptr, 0, "", 0, grahPoints);
    grahAlg.runAlgorithm();
    std::vector<QPointF> _points = std::vector<QPointF>();
    for ( auto i : grahAlg.convexHull() ){
        _points.emplace_back(i);
    }

    for ( unsigned i = 0; i < _points.size() - 1; ++i ){
        for (unsigned j = i+1; j < _points.size(); ++j) {
            Circle currCircle(_points[i], _points[j]);
            if ( currCircle.area < minCircle.area && circleEnclosesPoints(currCircle) ){
                minCircle = Circle(currCircle);
            }
        }
    }
    for ( unsigned i = 0; i < _points.size()-2; ++i){
        for (unsigned j = i+1; j < _points.size()-1; ++j) {
            for (unsigned k = j+1; k < _points.size(); ++k) {
                Circle currCircle = Circle(_points[i], _points[j], _points[k]);
                if ( currCircle.area < minCircle.area && circleEnclosesPoints(currCircle) ){
                    minCircle = Circle(currCircle);
                }
            }
        }
    }
}


Circle MinEnclosingCircle::result() const
{
    return minCircle;
}


void MinEnclosingCircle::clearResult()
{
    minCircle = Circle(QPointF(-1,-1), 0, std::numeric_limits<double>::max() );
}


Circle::Circle( const QPointF p1_, const QPointF p2_, const QPointF p3_ )
{
    QPointF alpha = QPointF(0.000001, 0.000001);
    QPointF p1 = QPointF(p1_) + alpha;
    QPointF p2 = QPointF(p2_);
    QPointF p3 = QPointF(p3_) - alpha;

    double ma = ( p2.y() - p1.y() ) / ( p2.x() - p1.x() );
    double mb = ( p3.y() - p2.y() ) / ( p3.x() - p2.x() );
    double cenX = ( ma * mb * ( p1.y() - p3.y() )
            + mb * ( p1.x() + p2.x() )
            - ma * ( p2.x() + p3.x() ) )
            / ( 2.0 * ( mb - ma ) );
    double cenY = -1 * ( cenX - ( p1.x() + p2.x() ) / 2.0 ) / ma
            + ( p1.y() + p2.y() ) / 2.0;
    center = QPointF( cenX, cenY );
    r = utils::distance2(center,p1);
    area = r*r*pi__;
}


Circle::Circle( const QPointF p1, const QPointF p2)
{
    center = (p1+p2)/2;
    r = utils::distance2(center,p1);
    area = r*r*pi__;
}


Circle& Circle::operator=( const Circle other )
{
    center.rx() = other.center.x();
    center.ry() = other.center.y();
    r = other.r;
    area = other.area;
    return *this;
}


bool Circle::containsPoint(QPointF p) const
{
    double alpha = 0.1;
    if ( utils::distance2(this->center, p) > this->r+alpha ){
        return false;
    }
    return true;
}


bool MinEnclosingCircle::circleEnclosesPoints(Circle currCircle)
{
    double alpha = 0.1;
    for ( auto p : points ){
        if ( utils::distance2(currCircle.center, p) > currCircle.r+alpha ){
            return false;
        }
    }
    return true;
}


std::vector<QPoint> MinEnclosingCircle::generateRandomPointsF(int pointsNum) const
{
    srand(static_cast<unsigned>(time(0)));

    int width, height;
    if(this->_pRenderer){
        width = _pRenderer->width();
        height = _pRenderer->height();
    }
    else{
        width = 30000;
        height = 30000;
    }

    int xCenter = width/2;
    int yCenter = height/2;
    int maxDistance = std::min(xCenter, yCenter)-10;

    float smileyLeftEyeX = xCenter - 2.0*maxDistance/5.0 * cos(pi__/6.0);
    float smileyLeftEyeY = yCenter - 2.0*maxDistance/5.0 * sin(pi__/6.0);

    float smileyRightEyeX = xCenter - 2.0*maxDistance/5.0 * cos(5*pi__/6.0);
    float smileyRightEyeY = yCenter - 2.0*maxDistance/5.0 * sin(5*pi__/6.0);
    QPoint smileyRightEye = QPoint(smileyRightEyeX, smileyRightEyeY);
    QPoint smileyLeftEye = QPoint(smileyLeftEyeX, smileyLeftEyeY);

    std::vector<QPoint> randomPoints;
    for(int i=0; i<pointsNum; i++){
        float a = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        float b = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        if ( b < a ){
            std::swap(b,a);
        }
        float currAngle = 2*pi__*a/b;
        float xAngle = std::cos(currAngle);
        float yAngle = std::sin(2*pi__*a/b);
        float currDist = b*maxDistance;
        float xPoint = xCenter + currDist*xAngle;
        float yPoint = yCenter + currDist*yAngle;
        bool isNotPartOfSmiley = true;
        QPoint currPoint(static_cast<int>(xPoint), static_cast<int>(yPoint));
//        if ( std::abs(currDist - 150) < 8 && currAngle < 7*pi__/8 && currAngle > 1*pi__/8){
//            isNotPartOfSmiley = false;
//        }
        if ( std::abs(xPoint-xCenter) < 150 && std::abs(yPoint-yCenter-60) < 10 ){
            isNotPartOfSmiley = false;
        }
        if ( utils::distance(currPoint, smileyLeftEye) < 25
                || utils::distance(currPoint, smileyRightEye) < 25 ){
            isNotPartOfSmiley = false;
        }

        if ( std::abs(xPoint - smileyRightEyeX - 20) < 70 &&
             std::abs((-50*cos((xPoint - smileyRightEyeX + 40)/35 - pi__/3)) - (yPoint - smileyRightEyeY + 40)) < 10){
            isNotPartOfSmiley = false;
        }
        if ( std::abs(xPoint - smileyLeftEyeX + 20) < 70  &&
             std::abs( (sinh(-(xPoint - smileyLeftEyeX + 20)/20)) - (yPoint - smileyLeftEyeY + 40) ) < 10){
            isNotPartOfSmiley = false;
        }

        if ( isNotPartOfSmiley){
            randomPoints.emplace_back(static_cast<int>(xPoint), static_cast<int>(yPoint));
        }
        else{
            i--;
        }
    }

    return randomPoints;
}
