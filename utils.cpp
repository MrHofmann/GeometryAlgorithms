#include "utils.h"
#include <iostream>
#include <vector>
#include <QtMath>

namespace utils
{
    bool negativeOrientation(QPoint p1, QPoint p2, QPoint p3, bool includeCollinear)
    {
        if(includeCollinear)
             return ((p2.x() - p1.x())*(p3.y() - p1.y()) - (p3.x() - p1.x())*(p2.y() - p1.y())) < 0;
        else
             return ((p2.x() - p1.x())*(p3.y() - p1.y()) - (p3.x() - p1.x())*(p2.y() - p1.y())) <= 0;
    }

    bool negativeOrientation(const QPointF& p1, const QPointF& p2, const QPointF& p3)
    {
        return ((p2.x() - p1.x())*(p3.y() - p1.y()) - (p3.x() - p1.x())*(p2.y() - p1.y())) <= 0;
    }

    double distance(QPoint a, QPoint b)
    {
        if((a.x() == b.x()) && (a.y() == b.y())) return 0;
        return sqrt(pow((a.x() - b.x()),2) + pow(a.y() - b.y() ,2));
    }

    bool lineContainsPoint(QLineF l, QPointF p)
    {
        //X must not be left from left end point
        if(l.p1().x() < l.p2().x())
        {
            if(p.x() < l.p1().x() || p.x() > l.p2().x())
                return false;
        }else{
            if(p.x() > l.p1().x() || p.x() < l.p2().x())
                return false;
        }

        if(l.p1().y() < l.p2().y()){
            if(p.y() < l.p1().y() || p.y() > l.p2().y())
                return false;
        }else{
            if(p.y() > l.p1().y() || p.y() < l.p2().y())
                return false;
        }

        return true;
    }

    bool lineIntersection(QLineF l1, QLineF l2, QPointF* i)
    {
        double k1 = (l1.p1().y() - l1.p2().y())/(l1.p1().x() - l1.p2().x());
        double k2 = (l2.p1().y() - l2.p2().y())/(l2.p1().x() - l2.p2().x());

        double n1 = l1.p1().y() - k1*l1.p1().x();
        double n2 = l2.p1().y() - k2*l2.p1().x();

        double dx = (n2-n1)/(k1-k2);

        *i = QPointF(dx,k1*dx + n1);

        return lineContainsPoint(l1, *i) && lineContainsPoint(l2, *i);
    }


    //GA15-------------------------------------------------------------------------------------------------//
    //-----------------------------------------------------------------------------------------------------//
    double distance2(QPointF a, QPointF b)
    {
        if((a.x() == b.x()) && (a.y() == b.y())) return 0;
        return sqrt(pow((a.x() - b.x()),2) + pow(a.y() - b.y() ,2));
    }

    bool lineContainsPoint2(const QLineF& l, const QPointF& p)
    {
        //X must not be left from left end point
        if(l.x1() < l.x2())
        {
            if(p.x() < l.x1() || p.x() > l.x2())
                return false;
        }
        else
        {
            if(p.x() > l.p1().x() || p.x() < l.p2().x())
                return false;
        }

        if(l.p1().y() < l.p2().y())
        {
            if(p.y() < l.p1().y() || p.y() > l.p2().y())
                return false;
        }
        else
        {
            int prvi = (int)(p.y()+0.5);
            int drugi = (int)(l.p1().y()+0.5);
            int treci = (int)(l.p2().y()+0.5);
            if(prvi > drugi || prvi < treci)
            {
                return false;
            }
        }

        return true;
    }

