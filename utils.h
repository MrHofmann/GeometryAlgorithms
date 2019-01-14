#ifndef COMMON_H
#define COMMON_H

#include <QPair>
#include <QPoint>
#include <QLine>
#include <cmath>
#include <vector>

namespace utils
{
    bool negativeOrientation(QPoint p1, QPoint p2, QPoint p3, bool includeCollinear);

    bool negativeOrientation(const QPointF &p1, const QPointF &p2, const QPointF &p3);

    double distance(QPoint a, QPoint b);

    bool lineContainsPoint(QLineF l, QPointF p);

    bool lineIntersection(QLineF l1, QLineF l2, QPointF* i);


    //GA15-------------------------------------------------------------------------------------//
    //-----------------------------------------------------------------------------------------------------//
    double distance2(QPointF a, QPointF b);

    bool lineContainsPoint2(const QLineF& l, const QPointF& p);

    bool lineIntersection2(const QLineF &l1, const QLineF &l2, QPointF* i);

    bool parallelIntersection2(const QLineF &l1, const QLineF &l2, QPointF* i);

    bool onSegment(QPointF p1, QPointF p2, QPointF p3);

    int orientation(const QPointF& p1, const QPointF& p2, const QPointF& p3);

    int pointDistanceFromLine(QLineF l, QPointF p);
    //-----------------------------------------------------------------------------------------------------//
    //GA15-------------------------------------------------------------------------------------------------//


    bool parallelIntersection3(const QLineF &l1, const QLineF &l2, QPointF* i);

    bool lineIntersection3(const QLineF &l1, const QLineF &l2, QPointF* i);

    bool segmentIntersection(const QPointF& p1, const QPointF& p2, const QPointF& q1, const QPointF& q2, QPointF *i);

    bool compareByAngleToPoint(const QPoint& referencePoint, const QPoint& p1, const QPoint& p2);

    ///
    /// \brief pointOutsideOfCircumcircle Proverava da li je tacka d van otvorenog kruga opisanog
    /// oko trougla sa temenima a, b, c.
    /// Redosled navodjenja temena trougla nije od znacaja za krajnji rezultat.
    /// \param a Teme trougla oko kojeg se opisuje krug.
    /// \param b Teme trougla oko kojeg se opisuje krug.
    /// \param c Teme trougla oko kojeg se opisuje krug.
    /// \param d Tacka za koju treba proveriti stanje.
    /// \return True ako se d nalazi van kruga opisanog oko tacaka a, b, c; false inace.
    ///
    bool pointOutsideOfCircumcircle(QPoint *a, QPoint *b, QPoint *c, QPoint *d);

    ///
    /// \brief det3 Vraca determinantu matrice 3x3, ciji su redovi oznaceni slovima, a kolone brojevima.
    /// \return Vrednost determinante.
    ///
    double det3(const double a0, const double a1, const double a2,
             const double b0, const double b1, const double b2,
             const double c0, const double c1, const double c2);

    ///
    /// \brief onSameSide Određuje da li su tačke p1 i p2 sa iste strane prave određene tačkama a i b.
    /// \param p1 Tačka čija se strana određuje.
    /// \param p2 Tačka čija se strana određuje.
    /// \param a Tačka koja određuje pravu.
    /// \param b Tačka koja određuje pravu.
    /// \return true ako su tačke p1, p2 sa iste strane prave ab; false inače.
    ///
    bool onSameSide(const QPoint& p1, const QPoint& p2, const QPoint& a, const QPoint& b);

    ///
    /// \brief isBetween Utvrdjuje da li je tacka p izmedju tacaka a i b.
    /// \param a Tacka a.
    /// \param b Tacka b.
    /// \param p Tacka p.
    /// \return true ako jeste; false inace.
    ///
    bool isBetween(const QPoint&a,const QPoint& b, const QPoint& p);

    ///
    /// \brief triangleContainsPoint
    /// Utvrđuje da li je data tačka unutar
    /// trougla određenog datim tačkama.
    /// \param a
    /// Teme trougla.
    /// \param b
    /// Teme trougla.
    /// \param c
    /// Teme trougla.
    /// \param point
    /// Tačka za koju se ispituje.
    /// \return
    /// True ukoliko se nalazi (uključujući i temena i ivice); false inače.
    ///
    bool triangleContainsPoint(QPoint *pa, QPoint *pb, QPoint *pc, QPoint *ppoint);

	// --------------------------------------------------------------
    // pomocne funkcije za rad sa odredjivanjem konveksnog omotaca
    // --------------------------------------------------------------

    ///
    /// \brief Odredjivanje sa koje strane se nalazi tacka P u odnosu na pravu koja prolazi kroz tacke P1 i P2
    /// \param p - tacka za koju se interesujemo
    /// \param p1 - leva tacka kroz koju prolazi prava
    /// \param p2 - desna tacka kroz koju prolazi prava
    /// \return - strana na kojoj se nalazi tacka p {leva (1), desna (-1), na pravoj(0)}
    ///
    short getPointSide(const QPoint p, const QPoint p1, const QPoint p2);

    ///
    /// \brief Poredjenje koje koristi algoritam za leksikografsko sortiranje.
    /// \param p1 - prva tacka
    /// \param p2 - druga tacka
    /// \return - rezultat poredjenja tacaka
    ///
    bool compareForLexicographicSort(const QPoint& p1, const QPoint& p2);

    ///
    /// \brief Izracunavanje udaljenosti tacke od prave.
    /// \param p - tacka za koju racunamo udaljenost
    /// \param p1 - leva tacka kroz koju prolazi prava
    /// \param p2 - desna tacka kroz koju prolazi prava
    /// \return - udaljenost tacke od prave
    ///
    double calculateDistanceFromLine(const QPoint &p, const QPoint &p1, const QPoint &p2);

    ///
    /// \brief Izracunava projekciju tacke P na pravu koja prolazi kroz tacke P1 i P2.
    /// \param p - tacka u odnosu na koju racunamo projekciju
    /// \param p1 - leva tacka kroz koju prolazi prava
    /// \param p2 - desna tacka kroz koju prolazi prava
    /// \return - projekcija tacke P
    ///
    QPoint getPointOnLine(const QPoint *p, const QPoint *p1, const QPoint *p2);

    ///
    /// \brief Poredjenje para tacaka
    /// \param pair1 - prvi par
    /// \param pair2 - drugi par
    /// \return - rezultat poredjenja para
    ///
    bool pairOfPointsEqual(const QPair<QPoint, QPoint>& pair1, const QPair<QPoint, QPoint>& pair2);
}

#endif // COMMON_H
