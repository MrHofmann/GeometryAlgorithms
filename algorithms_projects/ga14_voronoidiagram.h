#ifndef GA14_VORONOIDIAGRAM_H
#define GA14_VORONOIDIAGRAM_H


/*
Autor: Filip Novovic 1093/2016
Godina: 2017
Kratak opis problema: Naci Voronoj Dijagram za zadatih n tacaka.
*/

#include "algorithmbase.h"
#include <QDebug>
#include <algorithm>
#include <map>
#include <set>

class Intersection;

//tipovi dogadjaja koje obradjujemo
enum class VoronoiEventType
  {
     Site,
     Circle
  };

//dogadjaj u Forcenovom algoritmu
class VoronoiEventPoint{
    //tip dogadjaja
    VoronoiEventType type;
    //tacka dogadjaja (koordinate)
    QPointF point;
    //jedinstvena oznaka sedista (indeks nakon sortiranja)
    int index;
    //tekst koji pise ispod dogadjaja kruga u obliku "(levo sediste, srednje sediste, desno sediste)"
    QString circleEventText;
    //centar kruga kome je tacka kruga donja tacka
    QPointF* circleEventUpperPoint;
    //preseci koji nestaju u slucaju kruga
    Intersection* leftInt,*rightInt;
public:
    //konstruktor za dogadjaj sedista
    VoronoiEventPoint(QPointF iPoint,VoronoiEventType iType,int iIndex);
    //konstruktor za dogadjaj kruga
    VoronoiEventPoint(QPointF iPoint, QPointF* iupperPoint,VoronoiEventType iType,QString circInfo,Intersection* iLeftInt,Intersection* irightInt);
    //kopi konstruktor
    VoronoiEventPoint(VoronoiEventPoint* duplPoint);
    //geter za tip voronoj dogadjaja
    VoronoiEventType getType();
    //geter za tacku voronoj dogadjaja
    QPointF getPoint();
    //geter za indeks voronoj dogadjaja
    int getIndex();
    //geter za tekst kruga
    QString getCircleText();
    //seter za tekst kruga
    void setCircleText(QString str);
    //geter za centar kruga (kod dogadjaja kruga)
    QPointF* getCircleCenter();
    //geter za levi presek (kod kruga)
    Intersection* leftIntersection();
    //geter za desni presek (kod kruga)
    Intersection* rightIntersection();
};

//red dogadjaja
class EventQueue{
    //niz koji interno cuva red dogadjaja
    std::vector<VoronoiEventPoint*> internalEvents;
public:
    //uklanjanje dogadjaja (koristi se u slucaju dogadjaja sedista)
    void removeEvent(VoronoiEventPoint* point);
    //uklanjanje dogadjaja prema kodu (koristi se kod kruga), format koda je "(levo sediste, srednje sediste, desno sediste)"
    void removeCircleByCode(QString code);
    //provera da li je red dogadjaja prazan
    bool isEmpty();
    //dodavanje dogadjaja u red
    void addEvent(VoronoiEventPoint* point);
    //vracanje dogadjaja na nultoj poziciji kako bi se obradio (ovde se vraca i brise iz reda dogadjaja)
    VoronoiEventPoint* getEventAt();
    //citanje dogadjaja sa zadatim indeksom
    VoronoiEventPoint* readEventAt(int i);
    //vracanje velicine reda dogadjaja
    int size();
};


//klasa koja cuva presek dve parabole
class Intersection{
public:
    //konstruktor koji podesava sediste leve parabole i sediste desne parabole
    Intersection(VoronoiEventPoint* iA, VoronoiEventPoint* iB);
    //konkretna tacka koja oznacava pocetno teme voronoj grane
    QPointF* startPoint;
    //konkretna tacka koja oznacava krajnje teme voronoj grane
    QPointF* endPoint;
    //sediste leve parabole
    VoronoiEventPoint* A;
    //sediste desne parabole
    VoronoiEventPoint* B;
    //izracunavanje tacke preseka u odnosu na vrednost sweep line-a u trenutku poziva
    QPointF* getIntersectionPoint();
    //vracanje indeksa sedista A i B u citljivom formatu
    QString getIndexes();
    //voronoj dogadjaj koji predstavlja vezu ka dogadjaju kruga sa leve strane
    VoronoiEventPoint* Acirc=nullptr;
    //voronoj dogadjaj koji predstavlja vezu ka dogadjaju kruga sa desne strane
    VoronoiEventPoint* Bcirc=nullptr;
};