    bool lineIntersection2(const QLineF &l1, const QLineF &l2, QPointF* i)
    {
        double k1, k2;
        double n1, n2;
        if(l1.x1() == l1.x2() && l2.x1() == l2.x2())
        {
            return parallelIntersection2(l1, l2, i);
        }
        else if(l1.x1() == l1.x2())
        {
            //std::cout << "Line Intersection: first line is vertical!" << std::endl;
            k2 = (l2.y1() - l2.y2())/(l2.x1() - l2.x2());
            n2 = l2.y1() - k2*l2.x1();

            *i = QPointF(l1.x1(), k2*l1.x1() + n2);
            return lineContainsPoint2(l1, *i) && lineContainsPoint2(l2, *i);
        }
        else if(l2.x1() == l2.x2())
        {
            //std::cout << "Line Intersection: second line is vertical!" << std::endl;
            k1 = (l1.y1() - l1.y2())/(l1.x1() - l1.x2());
            n1 = l1.y1() - k1*l1.x1();

            *i = QPointF(l2.x1(), k1*l2.x1() + n1);
            return lineContainsPoint2(l1, *i) && lineContainsPoint2(l2, *i);
        }
        else
        {
            k1 = (l1.p1().y() - l1.p2().y())/(l1.p1().x() - l1.p2().x());
            k2 = (l2.p1().y() - l2.p2().y())/(l2.p1().x() - l2.p2().x());

            n1 = l1.y1() - k1*l1.x1();
            n2 = l2.y1() - k2*l2.x1();

            if(k1 == k2)
                return parallelIntersection2(l1, l2, i);
            else
            {
                double dx = (n2-n1)/(k1-k2);
                *i = QPointF(dx,k1*dx + n1);

                return lineContainsPoint2(l1, *i) && lineContainsPoint2(l2, *i);
            }
        }
    }
    //NE RADI UVEK, VRATILA JE LAZNO NEGATIVAN REZULTAT U isVisible U lineIntersection2 FUNKCIJAMA
    bool onSegment(QPointF p1, QPointF q, QPointF p2)
    {
        return  q.x() >= std::min(p1.x(), p2.x()) && q.x() <= std::max(p1.x(), p2.x()) &&
                q.y() >= std::min(p1.y(), p2.y()) && q.y() <= std::max(p1.y(), p2.y());
    }

    bool parallelIntersection2(const QLineF &l1, const QLineF &l2, QPointF *i)
    {
        QPointF q1, q2;
        if(onSegment(l1.p1(), l2.p1(), l1.p2()) && onSegment(l1.p1(), l2.p2(), l1.p2()))
        {
            q1 = l2.p1();
            q2 = l2.p2();
        }
        else
        {
            if(onSegment(l1.p1(), l2.p1(), l1.p2()))
                q1 = l2.p1();
            else if(onSegment(l1.p1(), l2.p2(), l1.p2()))
                q1 = l2.p2();
            else
                return false;

            if(onSegment(l2.p1(), l1.p1(), l2.p2()))
                q2 = l1.p1();
            else
                q2 = l1.p2();
        }

        *i = QPointF((q1.x()+q2.x())/2, (q1.y()+q2.y())/2);
        return true;
    }

    int orientation(const QPointF &p1, const QPointF &p2, const QPointF &p3)
    {
        int o = (p2.y() - p1.y())*(p3.x() - p2.x()) -
                (p2.x() - p1.x())*(p3.y() - p2.y());

        if(o < 0)
            return -1;
        else if(o > 0)
            return 1;
        else
            return 0;
    }
    //NE PREPOZNAJE TACKU NA DUZI, LICI NA ORIJENTACIJU, NE KORISITI SE VISE (GA15)
    int pointDistanceFromLine(QLineF l, QPointF p)
    {
        double d =  (p.x() - l.x1())*(l.y2() - l.y1()) -
                    (p.y() - l.y1())*(l.x2() - l.x1());

        if(d > 0)
            return 1;
        else if(d < 0)
            return -1;
        else
            return 0;
    }
    //-----------------------------------------------------------------------------------------------------//
    //GA15-------------------------------------------------------------------------------------------------//


    bool parallelIntersection3(const QLineF& l1, const QLineF& l2, QPointF *i)
    {
        QPointF q1, q2;
        if(onSegment(l1.p1(), l2.p1(), l1.p2()) && onSegment(l1.p1(), l2.p2(), l1.p2()))
        {
            q1 = l2.p1();
            q2 = l2.p2();
        }
        else
        {
            if(onSegment(l1.p1(), l2.p1(), l1.p2()))
                q1 = l2.p1();
            else if(onSegment(l1.p1(), l2.p2(), l1.p2()))
                q1 = l2.p2();
            else
                return false;

            if(onSegment(l2.p1(), l1.p1(), l2.p2()))
                q2 = l1.p1();
            else
                q2 = l1.p2();
        }

        if(i != nullptr)
            *i = QPointF((q1.x()+q2.x())/2, (q1.y()+q2.y())/2);

        return true;
    }

