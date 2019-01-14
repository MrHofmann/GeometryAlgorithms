/* ------------------------------------------------------
    Autor: Djordje Milicevic
    Godina: 2018.
    Opis problema: Incremental insertion algoritam
                   za konstrukciju konveksnog omotaca
                   skupa tacaka u ravni
------------------------------------------------------ */

#ifndef GA00_INCREMENTALINSERTION_H
#define GA00_INCREMENTALINSERTION_H

#include "algorithms_practice/convexhull.h"

#define LEFT_SIDE 1
#define RIGHT_SIDE -1
#define ON_THE_LINE 0

class Point;


class IncrementalInsertion : public ConvexHull
{
public:
    IncrementalInsertion(QWidget *pRenderer, int delayMs, std::string filename = "", int inputSize = DEFAULT_POINTS_NUM);

    ///
    /// \brief Implementacija naprednog algoritma.
    ///
    void runAlgorithm();

    ///
    /// \brief Iscrtavanje naprednog algoritma.
    /// \param painter
    ///
    void drawAlgorithm(QPainter &painter) const;

    ///
    /// \brief Implementacija naivnog algoritma.
    ///
    void runNaiveAlgorithm();

    ///
    /// \brief Geter metod.
    /// \return - vektor koji sadrzi tacke koje je pronasao napredni algoritam
    ///
    std::vector<QPoint> getConvexHull() const;

    ///
    /// \brief Geter metod.
    /// \return - vektor koji sadrzi tacke koje je pronasao naivni algoritam
    ///
    std::vector<QPoint> getConvexHullTest() const;

    ///
    /// \brief Okretanje povezane liste.
    ///
    void reverseList();

    ///
    /// \brief Konstruisanje pocetnog trougla pozitivne orijentacije.
    /// \param p1 - prva tacka
    /// \param p2 - druga tacka
    /// \param p3 - treca tacka
    ///
    void makeTriangle(Point *p1, Point *p2, Point *p3);

private:
    unsigned int _xPositionOfSweepline;         // pozicija brisuce prave (koristi se pri vizuelizaciji)
    unsigned int _numberOfProcessedPoints;      // broj obradjenih tacaka (koristi se pri vizuelizaciji)

    bool _collinearIndicator = false;           // indikator kolinearnosti tacaka

    Point *_topPoint = nullptr;                 // gornja potporna tacka
    Point *_bottomPoint = nullptr;              // donja potporna tacka
    Point *_currentPoint = nullptr;             // tacka koja se trenutno obradjuje

    std::vector<Point> _complexPoints;          // vektor cvorova koji predstavljaju tacke
    std::vector<QPoint> _convexHull;            // vektor tacaka koje se nalaze u konveksnom omotacu
};



class Point
{
public:
    Point(const QPoint &value)
        : _value(value), _nextPoint(nullptr), _prevPoint(nullptr)
    {}

    Point(const Point &point)
        : _value(point._value), _nextPoint(point.getNextPoint()), _prevPoint(point.getPrevPoint())
    {}

    ///
    /// \brief Postavljanje narednog cvora liste za narednu tacku.
    /// \param point - naredni cvor u listi
    ///
    void setNextPoint(Point *point) { _nextPoint = point; }

    ///
    /// \brief Postavljanje prethodnog cvora liste za prethodnu tacku.
    /// \param point - prethodni cvor u listi
    ///
    void setPrevPoint(Point *point) { _prevPoint = point; }

    ///
    /// \brief Geter metod.
    /// \return - naredni cvor u listi
    ///
    Point* getNextPoint() const { return _nextPoint; }

    ///
    /// \brief Geter metod.
    /// \return - prethodni cvor u listi
    ///
    Point* getPrevPoint() const { return _prevPoint; }

    ///
    /// \brief Geter metod.
    /// \return - tacku koju trenutni cvor sadrzi
    ///
    const QPoint& getValue() const { return _value; }

private:
    Point& operator=(const Point &point);

    const QPoint _value;    // tacka koji se nalazi u trenutnom cvoru
    Point *_nextPoint;      // pokazivac na naredni cvor
    Point *_prevPoint;      // pokazivac na prethodni cvor
};

#endif // GA00_INCREMENTALINSERTION_H
