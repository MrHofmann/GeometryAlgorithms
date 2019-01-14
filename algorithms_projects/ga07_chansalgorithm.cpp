#include "ga07_chansalgorithm.h"
#include "algorithms_practice/ga02_grahamscan.h"
#include "utils.h"

ChansAlgorithm::ChansAlgorithm(QWidget *pRenderer, int delayMs, std::string filename, int inputSize)
    : ConvexHull(pRenderer, delayMs, filename, inputSize)
{
    _tangentsFound = false;
    _drawConvexHull = false;

    //resavanje problema duplikata tacaka
    auto it = std::unique(_points.begin(), _points.end());
    _points.resize(std::distance(_points.begin(), it));
}

void ChansAlgorithm::runAlgorithm()
{
   //ako imamo manje od 3 tacke, izlaz je prazan
   if (_points.size() < 3)
   {
      emit animationFinished();
      return;
   }

   //računanje prve tačke koja je najlevlja tacka
   _firstPoint = _points[0];
   for(unsigned i=1; i < _points.size(); i++){
       if(_points[i].x() < _firstPoint.x() || (_points[i].x() == _firstPoint.x() && _points[i].y() > _firstPoint.y()))
       {
           _firstPoint = _points[i];
       }
   }

   //oznacava da li se pronasao citav konv. omotac
   bool complete;
   int exp = 1;
   while(true){
      complete = false;
      unsigned m = pow(2, pow(2,exp));
      if(m >= _points.size())
          m = _points.size();

      //kolika ce velicina vektora da bude?
      int numOfParts = _points.size()/m;
      int rest = _points.size() - numOfParts*m;
      if(rest > 0)
          numOfParts += 1;

      //deljenje skupa tacaka u delove od po najvise m elemenata
      _partsOfPoints.resize(numOfParts);
      for(int i=0; i < numOfParts; i++){
          for(unsigned j=0; j < m; j++){
              unsigned t = i*m + j;
              if( t >= _points.size())
                  break;
              _partsOfPoints[i].push_back(_points[i*m + j]);
          }
      }

      //racunanje graham scan
       for(int i=0; i < numOfParts; i++){
           if(_partsOfPoints[i].size() < 3 ){
               _hullsOfParts.push_back(_partsOfPoints[i]);
           }
           else{
               GrahamScan* g = new GrahamScan(_pRenderer, 0, "", 0, _partsOfPoints[i]);
               g->runAlgorithm();
               g->popFromHullLastPoint();
               _hullsOfParts.push_back(g->convexHull());
           }
           AlgorithmBase_updateCanvasAndBlock();
       }

      _tangents.resize(numOfParts);
      _convexHull.push_back(_firstPoint);
      _beforeP.setX(0);
      _beforeP.setY(-1000000);
      _P.setX(_firstPoint.x());
      _P.setY(_firstPoint.y());
      unsigned temp = 0;
      do{
          QPoint bestTangent(0,0);
          for(int i=0; i < numOfParts;i++){
              int tangent = findTangent(_hullsOfParts[i]);
              _tangents[i] =  _hullsOfParts[i][tangent];
              //ako smo prvi put u petlji, moramo da postavimo sta je najbolja do sad tangenta
              //a sledecih puta proveravamo da li je neka bolja
              if((i == 0) || compareTangents(_tangents[i], bestTangent) )
                  bestTangent = _tangents[i];
          }
          _convexHull.push_back(bestTangent);
          _beforeP = _P;
          _P = bestTangent;
          _tangentsFound = true;
          //zbog crtanja svih tangenti iz tacke
          AlgorithmBase_updateCanvasAndBlock();
          _drawConvexHull = true;

          temp++;
          //ovaj drugi uslov nam je bitan zbog kolinearnih tacaka,
          //slucaj kada imamo samo kolinearne tacke jer nece prepoznati da treba da stavi i prvu tacku na pocetak
          if((_P.x() == _firstPoint.x() && _P.y() == _firstPoint.y()) || (temp+1) == _points.size()){
              if((temp+1) == _points.size() && !(_P.x() == _firstPoint.x() && _P.y() == _firstPoint.y())){
                  _convexHull.push_back(_firstPoint);
                  AlgorithmBase_updateCanvasAndBlock();
              }
              _tangentsFound = false;
              AlgorithmBase_updateCanvasAndBlock();
              complete = true;
              break;
          }
          _tangentsFound = false;
          _drawConvexHull = false;
      }
      while(temp < m);

      if(complete){
           emit animationFinished();
           break;
      }

      exp++;
      _partsOfPoints.clear();
      _hullsOfParts.clear();
      _tangents.clear();
      _convexHull.clear();
      AlgorithmBase_updateCanvasAndBlock();
   }
}

