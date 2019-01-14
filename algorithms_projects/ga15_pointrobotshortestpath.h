/*
Autor: Matei Jon Stancu 1137/2015
Godina: 2018
Kratak opis problema:   Dat je skup prepreka u ravni i dve tacke, potrebno je naci najkraci put izmedju datih tacaka,
                        takav da ne sece unutrasnjost datih prepreka.
*/
#ifndef GA15_POINTROBOTSHORTESTPATH_H
#define GA15_POINTROBOTSHORTESTPATH_H

#include "algorithmbase.h"
#include "ga15_datastructures.h"
#include <limits>
#include <set>
#include <queue>

///
/// \brief The PointRobotShortestPath class - klasa koja implementira algoritam najkraceg puta izmedju dve tacke za tackastog robota
///
class PointRobotShortestPath : public AlgorithmBase
{
public:
    ///
    /// \brief PointRobotShortestPath   - konstruktor za animaciju
    /// \param pRender                  - prozor na kojem se iscrtava animacija
    /// \param delayMs                  - osvezavanje animacije
    /// \param fileName                 - putanja do ulazne datoteke
    /// \param search                   - tip pretrage
    /// \param start                    - pocetna tacka
    /// \param end                      - odrediste
    ///
    PointRobotShortestPath(QWidget *pRender, int delayMs, std::string fileName, searchType search, QPoint start, QPoint end);
    ///
    /// \brief PointRobotShortestPath   - konstruktor za testiranje
    /// \param pRender                  - prozor na kojem se iscrtava animacija
    /// \param delayMs                  - osvezavanje animacije
    /// \param fileName                 - putanja do ulazne datoteke
    /// \param search                   - tip prerage
    /// \param size                     - dimenzija problema
    ///
    PointRobotShortestPath(QWidget *pRender, int delayMs, std::string fileName, searchType search, int size);

    ///
    /// \brief runAlgorithm             - implementacija naprednog algoritma
    ///
    void runAlgorithm();
    ///
    /// \brief drawAlgorithm            - iscrtavanje vizualizacije algoritma
    /// \param painter                  - QPainter
    ///
    void drawAlgorithm(QPainter &painter) const;
    ///
    /// \brief runNaiveAlgorithm        - implementacija naivnog algoritma
    ///
    void runNaiveAlgorithm();

    ///
    /// \brief pointInsidePolygon       - provera da li tacka pripada unutrasnjosti poligona
    /// \param p1                       - tacka koja se ispituje
    /// \return                         - tacno ako je tacka unutar poligona, netacno inace
    ///
    bool pointInsidePolygon(QPointF p1);

    ///
    /// \brief shortestPath             - geter za najkraci put
    /// \return                         - vector koji cuva najkraci put izmedju _pStart i _pEnd
    ///
    std::vector<vertex *> shortestPath() const;
    ///
    /// \brief shortestPathLength       - geter za duzinu najkraceg puta
    /// \return                         - duzina najkraceg puta
    ///
    double shortestPathLength() const;

private:
    ///
    /// \brief naiveVisibilityGraph     - funkcija koja racuna graf vidljivosti naivnim pristupom
    ///
    void naiveVisibilityGraph();
    ///
    /// \brief naiveShortestPath        - funkcija koja racuna najkraci put naivnim pristupom
    ///
    void naiveShortestPath();
    ///
    /// \brief improvedVisibilityGraph  - funkcija koja racuna graf vidljivosti naprednim pristupom
    ///
    void improvedVisibilityGraph();
    ///
    /// \brief improvedShortestPath     - funkcija koja racuna najkraci put naprednim pristupom
    ///
    void improvedShortestPath();
    ///
    /// \brief updateSweepLine          - funkcija koja azurira polozaj brisuce prave
    /// \param p1                       - prva tacka koja definise brisucu pravu
    /// \param p2                       - druga tacka koja definise brisucu pravu
    ///
    void updateSweepLine(const QPoint p1, const QPoint p2);
    ///
    /// \brief initDataQueues           - inicijalizacija strukutura podataka
    /// \param w                        - cvor grafa u odnosu na kog se vrsi inicijalizacija
    ///
    void initDataQueues(vertex *w);
    ///
    /// \brief deleteFromStatus         - funkcija koja brise duz iz statusa u linearnom vremenu
    /// \param l                        - duz koja se brise
    ///
    void deleteFromStatus(QLine l);
    ///
    /// \brief deleteFromStatus         - funkcija koja brise duz iz statusa u logaritamskom vremenu
    /// \param l                        - duz koja se brise
    ///
    void deleteFromStatus2(QLine l);
    ///
    /// \brief updateStatusQueue        - funkcija koja azurira status
    /// \param v                        - cvor grafa u odnosu na kog se vrsi azuriranje
    ///
    void updateStatusQueue(vertex *v);
    ///
    /// \brief isVisible                - funkcija koja ispituje vidljivost cvora
    /// \param v                        - cvor iz kog se ispituje vidljivost
    /// \param w                        - cvor do kog se ispituje vidljivost
    /// \param prevVertex               - prethodni cvor za kog je ispitano da li je vidljiv
    /// \param prevVisible              - vidljivost prethodnog cvora
    /// \return                         - tacno ako je cvor w vidljiv iz v, netacno inace
    ///
    bool isVisible(const vertex *v, const vertex *w, vertex *prevVertex, bool prevVisible);

    ///
    /// \brief generateObstacleGrid     - funkcija koja generise instance za testiranje
    /// \param rows                     - broj redova matrice prepreka
    /// \param cols                     - broj kolona matrice prepreka
    /// \return                         - putanja do generisane datoteke
    ///
    std::string generateObstacleGrid(unsigned rows, unsigned cols) const;

    ///
    /// \brief _obstacles               - skup prepreka u DCEL formatu
    ///
    DCEL _obstacles;
    ///
    /// \brief _visibilityGraph         - graf vidljivosti
    ///
    Graph _visibilityGraph;
    ///
    /// \brief _pStart                  - tacka pocetnog polozaja
    ///
    QPoint _pStart;
    ///
    /// \brief _pEnd                    - tacka odredista
    ///
    QPoint _pEnd;
    ///
    /// \brief _sweepLine               - brisuca prava
    ///
    QLine _sweepLine;

    ///
    /// \brief _shortestPath            - najkraci put izmedju _pStart i _pEnd u _visibilityGraph
    ///
    std::vector<vertex*> _shortestPath;
    ///
    /// \brief _lineSegments            - skup ivica prepreka
    ///
    std::vector<QLineF> _lineSegments;
    ///
    /// \brief _eventQueue              - struktura red dogadjaja
    ///
    std::set<EventQueueVertex, EventQueueAngleComp> _eventQueue;
    ///
    /// \brief _statusQueue             - struktura status preseka ivica prepreka sa brisucom pravom
    ///
    std::multiset<QLineF, StatusQueueCompare> _statusQueue;
    ///
    /// \brief _searchQueue             - struktura red sa prioritetom za pretragu najkraceg puta
    ///
    std::priority_queue<vertex*, std::vector<vertex*>, SearchQueueCompare> _searchQueue;

    ///
    /// \brief _search                  - tip pretrage
    ///
    searchType _search;
};

#endif // GA15_POINTROBOTSHORTESTPATH_H
