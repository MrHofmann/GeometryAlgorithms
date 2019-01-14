/*
Autor: Milos Samardzija 1092/2016
Godina: 2017
Kratak opis problema: Dato je n tacaka u ravni. Potrebno je odrediti dve tacke sa najmanjim Euklidskim rastojanjem.
*/

#ifndef GA03_NEARESTPOINTS_H
#define GA03_NEARESTPOINTS_H

#include "algorithmbase.h"

class NearestPoints : public AlgorithmBase
{
public:
    enum AlgorithmStatus {OK, FAIL, CORRECT_INPUT, INVALID_INPUT};

    NearestPoints(QWidget* pRenderer, int delayMs, std::string filename = "", int inputSize = DEFAULT_POINTS_NUM);

    void runAlgorithm();
    void drawAlgorithm(QPainter &painter) const;
    void runNaiveAlgorithm();

    QPair<QPoint, QPoint> nearestPair() const;
    AlgorithmStatus status() const;

    /* Getter and setter for _points (only for testing purposes!) */
    std::vector<QPoint> points() const;
    void setPoints(const std::vector<QPoint> &points);

private:
    /* A recursive algorithm that finds two nearest points in the plane
     *  left    - left index of sorted array of points (by x axis)
     *  right   - right index of sorted array of points (by x axis)
     *  result  - the result of the algorithm
     */
    void findNearestPoints(int left, int right, QPair<QPoint, QPoint> &result);

    /* Changes width, color and style of the pen
     *  painter - a reference to QPainter object
     *  pen     - a reference to QPen object
     *  width   - pen width
     *  color   - pen color
     *  style   - pen style
     */
    void changePen(QPainter &painter, QPen &pen, int width, const QColor &color = Qt::black, Qt::PenStyle style = Qt::PenStyle::SolidLine) const;

    /* Draws two points and segment line they form
     *  painter - a reference to QPainter object
     *  pen     - a reference to QPen object
     *  p1      - the first point
     *  p2      - the second point
     */
    void drawNearestPair(QPainter &painter, QPen &pen, const QPoint &p1, const QPoint &p2) const;

    /* Draws frame of current subproblem (left and right bound, middle line,
     * two striped lines for candidates [middle - _distance, middle + _distance])
     *  painter - a reference to QPainter object
     *  pen     - a reference to QPen object
     */
    void drawCurrentSubproblemFrame(QPainter &painter, QPen &pen) const;

    /* Draws currently selected points (points currently being checked), and segment line they form
     *  painter - a reference to QPainter object
     *  pen     - a reference to QPen object
     */
    void drawCurrentlySelectedPoints(QPainter &painter, QPen &pen) const;

    /* Merges two sorted arrays of points into one (array[left:mid], array[mid:right]) -> array[left:right]
     *  left    - left index of array (included)
     *  mid     - middle index of array (included, excluded)
     *  right   - right index of array (excluded)
     */
    void merge(int left, int mid, int right);

    /* Sorts three points in array starting from index left (array[left:left + 3])
     *  left    - left index of array (included)
     */
    void sort3(int left);

    int _leftIndex, _rightIndex, _middleIndex; //left, right and middle indices of the current frame
    double _distance; //the minimum distance of the left and right subproblems
    AlgorithmStatus _status;
    QString _distanceLineLabel;
    std::vector<int> _middleLines; //vertical lines that split the plane into two parts (into two subproblems)
    std::vector<QPoint> _points;
    std::vector<QPoint> _pointsCopy; //a copy of the points (necessary to preserve order, according to the x axis)
    std::vector<QPoint> _candidates; //points with x coord in range (middle - _distance, middle + _distance)
    std::vector<QPair<QPoint, QPoint>> _localNearestPairs; //the nearest points in subproblems
    QPair<QPoint, QPoint> _nearestPair; //result
    QPoint *_currentFirst, *_currentSecond; //currently selected points (points currently being checked, if not selected, they are equal to nullptr)
};

#endif // GA03_NEARESTPOINTS_H
