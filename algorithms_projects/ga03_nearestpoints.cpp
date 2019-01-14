#include "ga03_nearestpoints.h"
#include "utils.h"

#include <algorithm>
#include <QPainter>

NearestPoints::NearestPoints(QWidget *pRenderer, int delayMs, std::string filename, int inputSize):AlgorithmBase{pRenderer, delayMs}
{
    if(filename == "")
        _points = generateRandomPoints(inputSize);
    else
        _points = readPointsFromFile(filename);

    if(_points.size() < 2) {
        _status = AlgorithmStatus::INVALID_INPUT;
    } else {
        _status = AlgorithmStatus::CORRECT_INPUT;
    }

    _localNearestPairs = {};
    _middleLines = {};
    _candidates = {};
    _distance = -1;
    _currentFirst = nullptr;
    _currentSecond = nullptr;
}

void NearestPoints::runAlgorithm()
{   
    if(_status == AlgorithmStatus::INVALID_INPUT) {
        emit animationFinished();
        return;
    }

    //sorts the points by the x axis O(nlogn)
    std::sort(_points.begin(), _points.end(), [](const QPoint &p1, const QPoint &p2) {
        return p1.x() < p2.x();
    });

    _pointsCopy = _points; //makes a copy of the sorted points

    AlgorithmBase_updateCanvasAndBlock();

    QPair<QPoint, QPoint> nearestPair;
    findNearestPoints(0, _points.size(), nearestPair); //O(nlogn) - T(N) = 2T(N/2) + cN
    if(_destroyAnimation) {
        _status = AlgorithmStatus::FAIL;
        return;
    } else {
        _status = AlgorithmStatus::OK;
    }

    _nearestPair = nearestPair;

    AlgorithmBase_updateCanvasAndBlock();

    emit animationFinished();
}

void NearestPoints::drawAlgorithm(QPainter &painter) const
{
    int length = (int) _pointsCopy.size();
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPen pen = painter.pen();
    pen.setCapStyle(Qt::RoundCap);

    //draws all the points (points outside the frame are painted in gray)
    for(int i = 0; i < length; i++) {
        if(i >= _leftIndex && i < _rightIndex)
            changePen(painter, pen, 5);
        else
            changePen(painter, pen, 5, Qt::gray);
        painter.drawPoint(_pointsCopy[i]);
    }

    //draws current frame
    if(_middleLines.size() != 0) {
        drawCurrentSubproblemFrame(painter, pen);
    }

    //draws local solutions
    for(const QPair<QPoint, QPoint> &pair : _localNearestPairs) {
        //draws a text label next to the line segment, if the segment is in the current frame
        drawNearestPair(painter, pen, pair.first, pair.second);
    }

    //draws selected points and their segment line (if they exist)
    if(_currentFirst != nullptr && _currentSecond != nullptr) {
        drawCurrentlySelectedPoints(painter, pen);
    }

    //paints the candidates with a new color
    changePen(painter, pen, 5, Qt::green);
    for(const QPoint &point : _candidates) {
        painter.drawPoint(point);
    }

    //draws d1 and d2
    if(_distance != -1) {
        unsigned lNPLen = _localNearestPairs.size();
        QLine d2 = QLine(_localNearestPairs[lNPLen - 1].first, _localNearestPairs[lNPLen - 1].second);
        QLine d1 = QLine(_localNearestPairs[lNPLen - 2].first, _localNearestPairs[lNPLen - 2].second);
        changePen(painter, pen, 1, Qt::blue);
        painter.drawText(d1.center(), "d1");
        painter.drawText(d2.center(), "d2");
    }
}

void NearestPoints::runNaiveAlgorithm()
{
    if(_status == AlgorithmStatus::INVALID_INPUT)
        return;

    unsigned length = _points.size();
    unsigned i = 0;
    unsigned j = 1;
    double minDistance, tmpDistance;

    _nearestPair = {_points[i], _points[j]};
    minDistance = utils::distance(_nearestPair.first, _nearestPair.second);

    for(; i < length - 1; i++) {
        for(j = i + 1; j < length; j++) {
            if((tmpDistance = utils::distance(_points[i], _points[j])) < minDistance) {
                _nearestPair = {_points[i], _points[j]};
                minDistance = tmpDistance;
            }
        }
    }

    AlgorithmBase_updateCanvasAndBlock();
    _status = AlgorithmStatus::OK;
}

