#include "ga03_linesegmentintersection.h"
#include <iostream>

LineSegmentIntersection::LineSegmentIntersection(QWidget *pRenderer, int delayMs, std::string filename, int dimension)
    :AlgorithmBase (pRenderer, delayMs), _eventQueue(), _statusQueue(StatusQueueComp(&_sweepLineY)), _intersections()
{
    (void)filename;
    _lineSegments = generateRandomLines(dimension);
    _sweepLineY = 0;

    //Dodavanje svih tacaka u eventQueue
    for(unsigned i=0; i<_lineSegments.size(); i++)
    {
        _eventQueue.insert({_lineSegments[i].p1(), &_lineSegments[i], nullptr, UPPER_POINT});
        _eventQueue.insert({_lineSegments[i].p2(), &_lineSegments[i], nullptr, LOWER_POINT});
    }
}

void LineSegmentIntersection::runAlgorithm()
{
    //Dok god ima neobradjenih dogadjaja
    while(!_eventQueue.empty())
    {
        //Preuzimamo sledeci dogadjaj i brisemo ga iz reda
        EventQueueNode eventNode = *_eventQueue.begin();
        _eventQueue.erase(_eventQueue.begin());
        QPointF intersection;

        //Ako je u pitanju gornje teme duzi
        if(eventNode.type == UPPER_POINT)
        {
            //Pomeramo brisucu pravu
            _sweepLineY = eventNode.event.y();
            AlgorithmBase_updateCanvasAndBlock();

            //Ubacujemo duz u status
            auto current_line = _statusQueue.insert(eventNode.line1).first;

            //Proveravamo presek sa prethodnom (ako je ima)
            if(_statusQueue.begin() != current_line)
            {
                auto prev = std::prev(current_line);
                if(utils::lineIntersection(**current_line, **prev, &intersection))
                {
                    if(eventNode.event != intersection && intersection.y() <= _sweepLineY)
                        _eventQueue.insert({intersection, *prev, *current_line, INTERSECTION});
                }
            }

            //Proveravamo presek sa sledecom u statusu (ako je ima)
            auto next = std::next(current_line);
            if(next != _statusQueue.end())
            {
                if(utils::lineIntersection(**current_line, **next, &intersection))
                {
                    if(eventNode.event != intersection && intersection.y() <= _sweepLineY)
                        _eventQueue.insert({intersection, *current_line, *next, INTERSECTION});
                }
            }
        }
        //Ako je u pitanju donje teme
        else if(eventNode.type == LOWER_POINT)
        {
            //Pomeramo brisucu pravu
            _sweepLineY = eventNode.event.y();
            AlgorithmBase_updateCanvasAndBlock();

            //Pronalazimo duz koju treba obrisati iz statusa
            auto linePos = _statusQueue.find(eventNode.line1);

            //Proveravamo presek njene prethodne i sledece u statusu (jer one postaju novi susedi)
            if(linePos != _statusQueue.begin())
            {
                auto prev = std::prev(linePos);
                auto next = std::next(linePos);
                if(next != _statusQueue.end())
                {
                    if(utils::lineIntersection(**prev, **next, &intersection))
                    {
                        if(eventNode.event != intersection && intersection.y() <= _sweepLineY)
                            _eventQueue.insert({intersection, *prev, *next, INTERSECTION});
                    }
                }
            }

            //Brisemo duz iz statusa
            deleteFromStatus(eventNode.line1);
        }
        else //Ako je u pitanju presecna tacka
        {
            //Dodajemo tacku u niz presecnih tacaka (zbog iscrtavanja)
            _intersections.push_back(eventNode.event);
            AlgorithmBase_updateCanvasAndBlock();

            //Razmenjujemo duzi koje se u toj tacki seku
            //Tako sto ih brisemo, pa pomeramo brisucu pravu pa ih opet ubacimo u status
            deleteFromStatus(eventNode.line1);
            deleteFromStatus(eventNode.line2);

            //Pomeramo brisucu pravu
            _sweepLineY = eventNode.event.y();
            AlgorithmBase_updateCanvasAndBlock();

            //Vracamo duzi u status
            auto pLine2 = _statusQueue.insert(eventNode.line1).first;
            auto pLine1 = _statusQueue.insert(eventNode.line2).first;

            //Proveravamo preseke sa novim susedima
            auto next = std::next(pLine2);
            if(next != _statusQueue.end())
            {
                if(utils::lineIntersection(**pLine2, **next, &intersection))
                {
                    if(intersection != eventNode.event && intersection.y() < _sweepLineY)
                        _eventQueue.insert({intersection, *pLine2, *next, INTERSECTION});
                }
            }
            if(pLine2 != _statusQueue.begin())
            {
                auto prev = std::prev(pLine2);
                if(utils::lineIntersection(**prev, **pLine2, &intersection))
                {
                    if(intersection != eventNode.event && intersection.y() < _sweepLineY)
                        _eventQueue.insert({intersection, *prev, *pLine2, INTERSECTION});
                }
            }


            next = std::next(pLine1);
            if(next != _statusQueue.end())
            {
                if(utils::lineIntersection(**pLine1, **next, &intersection))
                {
                    if(intersection != eventNode.event && intersection.y() < _sweepLineY)
                        _eventQueue.insert({intersection, *pLine1, *next, INTERSECTION});
                }
            }

            if((pLine1 != _statusQueue.begin()))
            {
                auto prev = std::prev(pLine1);
                if(utils::lineIntersection(**prev, **pLine1, &intersection))
                {
                    if(intersection != eventNode.event && intersection.y() < _sweepLineY)
                        _eventQueue.insert({intersection, *prev, *pLine1, INTERSECTION});
                }
            }

        }
    }
    _statusQueue.clear();
    _sweepLineY = 0;
    AlgorithmBase_updateCanvasAndBlock();
    emit animationFinished();
}

void LineSegmentIntersection::drawAlgorithm(QPainter &painter) const
{
    QPen blue = painter.pen();
    blue.setColor(Qt::blue);
    blue.setWidth(2);
    QPen green = painter.pen();
    green.setColor(Qt::green);
    green.setWidth(2);
    QPen yellow = painter.pen();
    yellow.setColor(Qt::yellow);
    yellow.setWidth(5);

    //Iscrtavamo sve duzi
    for(const QLineF& l: _lineSegments)
    {
        painter.drawLine(l);
    }

    //Iscrtavamo trenutnu poziciju brisuce prave
    painter.setPen(green);
    painter.drawLine(0, _sweepLineY, 2000, _sweepLineY);

    //Iscrtavamo trenutno stanje statusa
    int i=1;
    painter.setPen(blue);
    for(const QLineF* l: _statusQueue)
    {
        painter.drawLine(*l);
        painter.drawText(l->center(), QString::number(i++));
    }

    //Isrtavamo sve presecne tacke koje smo nasli
    painter.setPen(yellow);
    for(const QPointF p: _intersections)
    {
        painter.drawPoint(p);
    }

    painter.setPen(blue);
    painter.drawText(10,20, "Plavom bojom su obojene duzi koje se trenutno nalaze u statusu.");
}

void LineSegmentIntersection::runNaiveAlgorithm()
{

}

void LineSegmentIntersection::deleteFromStatus(QLineF *l)
{
//    _sweepLineY += 0.5;
//    _statusQueue.erase(l);
//    _sweepLineY -= 0.5;
    for(auto it = _statusQueue.begin(); it != _statusQueue.end(); ++it)
    {
        if(*it == l)
        {
            _statusQueue.erase(it);
            break;
        }
    }
}

StatusQueueComp::StatusQueueComp(double *ypos)
{
    _ypos = ypos;
}