    bool lineIntersection3(const QLineF& l1, const QLineF& l2, QPointF* i)
    {
        double k1, k2;
        double n1, n2;
        if(l1.x1() == l1.x2() && l2.x1() == l2.x2())
        {
            return parallelIntersection3(l1, l2, i);
        }
        else if(l1.x1() == l1.x2())
        {
            //std::cout << "Line Intersection: first line is vertical!" << std::endl;
            k2 = (l2.y1() - l2.y2())/(l2.x1() - l2.x2());
            n2 = l2.y1() - k2*l2.x1();


            QPointF intersectionPoint = QPointF(l1.x1(), k2*l1.x1() + n2);
            if(lineContainsPoint2(l1, intersectionPoint) && lineContainsPoint2(l2, intersectionPoint))
            {
                if(i != nullptr)
                    *i = intersectionPoint;
                return true;
            }
        }
        else if(l2.x1() == l2.x2())
        {
            //std::cout << "Line Intersection: second line is vertical!" << std::endl;
            k1 = (l1.y1() - l1.y2())/(l1.x1() - l1.x2());
            n1 = l1.y1() - k1*l1.x1();

            QPointF intersectionPoint = QPointF(l2.x1(), k1*l2.x1() + n1);
            if(lineContainsPoint2(l1, intersectionPoint) && lineContainsPoint2(l2, intersectionPoint))
            {
                if(i != nullptr)
                    *i = intersectionPoint;
                return true;
            }
        }
        else
        {
            k1 = (l1.p1().y() - l1.p2().y())/(l1.p1().x() - l1.p2().x());
            k2 = (l2.p1().y() - l2.p2().y())/(l2.p1().x() - l2.p2().x());

            n1 = l1.y1() - k1*l1.x1();
            n2 = l2.y1() - k2*l2.x1();

            if(k1 == k2)
                return parallelIntersection3(l1, l2, i);
            else
            {
                double dx = (n2-n1)/(k1-k2);
                QPointF intersectionPoint = QPointF(dx,k1*dx + n1);

                if(lineContainsPoint2(l1, intersectionPoint) && lineContainsPoint2(l2, intersectionPoint))
                {
                    if(i != nullptr)
                        *i = intersectionPoint;
                    return true;
                }
            }
        }

        return false;
    }

    bool segmentIntersection(const QPointF& p1, const QPointF& p2, const QPointF& q1, const QPointF& q2, QPointF *i)
    {
        int b1 = orientation(p1, p2, q1);
        int b2 = orientation(p1, p2, q2);
        int b3 = orientation(q1, q2, p1);
        int b4 = orientation(q1, q2, p2);

        utils::lineIntersection3(QLineF(p1, p2), QLineF(q1, q2), i);

        if(b1 != b2 && b3 != b4) return true;

        if(b1 == 0 && onSegment(p1, q1, p2)) return true;
        if(b2 == 0 && onSegment(p1, q2, p2)) return true;
        if(b3 == 0 && onSegment(q1, p1, q2)) return true;
        if(b4 == 0 && onSegment(q1, p2, q2)) return true;

        return false;
    }

    bool compareByAngleToPoint(const QPoint &referencePoint, const QPoint &p1, const QPoint &p2)
    {
        double angle1 = atan2(referencePoint.y() - p1.y(), referencePoint.x() - p1.x());
        double angle2 = atan2(referencePoint.y() - p2.y(), referencePoint.x() - p2.x());

        if(angle1 != angle2)
            return angle1 < angle2;
        else
            return p1.x() > p2.x();
    }

    bool pointOutsideOfCircumcircle( QPoint* a,  QPoint* b,  QPoint* c,  QPoint* d)
    {
        if(negativeOrientation(*a, *b, *c))
        {
            QPoint* swap = a;
            a = b;
            b = swap;
        }

        return 0 >= det3(a->x() - d->x(), a->y() - d->y(), (a->x() - d->x()) * (a->x() - d->x()) + (a->y() - d->y()) * (a->y() - d->y()),
                         b->x() - d->x(), b->y() - d->y(), (b->x() - d->x()) * (b->x() - d->x()) + (b->y() - d->y()) * (b->y() - d->y()),
                         c->x() - d->x(), c->y() - d->y(), (c->x() - d->x()) * (c->x() - d->x()) + (c->y() - d->y()) * (c->y() - d->y()));
    }

