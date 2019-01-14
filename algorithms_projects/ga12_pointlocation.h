/***
 * Autor: Marinela Parovic
 * Broj indeksa: 1008/2017
 * Godina: 2018
 * Kratak opis problema: Dato je n nepresijecajucih duzi u ravni koje indukuju planarno
 * razlaganje ravni na obasti. Potrebno je za zadatu tacku odrediti kojoj od oblasti
 * pripada.
 */

#ifndef GA12_TRAPEZOIDMAP_H
#define GA12_TRAPEZOIDMAP_H

#include <algorithm>
#include <QPointF>
#include <QLineF>
#include <fstream>
#include "algorithmbase.h"
#include "algorithms_practice/ga04_dcel.h"
#include "ga12_datastructures.h"
#include "unordered_set"
#include "functional"


//trapezna mapa za razlaganja ravni datom nizom nepresijecajucih duzi
class TrapezoidMap : public AlgorithmBase {
public:

    //enum koji sluzi za cuvanje informacije o validnosti ulaza za algoritam
    enum AlgorithmStatus {INVALID_INPUT, CORRECT_INPUT};

    //konstruktor
    TrapezoidMap(QWidget *pRenderer, int delayMs, QPointF locateMe, std::string filename="", int dimension=10);

    //destruktor
    ~TrapezoidMap() {}

    //locitanje zadate tacke koriscenjem trapezne mape
    const Trapezoid* pointLocation(const QPointF& p);

    //azuriranje crteza
    void updateMyDrawing();

    //metod kojim se dobijaju svi trapezi u mapi
    void getTrapezoids(std::vector<Trapezoid*>& trapezoids) const;

    //brisanje mape
    void makeEmpty();

    //metod za generisanje ulaza za algoritam iz fajla cije je ime argument
    void makeInputFromFile(std::string filename);

    //metod za generisanje nasumicnog ulaza za algoritam sa dimension tacaka
    void makeRandomInput(int dimension);

    //true ako medju duzima koje su ulaz ima presjeka
    bool checkInputSegments();

    // AlgorithmBase interface
    void runAlgorithm();
    void drawAlgorithm(QPainter &painter) const;
    void runNaiveAlgorithm();

    //getters and setters
    std::vector<Segment> getSegments() const;
    void setSegments(const std::vector<Segment> &segments);

    AlgorithmStatus getStatus() const;
    void setStatus(const AlgorithmStatus &status);

    std::string getFieldWithPoint() const;
    void setFieldWithPoint(const std::string &fieldWithPoint);

    QPointF getPointLocation() const;
    void setPointLocation(const QPointF &pointLocation);

private:
    //metod za dodavanje nove duzi u mapu
    void addSegment(Segment* segment);

    //pomocni metod za lociranje tacke
    SearchNode* query(QPointF p1, QPointF p2);

    //slucaj kada je duz sadrzana u jednom trapezu
    void simpleCase(SearchNode* trapNode, Segment* segment);

    //slucaj kada duz sijece vise trapeza
    //krajnje lijevi je leftMost, a krajnje desni je rightMost
    void generalCase(SearchNode* leftMost, SearchNode* rightMost, Segment* segment);

    //tacka koju korisnik zadaje za lociranje
    QPointF _pointLocation;
    //korijen strukture pretrage
    SearchNode* _root;
    //niz duzi za kreiranje trapezne mape
    std::vector<Segment> _segments;

    //promjenljiva za generisanje identifikatora oblasti u planarnom razlaganju
    int _fieldNumber = 0;

    AlgorithmStatus _status;
    //pomocne promjenljive za vizuelizaciju algoritma
    bool _nextSeg = true;
    bool _currentNode = false;
    bool _currentEndPoint = false;
    SearchNode *_currentNodeSearching = nullptr;
    QPointF _currentPointSearching;
    bool _locatingPoint = false;
    bool _ready;
    int _segmentsAdded = 0;
    std::string _fieldWithPoint;

};

#endif // GA12_TRAPEZOIDMAP_H
