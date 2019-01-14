/*
Autor: Rastko Djordjevic 1091/2017
Godina: 2017
Kratak opis problema: Dato je n tacaka. Naci najmanji disk koji pokriva sve tacke
*/
#ifndef GA18_SMALLESTENCLOSINGDISK_H
#define GA18_SMALLESTENCLOSINGDISK_H

#include "algorithmbase.h"
#include <QPainter>
#include <QPointF>
#include <QPoint>
#include <vector>

#define drawMe_updateCanvasAndBlock() \
    if(!_pThread) \
    { \
       ; \
    } \
    else if (updateCanvasAndBlock()) \
    { \
        return false; \
    }


struct Circle
{
public:
    ///
    /// \brief explicit constructor
    /// \param cen		- center of the circle
    /// \param rad	 	- radius of the circle
    /// \param ar	 	- area of the circle
    ///
    Circle(QPointF cen, double rad, double ar)
        : center(cen), r(rad), area(ar) {}

    ///
    /// \brief copy constructor for a circle
    /// \param another 	- circle that is to be copied
    ///
    Circle( const Circle& another)
        : center(another.center), r(another.r), area(another.area) {}

    ///
    /// \brief copy assignment operator
    /// \param other 	- circle that is to be copied
    /// \return reference to the circle which was changed
    ///
    Circle& operator=( const Circle other );


    ///
    /// \brief constructs a unique circle that is the circumcircle of the triangle that
    /// 		is made by points p1, p2, p3
    /// \param p1		- point p1
    /// \param p2		- point p2
    /// \param p3		- point p3
    ///
    Circle(QPointF p1, QPointF p2, QPointF p3);

    ///
    /// \brief constructs a unique circle whose diameter is formed by points p1 and p2
    /// \param p1		- point p1
    /// \param p2		- point p2
    ///
    Circle(QPointF p1, QPointF p2);
public:
    ///
    /// \brief center of the circle
    ///
    QPointF center;

    ///
    /// \brief radius of the circle
    ///
    double r;

    ///
    /// \brief area of the circle
    ///
    double area;

    ///
    /// \brief check if point p is located inside the circle
    /// \param p		- point p
    /// \return
    /// 	True, if point p is inside the circle
    /// 	False, if point p is outside the circle
    ///
    bool containsPoint(QPointF p) const;
};


class MinEnclosingCircle : public AlgorithmBase
{
public:
    MinEnclosingCircle(QWidget *pRenderer, int delayMs, std::string filename,
             int inputSize = DEFAULT_POINTS_NUM);
    MinEnclosingCircle(QWidget *pRenderer, int delayMs, std::vector<QPointF> newPoints );

public:
    ///
    /// \brief run the min enclosing disk algorithm
    ///
    void runAlgorithm();

    ///
    /// \brief draw the visualisation of min enclosing disk algorithm
    /// \param painter		- a reference to QPainter object
    ///
    void drawAlgorithm(QPainter &painter) const;

    ///
    /// \brief draw the circle with provided parameters on QPainter object
    /// \param painter		- a reference to QPainter object
    /// \param disk			- disk to be drawn
    /// \param brushTint 	- inside color of the disk
    /// \param penWidth	 	- width of disk's bounding circle
    /// \param penColor	 	- color of disk's bounding circle
    ///
    void drawDisk( QPainter &painter, Circle disk, QColor brushTint, int penWidth, QColor penColor) const;

    ///
    /// \brief draw first n points of the class attribute points on QPainter object
    /// \param painter		- a reference to QPainter object
    /// \param n			- number of points to be drawn
    /// \param penWidth 	- width of the points
    /// \param penColor 	- color of the points
    ///
    void drawFirstNPoints( QPainter &painter, int n, int penWidth, QColor penColor ) const;

    ///
    /// \brief draw the point that is currently being processed
    /// 		if the point is located inside the current smallest enclosing disk
    /// 		then the point will have color in
    /// 		otherwise it will have color out
    /// \param painter		- a reference to QPainter object
    /// \param pointIndex	- index of the point
    /// \param disk			- current min enclosing disk
    /// \param penWidth		- point width
    /// \param in			- color of the point if it's inside the disk
    /// \param out			- color of the point if it's outside the disk
    ///
    void drawInsertedPoint( QPainter &painter, std::size_t pointIndex, Circle disk,
                        int penWidth = 6, QColor in = Qt::green, QColor out = Qt::red) const;