void NearestPoints::findNearestPoints(int left, int right, QPair<QPoint, QPoint> &result)
{
    if(right - left == 2) { //base case (two points)
        _leftIndex = left;
        _rightIndex = right;
        merge(left, left + 1, right);
        result = {_points[left], _points[left + 1]};
        _localNearestPairs.push_back({result.first, result.second});
        AlgorithmBase_updateCanvasAndBlock();
        return;
    } else if(right - left == 3) { //base case (three points)
        _leftIndex = left;
        _rightIndex = right;
        sort3(left);

        double d1 = utils::distance(_points[left], _points[left + 1]);
        double d2 = utils::distance(_points[left], _points[left + 2]);
        double d3 = utils::distance(_points[left + 1], _points[left + 2]);

        double m = std::min(std::min(d1, d2), std::min(d2, d3));
        if(d1 == m) result = {_points[left], _points[left + 1]};
        else if(d2 == m) result = {_points[left], _points[left + 2]};
        else result = {_points[left + 1], _points[left + 2]};

        _localNearestPairs.push_back({result.first, result.second});
        AlgorithmBase_updateCanvasAndBlock();
        return;
    }

    _leftIndex = left;
    _rightIndex = right;
    _middleIndex = left + (right - left) / 2;
    _middleLines.push_back(_points[_middleIndex - 1].x() + (_points[_middleIndex].x() - _points[_middleIndex - 1].x()) / 2);
    AlgorithmBase_updateCanvasAndBlock();

    QPair<QPoint, QPoint> nearestLeft;
    QPair<QPoint, QPoint> nearestRight;

    if(_destroyAnimation)
        return;
    findNearestPoints(left, _middleIndex, nearestLeft);

    //restores middle index
    _middleIndex = left + (right - left) / 2;

    if(_destroyAnimation)
        return;
    findNearestPoints(_middleIndex, right, nearestRight);

    if(_destroyAnimation)
        return;

    //restores left, right, and middle indexes
    _leftIndex = left;
    _rightIndex = right;
    _middleIndex = left + (right - left) / 2;

    merge(_leftIndex, _middleIndex, _rightIndex);

    double d1 = utils::distance(nearestLeft.first, nearestLeft.second);
    double d2 = utils::distance(nearestRight.first, nearestRight.second);
    if(d1 <= d2) {
        _distance = d1;
        result = nearestLeft;
        _distanceLineLabel = "d1";
    } else {
        _distance = d2;
        result = nearestRight;
        _distanceLineLabel = "d2";
    }

    //filters candidates
    for(int i = left; i < right; i++)
        if(fabs(_middleLines.back() - _points[i].x()) < _distance)
            _candidates.push_back(_points[i]);

    //merges solutions
    double min = -1, tmp;
    QPair<QPoint, QPoint> tmp1;
    if(_candidates.size() >= 2) {
        for(unsigned i = 0; i < _candidates.size() - 1; i++) {
            _currentFirst = &_candidates[i];
            for(unsigned j = i + 1; j < _candidates.size() && _candidates[j].y() - _candidates[i].y() < _distance; j++) {
                _currentSecond = &_candidates[j];
                AlgorithmBase_updateCanvasAndBlock();
                tmp = utils::distance(*_currentFirst, *_currentSecond);
                if(tmp < min || min == -1) {
                    min = tmp;
                    tmp1 = {*_currentFirst, *_currentSecond};
                }
                _currentSecond = nullptr;
            }
            _currentFirst = nullptr;
        }
    }

    _candidates = {};

    if(min != -1) {
        _localNearestPairs.insert(_localNearestPairs.begin(), tmp1);
        AlgorithmBase_updateCanvasAndBlock();
        _localNearestPairs.erase(_localNearestPairs.begin());
    } else {
        AlgorithmBase_updateCanvasAndBlock();
    }

    if(min < _distance && min != -1) {
        result = tmp1;
    }

    _localNearestPairs.pop_back();
    _localNearestPairs.pop_back();
    _localNearestPairs.push_back(result);

    _distance = -1;

    AlgorithmBase_updateCanvasAndBlock();

    _middleLines.pop_back();
}

void NearestPoints::merge(int left, int mid, int right)
{
    std::vector<QPoint> l = std::vector<QPoint>(_points.begin() + left, _points.begin() + mid);
    std::vector<QPoint> r = std::vector<QPoint>(_points.begin() + mid, _points.begin() + right);

    int i = 0;
    int j = 0;
    int k = 0;

    while(i < mid - left && j < right - mid) {
        if(l[i].y() < r[j].y()) {
            _points[left + k] = l[i];
            i++;
        } else {
            _points[left + k] = r[j];
            j++;
        }
        k++;
    }

    while(i < mid - left) {
        _points[left + k] = l[i];
        i++;
        k++;
    }

    while(j < right - mid) {
        _points[left + k] = r[j];
        j++;
        k++;
    }
}

void NearestPoints::sort3(int left)
{
    std::sort(_points.begin() + left, _points.begin() + left + 3, [](const QPoint &p1, const QPoint &p2) {
        return p1.y() < p2.y();
    });
}