void ChansAlgorithm::drawAlgorithm(QPainter &painter) const
{
    QPen p = painter.pen();
    p.setWidth(6);
    p.setColor(Qt::blue);
    painter.setPen(p);

    //Iscrtavanje svih tacaka
    for(QPoint p: _points)
       painter.drawPoint(p);

    //prvu tacka se boji crvenom bojom
    p.setColor(Qt::red);
    p.setWidth(12);
    painter.setPen(p);
    painter.drawPoint(_firstPoint);

    //crtanje svih delova koji su konveksne figure
    p.setWidth(6);
    for(unsigned i=1; i <= _hullsOfParts.size(); i++){
        p.setColor(QColor::fromRgb(i*116 % 256,i*232 % 256, i*84 % 256));
        painter.setPen(p);
        for(QPoint p: _partsOfPoints[i-1])
            painter.drawPoint(p);
        drawGraham(_hullsOfParts[i-1], painter, i*116 % 256, i*232 % 256, i*84 % 256);
    }

    //iscrtavamo tangente
    if(_tangentsFound){
        p.setColor(Qt::black);
        p.setWidth(2);
        painter.setPen(p);
        for(unsigned i=0; i < _hullsOfParts.size(); i++){
            painter.drawLine(_beforeP, _tangents[i]);
        }
    }
    p.setColor(Qt::red);
    p.setWidth(3);
    painter.setPen(p);
    int j = 0;
    if(_convexHull.size() > 2){
        for(unsigned i=0; i < _convexHull.size()-2; i++){
            painter.drawLine(_convexHull[i], _convexHull[i+1]);
            j++;
        }
        //ako je sledeca tacka odabrana, nacrtaj je
        if(_drawConvexHull){
            painter.drawLine(_convexHull[j],_convexHull[j+1]);
        }
    }
}

void ChansAlgorithm::runNaiveAlgorithm()
{
    ConvexHull::runNaiveAlgorithm();
}

unsigned ChansAlgorithm::returnPrevious(unsigned i, int size)
{
    if(i == 0)
        return size - 1;
    else
        return i - 1;
}

