/* -----------------------------------------------------
    Autor: Djordje Milicevic
    Godina: 2018.
    Opis problema: QuickHull algoritam
                   za konstrukciju konveksnog omotaca
                   skupa tacaka u ravni
----------------------------------------------------- */

#ifndef GA05_QUICKHULL_H
#define GA05_QUICKHULL_H

#include "algorithms_practice/convexhull.h"

#define TOP_SIDE 1
#define BOTTOM_SIDE -1
#define LEFT_SIDE 1
#define RIGHT_SIDE -1
#define ON_THE_LINE 0
#define NOT_FOUND -1


class QuickHull : public ConvexHull
{
public:    
    QuickHull(QWidget *pRenderer, int delayMs, std::string filename = "", int inputSize = DEFAULT_POINTS_NUM);

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
    /// \return - vektor koji sadrzi tacke koje je pronasao naivni algoritam
    ///
    std::vector<QPoint> getConvexHullTest() const;

    ///
    /// \brief Geter metod.
    /// \return - vektor koji sadrzi tacke koje je pronasao napredni algoritam
    ///
    std::vector<QPoint> getConvexHull() const;

    ///
    /// \brief Metod za osvezavanje platna.
    ///
    void updateCanvas();

    ///
    /// \brief Rekurzivna funkcija za pronalazenje konveksnog omotaca.
    /// \param p1 - leva tacka duzi
    /// \param p2 - desna tacka duzi
    /// \param points - podskup polaznog skupa tacaka koji se razmatra u rekurzivnom pozivu
    /// \param canDeleteLine - indikator koji oznacava da li se neka od linija starog konveksnog omotaca moze ukloniti prilikom iscrtavanja
    /// \return - logicka vrednost koja oznacava da li je bilo tacaka za obradu u rekurzivnom pozivu
    ///
    bool findHull(const QPoint &p1, const QPoint &p2, const std::vector<const QPoint*> &points, bool canDeleteLine);

private:
    const QPoint *_minPoint;            // tacka sa minimalnom x koordinatom (koristi se prilikom vizuelizacije)
    const QPoint *_maxPoint;            // tacka sa maksimalnom x koordinatom (koristi se prilikom vizuelizacije)
    const QPoint *_pointToHighlight;    // tacka cije se rastojanje od prave trenutno ispituje (koristi se prilikom vizuelizacije)
    bool _findingMaxPointIndicator;     // indikator trazenja najudaljenije tacke (koristi se prilikom vizuelizacije)

    std::vector<QPoint> _convexHull;    // vektor koji sadrzi tacke koje pripadaju konveksnom omotacu
    std::vector<std::pair<QPoint, QPoint>> _pointDrawVector;    // vektor koji sadrzi tacke koje oznacavaju duz
};

#endif // GA05_QUICKHULL_H
