/*
Autor: Saša Bukurov 1064/2015
Godina: 2017
Kratak opis problema: Za dato platno dimenzije N*M i niz tačaka na platnu, naći najveći prazan pravugaonik.
*/

#ifndef GA23_LARGESTEMPTYRECTANGLE_H
#define GA23_LARGESTEMPTYRECTANGLE_H

#include "algorithmbase.h"
#include <set>
#include <stack>




class LargestEmptyRectangle : public AlgorithmBase
{
public:
    LargestEmptyRectangle(QWidget *pRenderer, int delayMs,  std::string filename = "", int inputSize=20, int maxX=1200, int maxY=500);
    void runAlgorithm();
    void drawAlgorithm(QPainter &painter) const;
    void runNaiveAlgorithm();
	///
	/// Računanje površine datog pravugaonika
	///
    int area(QPoint ll, QPoint ur);
	///
	/// Provera da li je pravugaonik prazan tj, da li se nalazi neka tačka u njemu ili ne.
	///
    bool areaIsEmpty(QPoint ll, QPoint ur);
	///
	/// Menjanje stanja keš niza tj linije kolizije.
	///
    void updateCashe(std::vector<int>& cache, std::vector<std::vector<int> > &matrix, int x);
	///
	///Inicijalizacija matrice koja pretstavlja platno.
	///    
	void updateMatrix(std::vector<std::vector<int>> &matrix);

    QPoint getBest_ll() const;
    void setBest_ll(const QPoint &value);

    QPoint getBest_ur() const;
    void setBest_ur(const QPoint &value);


    int getBestArea() const;
    void setBestArea(int value);


    int getWidth() const;

    int getHeight() const;

private:
	///
	///tačke ma platnu
	///
    std::vector<QPoint> _points;
	///
	///tačke najvećeg pravugaonika
    ///
	QPoint best_ll, best_ur;
	///
	///Površina najvećeg pravugaonika
	///
    int bestArea;
	///
	///Dimenzije ekrana
	///
    int _width, _height;
};

#endif // GA23_LARGESTEMPTYRECTANGLE_H
