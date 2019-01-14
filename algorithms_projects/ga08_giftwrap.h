#ifndef GA08_GIFTWRAP_H
#define GA08_GIFTWRAP_H

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


class GiftWrap : public ConvexHull
{
public:
    GiftWrap(QWidget *pRender, int delayMs, std::string filename = "", int inputSize=DEFAULT_POINTS_NUM);

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
    /// \brief CheckTest is a function that checks situation where all points lies on one line
    /// \param A is a set of points
    /// \return True/False
    ///

    bool CheckTest(const Points &A);
    ///
    /// \brief Checking is there any collinear points between points in current result
    /// \param result is current result
    /// \param source is given set of points
    /// \return result expanded
    ///
    void AddColinearPoints(Points &result, const Points source);

    ///
    /// \brief GetPointSide Checks if point P is left or right bewteen line from A to B
    /// \param A is the first point
    /// \param B is the second point
    /// \param P point we are checking
    /// \return enum left/right/collinear
    ///
    int GetPointSide(QPoint A, QPoint B, QPoint P);
    ///
    /// \brief PointALreadyIn Checks if point is already in given array
    /// \param source is given array
    /// \param A is a point we are checking for
    /// \return True/False
    ///
    bool PointALreadyIn(const Points &source, const QPoint &A);

private:

    int m_inputSize;
    Points m_result;
    Points::const_iterator m_potentialNode;
    Points::const_iterator m_workingNode;
};

#endif // GA08_GIFTWRAP_H
