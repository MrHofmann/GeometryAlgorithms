#ifndef GA03_DATASTRUCTURES_H
#define GA03_DATASTRUCTURES_H

#include <QPointF>
#include <QLineF>

#include "utils.h"

/*******************************EVENT QUEUE*****************************/

/* Enumeracija tipova tacaka */
enum EventType {UPPER_POINT, LOWER_POINT, INTERSECTION};

/* Struktura koja opisuje jedan dogadjaj:
    event - koordinate tacke
    type - tip dogadaja
    U slucaju da je u pitanu gornja ili donja tacka,
    line1 sadrzi pokazivac na duz kojoj ta tacka pripada,
    a line2 je nullptr
    U slucaju da je u pitanu tacka preseka,
    line1 i line2 su duzi koje se u njoj seku
*/
struct EventQueueNode
{
    QPointF event;
    QLineF* line1;
    QLineF* line2;
    EventType type;
};

/* Funktor za poredjenje dva dogadjaja */
struct EventQueueComp
{
    bool operator()(const EventQueueNode lhs, const EventQueueNode rhs) const
    {
        if(lhs.event.y() != rhs.event.y())
            return lhs.event.y() > rhs.event.y();
        else
            return lhs.event.x() < rhs.event.x();
    }
};


/*******************************STATUS QUEUE*****************************/

/*
Funktor za poredjenje dve duzi.
Duzi poredimo po mestu preseka sa brisucom pravom.
(Preciznije: x kooridinati preseka sa brisucom pravom)
Iz tog razloga nam treba podatak o trenutnoj poziciji brisuce prave.
Taj podatak cemo dobiti tako sto cemo na pocetku u _ypos staviti
adresu y koordinate nase brisuce prave.
*/
struct StatusQueueComp
{
private:
    double* _ypos;
public:
    StatusQueueComp(double * ypos);

    bool operator()(const QLineF* lhs, const QLineF* rhs) const
    {
        QLineF sweepLine = QLineF(0, *_ypos-0.5, 2000, *_ypos-0.5);
        QPointF i1, i2;
        utils::lineIntersection(sweepLine, *lhs, &i1);
        utils::lineIntersection(sweepLine, *rhs, &i2);
        return i1.x() < i2.x();
    }
};

#endif // GA03_DATASTRUCTURES_H
