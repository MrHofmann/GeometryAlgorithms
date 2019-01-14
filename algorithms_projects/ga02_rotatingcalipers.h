/*
Autor: Kristina Stanojević
Godina: 2018
Kratak opis problema: Pronalazak minimalnog pravougaonika koji
                      sadrži dati konveksni poligon sa n temena
                      koristeći algoritam rotating calipers.
*/


#ifndef GA02_ROTATINGCALIPERS_H
#define GA02_ROTATINGCALIPERS_H

#include "algorithmbase.h"
#include <math.h>
#include <iostream>
#include "algorithms_practice/ga02_grahamscan.h"


/*
 * Klasa Caliper je prava, ima index tacke (iz konv.omotaca) koju sadrzi
 * i ugao koji pravi ta prava sa x osom
 * sto nam je potrebno za tngA (tj ono k u formuli y = kx + n)
 * u koraku racunanja temena pravougaonika
 * po formuli tngA = (y2-y1)/(x2-x1).
 * Koristeci dve takve prave tj. dva calipera i dve ovakve odgovarajuce
 * formule nalazimo tacku koja predstavlja teme jednog od pravougaonika.
 */

class RotatingCalipers : public ConvexHull
{
public:
    RotatingCalipers(QWidget *pRenderer, int delayMs, std::string filename = "", int inputSize = DEFAULT_POINTS_NUM);

    std::vector<QPoint> convexHull() const;
public:
    class Caliper;
    void runAlgorithm();
    void drawAlgorithm(QPainter &painter) const;
    void runNaiveAlgorithm();
    double calculateArea(std::vector<QPoint> rectangle);
    double findSmallestAngle(Caliper c1, Caliper c2, Caliper c3, Caliper c4);

private:
    //najmanji pravougaonik koji trazimo
    QLine AB;
    QLine BC;
    QLine CD;
    QLine DA;
    /*Cetiri tacke sa min i max koordinataman x i y*/
    QPoint _x_min;
    QPoint _y_min;
    QPoint _x_max;
    QPoint _y_max;
    int _index_down;
    int _index_up;
    int _index_right;
    int _index_left;
    /*Temena pravougaonika i ivice*/
    QPoint A, B, C, D;
    QLine _AB;
    QLine _BC;
    QLine _CD;
    QLine _DA;

    /* Konveksni omotac */
    std::vector<QPoint> _convexHull;
    void drawGraham(const std::vector<QPoint>& points, QPainter &painter, int r, int g, int b) const;

public:
    class Caliper {
    public:
        std::vector<QPoint> _convexHull;
        int _pointIndex;
        double _angle;


        Caliper(std::vector<QPoint> convexHull, int pointIndex, double angle) {
            this->_convexHull = convexHull;
            this->_pointIndex = pointIndex;
            this->_angle = angle;
        }


        /*stepen u radijane: (_angle * M_PI) / 180
        funkcija vraca k iz formule y = kx + n, tj. tangens ugla prave*/
        double slope()
        {
            return tan((_angle * M_PI) / 180);
        }


        /*odvojeno dodatno racunanje za racunanje presecne tacke - temena*/
        double additionalCalculation()
        {
            QPoint point = _convexHull.at(_pointIndex);
            return slope() * point.x() - point.y();
        }


        /*trazi presecnu tacku dva kalipera tj. teme pravougaonika*/
        QPoint findIntersectionPoint(Caliper caliper)
        {
            double x, y;

            /*koriste se izvedene formule iz sistema jednacina tgA = (y1 - y) / (x1 - x) i tbB = (y2 - y) / (x2 - x)
            nakon sredjivanja:
            x = (tgA*x1 - y1 + tgB*x2 - y2) / (tgB + tgA)
            y = (tgA*(tgB*x2 - y2) - tgB*(tgA*x1 - y1)) / (tgB - tgA) */

            x = (caliper.additionalCalculation() + this->additionalCalculation()) / (caliper.slope() + this->slope());
            y = (caliper.slope() * this->additionalCalculation() - this->slope() * caliper.additionalCalculation()) / (this->slope() - caliper.slope());

            return QPoint(x, y);
        }


        /*racuna ugao koji pravi trenutni caliper sa mogucim sledecim caliperom*/
        double calculateAngle()
        {
            //ugao izmedju jednog calipera i sledeceg calipera (koji bi nastao
            //nakon rotacije tog pocetnog) se moze naci pomocu funkcije
            //atan2(y, x) koja vraca ugao izmedju x-ose i vektora (y, x),
            //a to je nama tacka kroz koju prolazi caliper.
            //Onda je ugao izmedju dva calipera zapravo angle = alfa2 - alfa1
            //odnosno angle = atan2(y2, x2) - ugaoPoznatogCalipera

            int thisIndex = _pointIndex;
            int nextIndex = (_pointIndex + 1) % _convexHull.size();

            QPoint caliperPoint = _convexHull.at(thisIndex);
            QPoint nextPoint = _convexHull.at(nextIndex);

            double angleNextCaliper;

            double deltaX = nextPoint.x() - caliperPoint.x();
            double deltaY = nextPoint.y() - caliperPoint.y();

            //trazimo ugao koji bi zaklapao sledeci susedni caliper sa x-osom
            angleNextCaliper = atan2(deltaY, deltaX) * 180 / M_PI;
            //normalizacija
            double rotatingAngle = angleNextCaliper < 0? 360 + angleNextCaliper : angleNextCaliper;

            //kao sto je vec receno,
            //ugao izmedju trenutnog i sledeceg calipera se dobija kada se od
            //ugla koji sledeci caliper zaklapa sa x-osom oduzme ugao
            //trenutnog calipera sa x-osom
            rotatingAngle = rotatingAngle < 0? 360 + rotatingAngle - _angle : rotatingAngle - _angle;

            return rotatingAngle < 0 ? 360 : rotatingAngle;
        }


        /*rotiranje calipera za ugao*/
        void rotate(double angle)
        {
            //ako je ugao tog calipera onaj za koji rotiramo,
            //pomeri ga na sledecu tacku konveksnog omotaca
            if(this->calculateAngle() == angle)
            {
                _pointIndex++;
            }

            //povecaj ugao
            this->_angle += angle;        

            std::cout << this->_angle << ", " << angle << ", "<<_pointIndex<< std::endl;
        }

    };

};


#endif // GA02_ROTATINGCALIPERS_H