std::vector<QPoint> NearestPoints::points() const
{
    return _points;
}

void NearestPoints::setPoints(const std::vector<QPoint> &points)
{
    if(points.size() < 2)
        _status = AlgorithmStatus::INVALID_INPUT;
    else
        _status = AlgorithmStatus::CORRECT_INPUT;

    _points = points;
}

NearestPoints::AlgorithmStatus NearestPoints::status() const
{
    return _status;
}

QPair<QPoint, QPoint> NearestPoints::nearestPair() const
{
    return _nearestPair;
}

/*
 * Helper methods for drawing
 */

void NearestPoints::changePen(QPainter &painter, QPen &pen, int width, const QColor &color, Qt::PenStyle style) const
{
    pen.setWidth(width);
    pen.setColor(color);
    pen.setStyle(style);
    painter.setPen(pen);
}

void NearestPoints::drawNearestPair(QPainter &painter, QPen &pen, const QPoint &p1, const QPoint &p2) const
{
    changePen(painter, pen, 2, Qt::red);
    painter.drawLine(p1, p2);

    changePen(painter, pen, 5, Qt::blue);
    painter.drawPoint(p1);
    painter.drawPoint(p2);
}

void NearestPoints::drawCurrentSubproblemFrame(QPainter &painter, QPen &pen) const
{
    //draws left and right bound
    changePen(painter, pen, 1, Qt::gray);
    painter.fillRect(0, 0, _pointsCopy[_leftIndex].x() - 4, _pRenderer->height(), Qt::Dense4Pattern);
    painter.fillRect(_pointsCopy[_rightIndex - 1].x() + 4, 0,
            _pRenderer->width() - (_pointsCopy[_rightIndex - 1].x() + 4), _pRenderer->height(), Qt::Dense4Pattern);

    if(_rightIndex - _leftIndex > 3) {
        //draws vertical line
        changePen(painter, pen, 3);
        painter.drawLine(_middleLines.back(), 0, _middleLines.back(), _pRenderer->height());
    }

    if(_distance != -1) {
        QLine firstHalf, secondHalf;
        double leftPoint = _middleLines.back() - _distance;
        double rightPoint = _middleLines.back() + _distance;

        firstHalf = QLine(leftPoint, _pRenderer->height() - 50, _middleLines.back(), _pRenderer->height() - 50);
        secondHalf = QLine(_middleLines.back(), _pRenderer->height() - 50, rightPoint, _pRenderer->height() - 50);

        //draws two striped lines for the candidates
        changePen(painter, pen, 1, Qt::black, Qt::PenStyle::DashLine);
        painter.drawLine(leftPoint, 0, leftPoint, _pRenderer->height());
        painter.drawLine(rightPoint, 0, rightPoint, _pRenderer->height());

        //draws distance arrow lines
        painter.drawLine(firstHalf);
        painter.drawText(QPoint(firstHalf.center().x(), firstHalf.center().y() - 2), _distanceLineLabel);
        painter.drawLine(secondHalf);
        painter.drawText(QPoint(secondHalf.center().x(), secondHalf.center().y() - 2), _distanceLineLabel);

        changePen(painter, pen, 1, Qt::black);
        painter.drawLine(firstHalf.p1(), QPoint(firstHalf.p1().x() + 5, firstHalf.p1().y() + 2));
        painter.drawLine(firstHalf.p1(), QPoint(firstHalf.p1().x() + 5, firstHalf.p1().y() - 2));
        painter.drawLine(firstHalf.p2(), QPoint(firstHalf.p2().x() - 5, firstHalf.p2().y() + 2));
        painter.drawLine(firstHalf.p2(), QPoint(firstHalf.p2().x() - 5, firstHalf.p2().y() - 2));
        painter.drawLine(secondHalf.p1(), QPoint(secondHalf.p1().x() + 5, secondHalf.p1().y() + 2));
        painter.drawLine(secondHalf.p1(), QPoint(secondHalf.p1().x() + 5, secondHalf.p1().y() - 2));
        painter.drawLine(secondHalf.p2(), QPoint(secondHalf.p2().x() - 5, secondHalf.p2().y() + 2));
        painter.drawLine(secondHalf.p2(), QPoint(secondHalf.p2().x() - 5, secondHalf.p2().y() - 2));
    }
}

void NearestPoints::drawCurrentlySelectedPoints(QPainter &painter, QPen &pen) const
{
    changePen(painter, pen, 2, Qt::green);
    painter.drawLine(*_currentFirst, *_currentSecond);

    changePen(painter, pen, 5, Qt::green);
    painter.drawPoint(*_currentFirst);
    painter.drawPoint(*_currentSecond);
}