    double det3(const double a0, const double a1, const double a2,
             const double b0, const double b1, const double b2,
             const double c0, const double c1, const double c2)
    {
        return - (c0 * b1 * a2 + c1 * b2 * c0 + c2 * b0 * a1) + (a0 * b1 * c2 + a1 * b2 * c0 + a2 * b0 * c1);
    }

    bool onSameSide(const QPoint &p1, const QPoint &p2, const QPoint &a, const QPoint &b)
    {
        return negativeOrientation(p1, a, b) == negativeOrientation(p2, a, b);
    }

    bool isBetween( const QPoint &a, const QPoint &b, const QPoint&p)
    {
        if(std::abs((p.y() - a .y()) * (b.x() - a.x()) - (p.x() - a.x()) * (b.y() -a .y())) != 0)
            return false;

        int dotProduct = (p.x() - a.x())*(b.x() -a.x()) + (p.y()-a.y())*(b.y() - a.y());
        if(dotProduct < 0)
            return false;

        int distance = (b.x() -a .x()) * (b.x() -a .x()) + (b.y() - a.y()) * (b.y() - a.y());
        return dotProduct <= distance;
    }

    bool triangleContainsPoint(QPoint *pa, QPoint *pb, QPoint *pc, QPoint *ppoint)
    {
        QPoint a = *pa;
        QPoint b = *pb;
        QPoint c = *pc;
        QPoint point = *ppoint;

        return
            (a.x() == point.x() && a.y() == point.y())
            || (b.x() == point.x() && b.y() == point.y())
            || (c.x() == point.x() && c.y() == point.y())
            || utils::isBetween(a, b, point)
            || utils::isBetween(b, c, point)
            || utils::isBetween(c, a, point)
            || ((utils::onSameSide(a, point, b, c)
                && utils::onSameSide(b, point, a, c)
                && utils::onSameSide(c, point, b, a)));
    }

	short getPointSide(const QPoint p, const QPoint p1, const QPoint p2)
    {
        int sgn = (p.x() - p1.x())*(p2.y()-p1.y()) - (p.y() - p1.y())*(p2.x() - p1.x());

        if (sgn > 0)
        {
            return 1;
        }
        else if (sgn < 0)
        {
            return -1;
        }
        else
        {
            return 0;
        }
    }

    bool compareForLexicographicSort(const QPoint &p1, const QPoint &p2)
    {
        if (p1.x() < p2.x())
        {
            return true;
        }
        else if (p1.x() > p2.x())
        {
            return false;
        }
        else
        {
            if (p1.y() < p2.y())
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }

    double calculateDistanceFromLine(const QPoint &p, const QPoint &p1, const QPoint &p2)
    {
        double nominator = (p2.y() - p1.y())*p.x() - (p2.x() - p1.x())*p.y() + p2.x()*p1.y() - p2.y()*p1.x();
        double denominator = qPow((p2.y() - p1.y()), 2) + qPow((p2.x() - p1.x()), 2);

        return abs(nominator)/sqrt(denominator);
    }

    QPoint getPointOnLine(const QPoint *p, const QPoint *p1, const QPoint *p2)
    {
        double t = static_cast<double>((p->x() - p1->x())*(p2->x() - p1->x()) + (p->y() - p1->y())*(p2->y() - p1->y()))
                / static_cast<double>(qPow(p2->x() - p1->x(), 2) + qPow(p2->y() - p1->y(), 2));

        double x = p1->x() + t * (p2->x() - p1->x());
        double y = p1->y() + t * (p2->y() - p1->y());

        return QPoint(x, y);
    }

    bool pairOfPointsEqual(const QPair<QPoint, QPoint>& pair1, const QPair<QPoint, QPoint>& pair2) {
        return pair1 == pair2 || (pair1.first == pair2.second && pair1.second == pair2.first);
    }
}