    ///
    /// \brief change the pen of the QPainter object that is passed as a parameter
    /// \param painter	- a reference to QPainter object
    /// \param penWidth	- pen width
    /// \param penColor - pen color
    ///
    void changePen( QPainter &painter, int penWidth, QColor penColor) const;

    ///
    /// \brief run the brute force algorithm for the min enclosing diks problem.
    /// 		First preprocces the points so only the points on the convex hull are left
    /// 		Then for all pairs and triplets of points do the following:
    /// 		if the unique circle that currently selected points form encloses
    /// 		all points and is smaller than the current min disk,
    /// 		than the current min disk is set to that value.
    ///
    /// 	for a detailed explanation look in /docs/ga18_minEnclosingDiskv.pdf
    ///
    void runNaiveAlgorithm();

    ///
    /// \brief getter for the result of the smallest enclosing disk problem
    /// \return smallest enclosing disk
    ///
    Circle result() const;

    ///
    /// \brief clear the result of the smallest enclosing disk problem
    /// 		minCircle class attribute is set to default value.
    ///
    void clearResult();

private:
    ///
    /// \brief @todo deleteme
    ///
    int n;

    ///
    /// \brief current level of recursion that the min enclosing disk algorithm is in
    /// 		phase == 0, before any additions
    /// 		phase == 1, no fixed points
    /// 		phase == 2, one fixed point
    /// 		phase == 3, two fixed points
    ///
    int phase;

    ///
    /// \brief index of the point that is being inserted in first phase of the SED algorithm
    ///
    std::size_t InsertPointIndex;

    ///
    /// \brief index of the point that is being inserted in second phase of the SED algorithm
    ///
    std::size_t ph2InsertPointIndex;

    ///
    /// \brief index of the point that is being inserted in third phase of the SED algorithm
    ///
    int ph3InsertPointIndex;

    ///
    /// \brief smallest enclosing disk in the second phase of the SED algorithm
    ///
    Circle ph2MinCircle;

    ///
    /// \brief smallest enclosing disk in the third phase of the SED algorithm
    ///
    Circle ph3MinCircle;

    ///
    /// \brief points, which we procces in brute force and SED algorithms
    ///
    std::vector<QPointF> points;

    ///
    /// \brief smallest enclosing disk of the points
    ///
    Circle minCircle;

    ///
    /// \brief minDisk
    /// 	for a detailed explanation look in /docs/ga18_minEnclosingDiskv.pdf
    /// \return
    /// 	Flase, if the algorithm should stop
    /// 	True, otherwise
    ///
    bool minDisk( );

    ///
    /// \brief minDisk
    /// 	for a detailed explanation look in /docs/ga18_minEnclosingDiskv.pdf
    /// \param p1_index		- index of fixed point p1
    /// \return
    /// 	Flase, if the algorithm should stop
    /// 	True, otherwise
    ///
    bool minDisk( unsigned int p1_index );

    ///
    /// \brief minDisk
    /// 	for a detailed explanation look in /docs/ga18_minEnclosingDiskv.pdf
    /// \param p1_index		- index of fixed point p1
    /// \param p2_index		- index of fixed point p2
    /// \return
    /// 	Flase, if the algorithm should stop
    /// 	True, otherwise
    ///
    bool minDisk( unsigned int p1_index, unsigned int p2_index);

    ///
    /// \brief circleEnclosesPoints checks if circle that is passed encloses all
    /// 		points that are in class attribute points
    /// \param currCircle	- circle that is being processed
    /// \return True if the circle encloses all points
    /// 		False if it doesn't
    ///
    bool circleEnclosesPoints(Circle currCircle);

    ///
    /// \brief generate random points such that their smallest enclosing circle
    /// 		doesn't go off the canvas.
    /// \param pointsNum	- number of points to be generated
    /// \return vector of points that are generated
    ///
    std::vector<QPoint> generateRandomPointsF( int pointsNum = DEFAULT_POINTS_NUM) const;
};

#endif // GA18_SMALLESTENCLOSINGDISK_H
