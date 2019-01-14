/*
Autor: Milica Đurić
Godina: 2018
Kratak opis problema: Za datih n tačaka u ravni odrediti konveksan omotač preko Chan-ovog algoritma složenosti O(nlogh)
                      gde je h broj tačaka u konveksnom omotaču.
*/

#ifndef GA07_CHANSALGORITHM_H
#define GA07_CHANSALGORITHM_H

#include "algorithms_practice/convexhull.h"
#include "algorithms_practice/ga02_grahamscan.h"

class ChansAlgorithm : public ConvexHull
{
public:
public:
    ChansAlgorithm(QWidget *pRenderer, int delayMs, std::string filename = "", int inputSize = DEFAULT_POINTS_NUM);
    void runAlgorithm();
    void drawAlgorithm(QPainter &painter) const;
    void runNaiveAlgorithm();

    /* getteri za konveksne omotače nađene naivnim i naprednim algoritmom */
    std::vector<QPoint> convexHullTest() const;
    std::vector<QPoint> convexHull() const;
private:
    std::vector<QPoint> _convexHull; //konveksni omotač izračunat optimalnim algoritmom
    QPoint _firstPoint; //najlevlja tačka, prva tačka koja se ubacuje u konveksni omotač
    QPoint _P; //poslednja tačka u trenutnom konveksnom omotaču, tačka iz koje se traže tangente
    QPoint _beforeP; //pretposlednja tačka u trenutnom konveksnom omotaču, potrebna zbog računanja ugla koji treba da se maksimizuje
    std::vector< std::vector<QPoint> > _partsOfPoints; //podela ulaznih tačaka na grupe od po m tačaka
    std::vector< std::vector<QPoint> > _hullsOfParts; //konveksni omotači nađeni preko GrahamScan-a za _partOfPoints
    bool _tangentsFound; //promenljiva koja pokazuje da li su tangente pronađene, potrebna zbog iscrtavanja istih
    std::vector<QPoint> _tangents; //sve tangente na konv. omotače iz _hullsOfParts iz trenutne tačke _P
    bool _drawConvexHull; //promenljiva koja pokazuje da li treba iscrtati poslednju tačku u konv. omotaču (jer prvo je potrebno da se tangente iscrtaju, pa se izabrana tang. boji crvenom bojom)

    /* Funkcija koja vraća prethodnika nekog broja po nekom modulu
     *  i      - broj čiji se prethodnik traži
     *  size   - broj po kojom modulu se traži prethodnik
     */
    unsigned returnPrevious(unsigned i, int size);
    /* Funkcija koja iscrtava konveksan omotač pronađem algoritmom GrahamScan
     *  points    - vektor tačaka koje pripadaju konveksnom omotaču koji se iscrtava
     *  painter   - painter koji iscrtava konv. omotač
     *  r         - r komponenta boje kojom se iscrtava konv. omotač
     *  g         - g komponenta boje kojom se iscrtava konv. omotač
     *  b         - b komponenta boje kojom se iscrtava konv. omotač
     */
    void drawGraham(const std::vector<QPoint>& points, QPainter &painter, int r, int g, int b) const;
    /* Funkcija koja poredi dve tangente iz tačke _P i vraća true ako tangenta iz prve prosleđene tačke gradi veći ugao od druge
     *  p1    -  tačka za koju se proverava da li je tangenta u njoj bolja od tangente u sledećem parametru
     *  p2    -  tačka sa kojom se poredi tangenta iz prvog argumenta
     */
    bool compareTangents(const QPoint& p1, const QPoint& p2);
    /* Funkcija koja proverava da li se u određenoj tački Q nalazi tangenta koja maksimizuje ugao (_beforeP, _P, Q) za dati konveksan omotač tačaka
     *  i        -  indeks tačke u vektoru za koju se proverava da li se u njoj nalazi tražena tangenta (tačka Q)
     *  points   -  vektor koji predstavlja konveksan omotač tačaka za koji se traži tangenta
     */
    bool isMaxPoint(unsigned i, const std::vector<QPoint>& points);
    /* Funkcija koja proverava da li se u prosleđenom segmentu nalazi tangenta koja maksimizuje ugao za prosleđeni konv. omotač
     *  start   - početak segmenta za koji se proverava da li se tangenta u njemu nalazi
     *  end     - kraj segmenta za koji se proverava da li se tangenta u njemu nalazi
     *  points  - vektor tačaka koje čine konveksan omotač za čiji se segment proverava da li se tangenta u njemu nalazi
     */
    bool containsMaxPoint(unsigned start, unsigned end, const std::vector<QPoint>& points);
    /* Funkcija koja binarnom pretragom nalazi tangentu iz tačke _P na određen konveksan omotač (tangenta koja se traži maksimizuje ugao (_beforeP, _P, Q), gde Q pripada prosleđenom konv. omotaču konv.), a vraća indeks tačke kroz koju prolazi tangenta
     *  points    - vektor tačaka koje predstavljaju konveksan omotač za koji se traži tangenta
     */
    int findTangent(const std::vector<QPoint>& points);
};

#endif // GA07_CHANSALGORITHM_H