//struktura koja se koristi kao komparator u mapi za uredjenje preseka na frontu
struct cmpInt {
    bool operator()(Intersection* a, Intersection* b);
};

//struktura koja se koristi kao komparator u setu
struct cmpPoints {
    bool operator()(QPointF a, QPointF b);
};

//klasa koja cuva sve strukture vezane za Forcenov algoritam
class VoronoiDiagram : public AlgorithmBase
{
public:
    //oznacava flag za debagovanje naivnog algoritma
    bool drawMap=false;
    //tacke koje su deo voronoj dijagrama
    std::vector<QPointF*> tackeVoronoiDijagrama;
    //linije koje su deo voronoj dijagrama
    std::vector<Intersection*> linijeVoronoiDijagrama;
    //svi aktuelni preseci na frontu
    std::map<Intersection*,Intersection*,cmpInt> frontPreseci;
    //vrednost koja oznacava da li postoji prva tacka na frontu ili ne
    //(ako postoji onda se na njoj racunaju preseci, ako ne postoji onda se na nju postavlja aktivno teme)
    VoronoiEventPoint* onlyPoint=nullptr;

    VoronoiDiagram(QWidget* pRenderer, int delayMs, std::string filename = "",
             int inputSize = DEFAULT_POINTS_NUM);
    void runAlgorithm();
    void drawAlgorithm(QPainter &painter) const;
    void runNaiveAlgorithm();

    //metod za obradu dogadjaja sedista
    void processSiteEvent(std::vector<VoronoiEventPoint*> eventP,EventQueue * allEvents);
    //metod za obradu dogadjaja kruga
    void processCircleEvent(std::vector<VoronoiEventPoint*> eventP,EventQueue * allEvents);
    //naivni algoritam
    std::map<int,std::vector<QPointF*>*>* naiveMap(bool testing,double maxX,double maxY);

    //unit test
    bool unitTest=false;
    //izlaz za unit testove Forcenov
    std::vector<QPointF> unitForcen;
    //izlaz za unit testove Naivni
    std::vector<QPointF> unitNaivni;
    //naivni set
    std::set<QPointF,cmpPoints> tackeNaivnogSet;
    //maksimalne dimenzije ekrana naivnog
    double naiveMaxX,naiveMaxY;

private:
    //red svih dogadjaja (i sedista i kruga, sortiranih po y osi)
    EventQueue * allEvents=new EventQueue();
    //promenljiva u kojoj se pri debagovanju naivnog algoritma moze cuvati rezultat izvrsavanja algoritma
    std::map<int,std::vector<QPointF*>*>* mapPointHull;
    //oznaka da li bi trebalo iscrtavati poligon u naivnom algoritmu
    bool currentPolygonSet=false;
    //oznaka da li se vrsi debagovanje naivnog algoritma
    bool naiveDebug = false;
    //aktivni poligon koji se obradjuje u naivnom algoritmu
    std::vector<QPointF*>* currentPolygon= new std::vector<QPointF*>();
    //metod za ciscenje rezultata
    void clearResult();
    //ulaz u algoritam
    std::vector<QPoint> points;
    //dogadjaji sedista
    std::vector<VoronoiEventPoint*> eventPoints;
    //oznaka da li se obradjuje neki dogadjaj
    bool currentEventSet;
    //dogadjaj koji se obradjuje
    VoronoiEventPoint* currentEvent;
};

#endif // GA14_VORONOIDIAGRAM_H