//proverava da li je u tacki p1 bolje tangenta nego u p2
bool ChansAlgorithm::compareTangents(const QPoint& p1, const QPoint& p2)
{
    //zbog duplikata tacaka: ako su ove dve tacke iste, vracam true
    if(p1.x() == p2.x() && p1.y() == p2.y())
        return true;

    QLineF firstLine(_beforeP,_P);
    QLineF secondLineFirstPoint(_P, p1);
    QLineF secondLineSecondPoint(_P, p2);
    //angleTo vraca broj stepeni koje treba prvoj da bi postala druga linija, suprotno od smera kazaljke
    qreal firstAngle = firstLine.angleTo(secondLineFirstPoint);
    qreal secondAngle = firstLine.angleTo(secondLineSecondPoint);

    //ovo ovde radim jer kad su kolinearne tacke, on za ugao beforeP P p1 uzme nula, iako mi odgovara da uzme 360
    // da bi sledecu kolinearnu tacku preferirao
    if(firstAngle == 0)
        firstAngle = 360;
    if(secondAngle == 0)
        secondAngle = 360;

    if(p1 == _P || p1 == _beforeP)
        firstAngle = 0;
    if(p2 == _P || p2 == _beforeP)
        secondAngle = 0;

    if(firstAngle > secondAngle)
        return true;
    else if(firstAngle == secondAngle){
        double dist1 = utils::distance(_P,p1);
        double dist2 = utils::distance(_P,p2);
        if(dist1 < dist2 && dist1 != 0)
            return true;
    }
    return false;
}
//proverava da li se u i-toj tacki konv. omotaca points nalazi tangenta u kojoj je ugao najveci
bool ChansAlgorithm::isMaxPoint(unsigned i, const std::vector<QPoint>& points)
{
    unsigned before = returnPrevious(i, points.size());
    unsigned after = (i + 1) % points.size();

    bool beforeI = compareTangents(points[i], points[before]);
    bool afterI = compareTangents(points[i], points[after]);
    if(beforeI && afterI)
        return true;
    else
        return false;

}
//proverava da li se u ovom intervalu nalazi tacka u kojoj tangenta ima maks. ugao
bool ChansAlgorithm::containsMaxPoint(unsigned start, unsigned end, const std::vector<QPoint>& points)
{
    unsigned beforeStart = returnPrevious(start, points.size());
    unsigned afterStart = (start + 1) % points.size();
    unsigned beforeEnd = returnPrevious(end, points.size());
    unsigned afterEnd = (end + 1) % points.size();

    bool tanStartBefore = compareTangents(points[start], points[beforeStart]);
    bool tanStartAfter = compareTangents(points[start], points[afterStart]);
    bool tanEndBefore = compareTangents(points[end], points[beforeEnd]);
    bool tanEndAfter = compareTangents(points[end], points[afterEnd]);
    bool tanStartEnd = compareTangents(points[start], points[end]);

    //imamo 5 slucajeva u kojima ova funkcija vraca true, sve oznake se odnose na uglove
    // xb < x < xa  i yb > y > ya
    if(tanStartBefore && !tanStartAfter && !tanEndBefore && tanEndAfter)
        return true;
    // xb > x > xa  i yb > y > ya i x<y
    if(!tanStartBefore && tanStartAfter && !tanEndBefore && tanEndAfter && !tanStartEnd)
        return true;
    // xb < x < xa  i yb < y < ya i x>y
    if(tanStartBefore && !tanStartAfter && tanEndBefore && !tanEndAfter && tanStartEnd)
        return true;
    //ako je x minimum xb > x < xa, onda treba da vazi i yb > y > ya
    if(!tanStartBefore && !tanStartAfter && !tanEndBefore && tanEndAfter)
        return true;
    //ako je y minimum yb > y < ya, onda treba da vazi i xb < x < xa
    if(!tanEndBefore && !tanEndAfter && tanStartBefore && !tanStartAfter)
        return true;
    return false;
}

int ChansAlgorithm::findTangent(const std::vector<QPoint>& points)
{
    if(points.size() == 1)
        return 0;
    if(points.size() == 2){
        if(compareTangents(points[0], points[1]))
            return 0;
        else
            return 1;
    }

    int start = 0;
    int end = points.size()-1;

    if(isMaxPoint(start, points))
        return start;
    if(isMaxPoint(end, points))
        return end;
    while (start <= end) {
        if(start == end)
            return start;

        unsigned middle = (start + end)/2;
        unsigned before = returnPrevious(middle, points.size());
        unsigned after = (middle + 1) % points.size();

        //ako je to bas tacka iz koje trazimo tangentu, onda max point mora da bude ili u prosloj ili u sled. tacki
        if(points[middle].x() == _P.x() && points[middle].y() == _P.y()){
            if (compareTangents(points[before], points[after]))
                return before;
            else
                return after;
        }

        if(isMaxPoint(middle, points))
                return middle;
        if(containsMaxPoint(start, middle, points))
            end = middle;
        else
            start = middle;
    }
    //zbog warninga
    return start;
}

void ChansAlgorithm::drawGraham(const std::vector<QPoint>& points, QPainter &painter, int r, int g, int b) const
{
    QPen p = painter.pen();
    p.setWidth(1);
    p.setColor(QColor(r, g, b));
    painter.setPen(p);

    QPainterPath path;
    path.moveTo(points[0]);

    int sizeOfHull = points.size()-1;
    for(int i = 0; i < sizeOfHull; i++)
    {
        path.lineTo(points[i+1]);
    }
    path.lineTo(points[0]);
    painter.drawLine(points[sizeOfHull],points[0]);
    painter.fillPath(path, QBrush(QColor(r,g,b,140)));
}

std::vector<QPoint> ChansAlgorithm::convexHull() const
{
    return _convexHull;

}

std::vector<QPoint> ChansAlgorithm::convexHullTest() const
{
    return ConvexHull::convexHullTest();
}


