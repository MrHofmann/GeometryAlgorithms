#ifndef GA08_MONOTONECHAIN_H
#define GA08_MONOTONECHAIN_H

/*
Autor: David ivic
Godina: 2018
Kratak opis problema: Odredjivanje konveksnog omotaca za zadati ulaz
*/

#include "algorithms_practice/convexhull.h"
#include <iostream>
#include <vector>
#include <algorithm>


typedef std::vector<QPoint> Points;


class MonotoneChain : public ConvexHull
{
public:
    MonotoneChain(QWidget *pRender, int delayMs, std::string filename = "", int inputSize=DEFAULT_POINTS_NUM);

    enum Sides {LEFT, RIGHT, COLLINEAR};

    virtual void runAlgorithm();
    virtual void drawAlgorithm(QPainter &painter) const;
    virtual void runNaiveAlgorithm();

    ///
    /// \brief convexHull
    /// \return final result of algorithm
    ///
    Points convexHull() const;
    ///
    /// \brief GetPointSide Checks if point P is left or right bewteen line from A to B
    /// \param A is the first point
    /// \param B is the second point
    /// \param P point we are checking
    /// \return enum left/right/collinear
    ///
    int GetPointSide(const QPoint &A, const QPoint &B, const QPoint &P);
    ///
    /// \brief PointALreadyIn Checks if point is already in given array
    /// \param source is given array
    /// \param A is a point we are checking for
    /// \return True/False
    ///
    bool PointALreadyIn(const Points &source, const QPoint &A);
    ///
    /// \brief CheckTest is a function that cheks situation where all points lies on one line
    /// \param A is a set of points
    /// \return True/False
    ///
    bool CheckTest(const Points &A);
    ///
    /// \brief checkCondition is a function that checks did we change the orientation of the points
    /// \return True/False
    ///
    bool checkCondition(Sides);

private:

    int m_inputSize;
    Points m_result;
    int m_lastPointAdded;
    QPoint m_minXminY;
    QPoint m_maxXmaxY;
};

#endif // GA08_MONOTONECHAIN_H
