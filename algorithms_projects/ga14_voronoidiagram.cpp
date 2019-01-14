#include <QPainter>
#include <QtDebug>
#include <QtMath>
#include <cstdlib>
#include "ga14_voronoidiagram.h"
#include <algorithm>
#include <QCoreApplication>


#include "utils.h"

qreal lineY=0;

bool pointComparator (QPointF i,QPointF j) { return (i.y()<j.y()); }

//RED DOGADJAJA
int EventQueue::size(){
    return internalEvents.size();
}

void EventQueue::addEvent(VoronoiEventPoint* point){
    if(point->getType()==VoronoiEventType::Site)
        internalEvents.push_back(point);
    else{
        int indeks=0;
        if(internalEvents.size()>0){
            int i=0;
            double newY=point->getPoint().y();
            while(i<internalEvents.size() && newY>= internalEvents[i]->getPoint().y()){
                i++;
                indeks++;
            }
        }
        internalEvents.insert(internalEvents.begin()+indeks,point);
    }
}

void EventQueue::removeCircleByCode(QString code){
        int k=-1;
        for (int i = 0; i < internalEvents.size(); ++i) {
            if(internalEvents[i]->getCircleText()==code){
                k=i;
                break;
            }
        }
        if(k>=0)
            internalEvents.erase(internalEvents.begin()+k);
    //EventQueue::eventPoints.erase(std::remove(EventQueue::eventPoints.begin(),EventQueue::eventPoints.end(),point),EventQueue::eventPoints.end());
}

void EventQueue::removeEvent(VoronoiEventPoint* point){
    if(point!=nullptr && point->getType()==VoronoiEventType::Circle){
        int k=-1;
        for (int i = 0; i < internalEvents.size(); ++i) {
            if(internalEvents[i]->getCircleText()==point->getCircleText()){
                k=i;
                break;
            }
        }
        if(k>=0)
            internalEvents.erase(internalEvents.begin()+k);
    }
    //EventQueue::eventPoints.erase(std::remove(EventQueue::eventPoints.begin(),EventQueue::eventPoints.end(),point),EventQueue::eventPoints.end());
}

VoronoiEventPoint* EventQueue::getEventAt(){
    /*if(i>=0 && i<eventPoints.size()){
        return eventPoints[i];
    }*/
    VoronoiEventPoint* retPoint =  internalEvents.size()>0?internalEvents[0]:nullptr;

    if(retPoint!=nullptr)
        internalEvents.erase(internalEvents.begin());

    return retPoint;
}

VoronoiEventPoint* EventQueue::readEventAt(int i){
    if(i < internalEvents.size()){
        return internalEvents[i];
    }
    return nullptr;
}

bool EventQueue::isEmpty(){
    return internalEvents.size()==0;
}
//RED DOGADJAJA KRAJ



bool parabolasClosing(VoronoiEventPoint* A, VoronoiEventPoint* B,VoronoiEventPoint* C){
    QPointF pA = A->getPoint();
    QPointF pB = B->getPoint();
    QPointF pC = C->getPoint();

    double k1 = -(1.0/((1.0*(pB.y()-pA.y()))/(pB.x()-pA.x())));
    double k2 = -(1.0/((1.0*(pC.y()-pB.y()))/(pC.x()-pB.x())));

    QPointF M1((pA.x()+pB.x())/2.0,(pA.y()+pB.y())/2.0);
    QPointF M2((pB.x()+pC.x())/2.0,(pB.y()+pC.y())/2.0);

    double x = ((M2.y()+k1*M1.x()-M1.y()-k2*M2.x()));
    double y = k1*x+M1.y()-k1*M1.x();

    //proveravamo da li se za dati poredak zatvara
    double ax=pA.x(),ay=pA.y(),bx=pB.x(),by=pB.y(),cx=pC.x(),cy=pC.y();
    double det = (bx - ax)*(cy - ay) - (cx - ax)*(by - ay);

    return det>0;

}

QPointF* cicrcleCenterFromPoints(QPointF A, QPointF B, QPointF C,float * pR){
    double offset = qPow(B.x(),2) + qPow(B.y(),2);
    double bc =   ( qPow(A.x(),2) + qPow(A.y(),2) - offset )/2.0;
    double cd =   (offset - qPow(C.x(), 2) - qPow(C.y(), 2))/2.0;
    double det =  (A.x() - B.x()) * (B.y() - C.y()) - (B.x() - C.x())* (A.y() - B.y());

    double idet = 1/det;

    double centerx =  (bc * (B.y() - C.y()) - cd * (A.y() - B.y())) * idet;
    double centery =  (cd * (A.x() - B.x()) - bc * (B.x() - C.x())) * idet;
    double radius = qSqrt( qPow(B.x() - centerx,2) + qPow(B.y()-centery,2));
    *pR=radius;

    return new QPointF(centerx,centery);
}



//VORONOI EVENT POINT
VoronoiEventPoint::VoronoiEventPoint(QPointF iPoint, VoronoiEventType iType,int iIndex)
    :type(VoronoiEventType::Site),point(iPoint),index(iIndex)
{}

VoronoiEventPoint::VoronoiEventPoint(QPointF iPoint, QPointF* iupperPoint,VoronoiEventType iType,QString circInfo,Intersection* iLeftInt,Intersection* irightInt)
    :type(VoronoiEventType::Circle),point(iPoint),circleEventUpperPoint(iupperPoint),circleEventText(circInfo),leftInt(iLeftInt),rightInt(irightInt)
{
}

VoronoiEventPoint::VoronoiEventPoint(VoronoiEventPoint *duplPoint)
    :type(duplPoint->getType()),point(duplPoint->getPoint()),index(duplPoint->index){

}

QPointF* VoronoiEventPoint::getCircleCenter(){
    return circleEventUpperPoint;
}

QPointF VoronoiEventPoint::getPoint(){
    return point;
}
VoronoiEventType VoronoiEventPoint::getType(){
    return type;
}
int VoronoiEventPoint::getIndex(){
    return index;
}

void VoronoiEventPoint::setCircleText(QString str){
    circleEventText=str;
}

QString VoronoiEventPoint::getCircleText(){
    return circleEventText;
}

Intersection* VoronoiEventPoint::leftIntersection(){
    return leftInt;
}

Intersection* VoronoiEventPoint::rightIntersection(){
    return rightInt;
}

// VORONOI END POINT END

Intersection::Intersection(VoronoiEventPoint *iA, VoronoiEventPoint *iB)
    :A(iA),B(iB){
    startPoint=getIntersectionPoint();
}


QPointF* Intersection::getIntersectionPoint(){
    QPointF A1 = A->getPoint();
    QPointF B1 = B->getPoint();

    double x=0,y=0;

    //specijalan slucaj ako su isti y
    if(A1.y()==B1.y()&& A1.y()==lineY){
        return new QPointF((A1.x()+B1.x())/2.0,-100);
    }else if(A1.y()==lineY){
        double dX=B1.x();
        double dY=B1.y();
        x=A1.x();
        y=(1.0/(2*(dY-lineY)))*((x-dX)*(x-dX))+(1.0/2)*(dY+lineY);

    }else if(B1.y()==lineY){
        double dX=A1.x();
        double dY=A1.y();
        x=B1.x();
        y=(1.0/(2*(dY-lineY)))*((x-dX)*(x-dX))+(1.0/2)*(dY+lineY);

    }else{
        double a1=A1.x(),b1=A1.y(),a2=B1.x(),b2=B1.y();
        double k=lineY;
        double q1=(1.0/(2.0*(b1-k))),k1=((1.0/2.0)*(b1+k));
        double q2=(1.0/(2.0*(b2-k))),k2=((1.0/2.0)*(b2+k));

        if(b1==b2){
            x=(a1+a2)/2.0;
        }else{

            double a = q1-q2;
            double b = 2.0*(q2*a2-q1*a1);
            double c = q1*a1*a1+k1-q2*a2*a2-k2;
            double D=b*b-4*a*c;
            double x1 = ((-b)-sqrt(D))/(2.0*a);
            double x2 = ((-b)+sqrt(D))/(2.0*a);
            float minX = x1<x2?x1:x2;
            float maxX = x1>x2?x1:x2;
            x=0;

            if(qIsNaN(minX) && qIsNaN(maxX)){
                if(A->getIndex()>B->getIndex())
                    x= A->getPoint().x();
                else
                    x= B->getPoint().x();
            }else{
                if(A->getIndex()>B->getIndex())
                    x=maxX;
                else
                    x=minX;
            }
        }
        y=q1*(x-a1)*(x-a1)+k1;
    }
    return new QPointF(x,y);
}

bool cmpInt::operator()(Intersection* a, Intersection* b){
        if(a->A->getIndex()==b->B->getIndex() && a->B->getIndex()==b->A->getIndex())
            return a->A->getIndex()<b->A->getIndex();
        else
            return  a->getIntersectionPoint()->x() <= b->getIntersectionPoint()->x();
}

bool cmpPoints::operator()(QPointF a, QPointF b){
        if(round(a.x())!=round(b.x()))
              return round(a.x())<round(b.x());
        else
            return round(a.y())<round(b.y());
}

QString Intersection::getIndexes(){
    return QString::number(A->getIndex())+"-"+QString::number(B->getIndex());
}

VoronoiDiagram::VoronoiDiagram(QWidget *pRenderer, int delayMs, std::string filename, int inputSize)
    : AlgorithmBase (pRenderer, delayMs)
{

    if (filename == "")
    {
        points = generateRandomPoints(inputSize);
    }
    else
    {
        points = readPointsFromFile(filename);
    }
    std::sort(points.begin(),points.end(),pointComparator);


    for (int i = 0; i < points.size(); ++i) {
        QPointF cPt(points[i].x(),points[i].y());
        VoronoiEventPoint* vPoint =new VoronoiEventPoint(cPt,VoronoiEventType::Site,i);
        eventPoints.push_back(vPoint);
    }
    currentEventSet=false;


}



void VoronoiDiagram::processSiteEvent(std::vector<VoronoiEventPoint*> eventP,EventQueue * allEvents){
    //ako je front prazan, postaviti teme
    if(frontPreseci.size()==0){
        if(onlyPoint==nullptr){
            onlyPoint=currentEvent;
        }else{
            QString keyOld = QString::number(onlyPoint->getIndex());
            QString keyNew = QString::number(currentEvent->getIndex());

            QString keyLeft = keyOld+"-"+keyNew;
            QString keyRight = keyNew+"-"+keyOld;

            Intersection* newIntLeft = new Intersection(onlyPoint,currentEvent);
            Intersection* newIntRight = new Intersection(currentEvent,onlyPoint);

            if(onlyPoint->getPoint().y()==currentEvent->getPoint().y()){
                if(onlyPoint->getPoint().x()<currentEvent->getPoint().x() )
                    frontPreseci.insert(frontPreseci.begin(),std::pair<Intersection*,Intersection*>(newIntLeft,newIntLeft));
                else
                    frontPreseci.insert(frontPreseci.begin(),std::pair<Intersection*,Intersection*>(newIntRight,newIntRight));
            }else{
                frontPreseci.insert(frontPreseci.begin(),std::pair<Intersection*,Intersection*>(newIntLeft,newIntLeft));
                frontPreseci.insert(frontPreseci.begin(),std::pair<Intersection*,Intersection*>(newIntRight,newIntRight));
            }
        }

    }else{
        //ako nije prazan naci luk direktno iznad (naci presek izmedju kojih je ovo binarnom pretragom)
        int xP= currentEvent->getPoint().x();
        VoronoiEventPoint* upperIndex=nullptr;
        Intersection* previousTriple=nullptr;
        Intersection* upperTriple=nullptr;


        for (auto const& x : frontPreseci)
        {
            int currX=x.first->getIntersectionPoint()->x();

            if(xP <= currX){
                upperTriple=x.first;
                upperIndex=x.first->A;
                break;
            }
            previousTriple=x.first;
        }
        Intersection* newIntLeft=nullptr;
        Intersection* newIntRight=nullptr;
        //ubaciti dva nova preseka (sa srednjim elementom nadjenih preseka)
        if(upperIndex!= nullptr){
            newIntLeft = new Intersection(upperIndex,currentEvent);
            newIntRight = new Intersection(currentEvent,upperIndex);
            frontPreseci.insert(frontPreseci.begin(),std::pair<Intersection*,Intersection*>(newIntLeft,newIntLeft));
            frontPreseci.insert(frontPreseci.begin(),std::pair<Intersection*,Intersection*>(newIntRight,newIntRight));

            //uklanjanje prethodnog dogadjaja kruga


            //azuriranje tacaka kruga u odnosu na nov luk

            VoronoiEventPoint* Ai=upperTriple->A;
            VoronoiEventPoint* Bi=upperTriple->B;

            float R;
            float * pR=&R;
            if(parabolasClosing(currentEvent,Ai,Bi)){ //Desna trojka
                QPointF * newCircleEvent = cicrcleCenterFromPoints(currentEvent->getPoint(),Ai->getPoint(),Bi->getPoint(),pR);
                QString circString = "("+QString::number(currentEvent->getIndex())+ ","+QString::number(Ai->getIndex())+","+QString::number(Bi->getIndex())+")";


                    QPointF * centerCirclePoint= new QPointF((*newCircleEvent).x(),(*newCircleEvent).y());
                    QPointF * bottomCirclePoint= new QPointF((*newCircleEvent).x(),(*newCircleEvent).y()+R);
                    VoronoiEventPoint* circleEvent = new VoronoiEventPoint(*bottomCirclePoint,centerCirclePoint,VoronoiEventType::Circle,circString,newIntRight,upperTriple);
                    allEvents->addEvent(circleEvent);
                    upperTriple->Acirc=circleEvent;

            }
        }else{
            //ubacujemo skroz desno
            newIntLeft = new Intersection(previousTriple->B,currentEvent);
            newIntRight = new Intersection(currentEvent,previousTriple->B);
            frontPreseci.insert(frontPreseci.begin(),std::pair<Intersection*,Intersection*>(newIntLeft,newIntLeft));
            frontPreseci.insert(frontPreseci.begin(),std::pair<Intersection*,Intersection*>(newIntRight,newIntRight));

        }

        //proveriti trojku tacaka kojoj je nova tacka levi sused i kojoj je desni sused (priblizavanje)
        if(previousTriple!=nullptr){
            VoronoiEventPoint* Ai=previousTriple->A;
            VoronoiEventPoint* Bi=previousTriple->B;

            float R;
            float * pR=&R;
            if(parabolasClosing(Ai,Bi,currentEvent)){ //Leva trojka
                QPointF * newCircleEvent = cicrcleCenterFromPoints(Ai->getPoint(),Bi->getPoint(),currentEvent->getPoint(),pR);
                QString circString = "("+QString::number(Ai->getIndex())+","+QString::number(Bi->getIndex())+","+QString::number(currentEvent->getIndex())+")";


                QPointF * centerCirclePoint= new QPointF((*newCircleEvent).x(),(*newCircleEvent).y());
                QPointF * bottomCirclePoint= new QPointF((*newCircleEvent).x(),(*newCircleEvent).y()+R);
                VoronoiEventPoint* circleEvent = new VoronoiEventPoint(*bottomCirclePoint,centerCirclePoint,VoronoiEventType::Circle,circString,previousTriple,newIntLeft);
                allEvents->addEvent(circleEvent);


            }
        }
        if(previousTriple!=nullptr && upperTriple!=nullptr){
            allEvents->removeCircleByCode("("+QString::number(previousTriple->A->getIndex())+","+QString::number(previousTriple->B->getIndex())+","+QString::number(upperTriple->B->getIndex())+")");
        }
    }


}

bool sameIndexes(int a,int b,int c, int a1, int b1, int c1){
    if((a==a1||a==b1||a==c1)&&
       (b==a1||b==b1||b==c1)&&
       (c==a1||c==b1||c==c1))
        return true;
    return false;
}

void VoronoiDiagram::processCircleEvent(std::vector<VoronoiEventPoint*> eventP,EventQueue * allEvents){
    tackeVoronoiDijagrama.push_back(currentEvent->getCircleCenter());

    //preseci postaju grane voronoj dijagrama
    currentEvent->leftIntersection()->endPoint=currentEvent->getCircleCenter();
    currentEvent->rightIntersection()->endPoint=currentEvent->getCircleCenter();
    linijeVoronoiDijagrama.push_back(currentEvent->leftIntersection());
    linijeVoronoiDijagrama.push_back(currentEvent->rightIntersection());

    //brisemo preseke koji su se susreli

    for(std::map<Intersection*, Intersection*>::iterator it = frontPreseci.begin(); it != frontPreseci.end();)
    {
        if((it->first->A->getIndex()==currentEvent->leftIntersection()->A->getIndex() &&it->first->B->getIndex()==currentEvent->leftIntersection()->B->getIndex()) ||
           (it->first->A->getIndex()==currentEvent->rightIntersection()->A->getIndex() &&it->first->B->getIndex()==currentEvent->rightIntersection()->B->getIndex()))
        {
            it = frontPreseci.erase(it);
        }
        else
        {
            it++;
        }

    }


    Intersection* newIntersection= new Intersection(currentEvent->leftIntersection()->A,currentEvent->rightIntersection()->B);
    frontPreseci.insert(frontPreseci.begin(),std::pair<Intersection*,Intersection*>(newIntersection,newIntersection));
    Intersection* previousInt=nullptr,* nextInt=nullptr;
    bool nextOne=false,previousDone=false;
    for (auto const& x : frontPreseci)
    {
        if(nextOne){
            nextInt=x.first;
            break;
        }else if(x.first->getIndexes()==newIntersection->getIndexes()){
            nextOne=true;
            previousDone=true;
        }

        if(!previousDone){
            previousInt=x.first;
        }
    }

    if(previousInt!=nullptr){

        allEvents->removeCircleByCode("("+QString::number(previousInt->A->getIndex())+","+QString::number(currentEvent->leftIntersection()->A->getIndex())+","+QString::number(currentEvent->leftIntersection()->B->getIndex())+")");

        VoronoiEventPoint* Ai=previousInt->A;
        VoronoiEventPoint* Bi=previousInt->B;

        float R;
        float * pR=&R;
        if(!sameIndexes(currentEvent->leftIntersection()->A->getIndex(),currentEvent->leftIntersection()->B->getIndex(),currentEvent->rightIntersection()->B->getIndex(),
                        Ai->getIndex(),Bi->getIndex(),newIntersection->B->getIndex())&& parabolasClosing(Ai,Bi,newIntersection->B)){ //Leva trojka
            QPointF * newCircleEvent = cicrcleCenterFromPoints(Ai->getPoint(),Bi->getPoint(),newIntersection->B->getPoint(),pR);


            QString circString = "("+QString::number(Ai->getIndex())+","+QString::number(Bi->getIndex())+","+QString::number(newIntersection->B->getIndex())+")";



                QPointF * centerCirclePoint= new QPointF((*newCircleEvent).x(),(*newCircleEvent).y());
                QPointF * bottomCirclePoint= new QPointF((*newCircleEvent).x(),(*newCircleEvent).y()+R);
                VoronoiEventPoint* circleEvent = new VoronoiEventPoint(*bottomCirclePoint,centerCirclePoint,VoronoiEventType::Circle,circString,previousInt,newIntersection);
                allEvents->addEvent(circleEvent);
                previousInt->Bcirc=circleEvent;
                newIntersection->Acirc=circleEvent;
        }
    }

    if(nextInt!=nullptr){
        allEvents->removeCircleByCode("("+QString::number(currentEvent->rightIntersection()->A->getIndex())+","+QString::number(currentEvent->rightIntersection()->B->getIndex())+","+QString::number(nextInt->B->getIndex())+")");

        VoronoiEventPoint* Ai=nextInt->A;
        VoronoiEventPoint* Bi=nextInt->B;

        float R;
        float * pR=&R;
        if(!sameIndexes(currentEvent->leftIntersection()->A->getIndex(),currentEvent->leftIntersection()->B->getIndex(),currentEvent->rightIntersection()->B->getIndex(),
                        newIntersection->A->getIndex(),Ai->getIndex(),Bi->getIndex())&&parabolasClosing(newIntersection->A,Ai,Bi)){ //Desna trojka
            QPointF * newCircleEvent = cicrcleCenterFromPoints(newIntersection->A->getPoint(),Ai->getPoint(),Bi->getPoint(),pR);
            QString circString = "("+QString::number(newIntersection->A->getIndex())+ ","+QString::number(Ai->getIndex())+","+QString::number(Bi->getIndex())+")";

            QPointF * centerCirclePoint= new QPointF((*newCircleEvent).x(),(*newCircleEvent).y());
            QPointF * bottomCirclePoint= new QPointF((*newCircleEvent).x(),(*newCircleEvent).y()+R);
            VoronoiEventPoint* circleEvent = new VoronoiEventPoint(*bottomCirclePoint,centerCirclePoint,VoronoiEventType::Circle,circString,newIntersection,nextInt);
            allEvents->addEvent(circleEvent);
            nextInt->Acirc=circleEvent;
            newIntersection->Bcirc=circleEvent;
            //upperTriple->Acirc=circleEvent;
         }
    }

}

QPointF *
presekDuziIPoluprave (QPointF * a, QPointF * b, QPointF * c, QPointF * d)
{
    double x,y;
    double k1,n1,k2,n2,p;
    if(b->x()-a->x()==0){
        x=b->x();
        k2 = (d->y() - c->y()) / (d->x() - c->x());
        n2 = -k2*c->x() + c->y();
        y=k2*x+n2;
        double minY = a->y()<=b->y()?a->y():b->y();
        double maxY = a->y()>=b->y()?a->y():b->y();

        if(y>=minY && y<=maxY)
            p=0.5;
        else
            p=-0.5;
    }else if(d->x()-c->x()==0){
        x=d->x();
        k1 = (b->y() - a->y()) / (b->x() - a->x());
        n1 = -k1*a->x() + a->y();
        y=k1*x+n1;
        p = (x - a->x()) / (b->x() - a->x());
    }else{
        k1 = (b->y() - a->y()) / (b->x() - a->x());
        n1 = -k1*a->x() + a->y();

        k2 = (d->y() - c->y()) / (d->x() - c->x());
        n2 = -k2*c->x() + c->y();

        x = (n2 - n1) / (k1 - k2);
        y = k1 * x + n1;
        p = (x - a->x()) / (b->x() - a->x());
    }

  if (p >= 0 && p <= 1 && !std::isnan(x)&&!std::isnan(y))
    {
      return new QPointF (x, y);
    }
  else
    return nullptr;
}

bool orijentacijaTacaka(QPointF* pA, QPointF* pB, QPointF * pC){
    //proveravamo da li se za dati poredak zatvara
    double ax=pA->x(),ay=pA->y(),bx=pB->x(),by=pB->y(),cx=pC->x(),cy=pC->y();
    double det = (bx - ax)*(cy - ay) - (cx - ax)*(by - ay);
    return det>0;
}



std::map<int,std::vector<QPointF*>*>* VoronoiDiagram::naiveMap(bool testing,double maxX,double maxY){
     std::map<int, std::vector<QPointF*>*>* mnogouglovi = new std::map<int,std::vector<QPointF*>*>();
      int brTemena=eventPoints.size();
      std::vector<QPointF*> *celijaTrTemena= new std::vector<QPointF*>();
      celijaTrTemena->push_back(new QPointF(0,0));
      if(testing){
          celijaTrTemena->push_back(new QPointF(maxX+200 ,0));
          celijaTrTemena->push_back(new QPointF(maxX+200,maxY+200));
          celijaTrTemena->push_back(new QPointF(0,maxY+200));
      }else{
          celijaTrTemena->push_back(new QPointF(_pRenderer->width() ,0));
          celijaTrTemena->push_back(new QPointF(_pRenderer->width(),_pRenderer->height()));
          celijaTrTemena->push_back(new QPointF(0,_pRenderer->height()));
      }

      //za svako teme
      QPointF  S,K;
      for(int i=0;i<brTemena;i++){
            S=eventPoints[i]->getPoint();
            //cout<<"S:"<<i<<endl;

            celijaTrTemena= new std::vector<QPointF*>();
            celijaTrTemena->push_back(new QPointF(0,0));
            if(testing){
                celijaTrTemena->push_back(new QPointF(maxX+200 ,0));
                celijaTrTemena->push_back(new QPointF(maxX+200,maxY+200));
                celijaTrTemena->push_back(new QPointF(0,maxY+200));
            }else{
                celijaTrTemena->push_back(new QPointF(_pRenderer->width() ,0));
                celijaTrTemena->push_back(new QPointF(_pRenderer->width(),_pRenderer->height()));
                celijaTrTemena->push_back(new QPointF(0,_pRenderer->height()));
            }

            //proveravamo svako drugo teme
            for(int j=0;j<brTemena;j++){
                if(i!=j){
                    std::vector<QPointF *>* prviPoligon = new std::vector<QPointF*>();
                    std::vector<QPointF*>*drugiPoligon = new std::vector<QPointF*>();
                    K = eventPoints[j]->getPoint();
                    QPointF* C;
                    QPointF* D;
                    if(K.y()!=S.y()){
                        double mx=(S.x()+K.x())/2;
                        double my=(S.y()+K.y())/2;

                        double k = (K.y()-S.y())/(K.x()-S.x());
                        double iK=(-1.0/(k));

                        double b=my-iK*mx;

                        //tacke simetrale
                        C = new QPointF(0,b);
                        D = new QPointF((-b)/iK,0);
                    }else{
                        double midX = (S.x()+K.x())/2;
                        C = new QPointF(midX,S.y());
                        D = new QPointF(midX,S.y()-10);
                    }

                    bool firstSet=false,secondSet=false;
                    QPointF * pr1=nullptr, *pr2=nullptr;
                    int indeksPreseka1;
                    int indeksPreseka2;

                    //trazimo preseke i smanjujemo omotac
                    for(int k=0;k<celijaTrTemena->size();k++){
                        QPointF*A = (*celijaTrTemena)[k];
                        QPointF*B=nullptr;
                        if(k==celijaTrTemena->size()-1){
                            B=(*celijaTrTemena)[0];
                        }else{
                            B=(*celijaTrTemena)[k+1];
                        }

                        QPointF* trPresek=presekDuziIPoluprave(A,B,C,D);
                        if(!firstSet || secondSet){
                            prviPoligon->push_back(A);
                            if(trPresek!=nullptr){
                                pr1=trPresek;
                                firstSet=true;
                                prviPoligon->push_back(trPresek);
                                drugiPoligon->push_back(trPresek);
                            }
                        }else{
                            drugiPoligon->push_back(A);
                            if(trPresek!=nullptr){
                                pr2=trPresek;
                                secondSet=true;
                                prviPoligon->push_back(trPresek);
                                drugiPoligon->push_back(trPresek);
                            }
                        }

                    }
                    if(pr1!=nullptr && pr2!=nullptr){
                        if(orijentacijaTacaka(pr1,pr2,new QPointF(S.x(),S.y()))){
                            celijaTrTemena=prviPoligon;

                        }else{
                            celijaTrTemena=drugiPoligon;
                        }
                    }
                    //AlgorithmBase_updateCanvasAndBlock();
                    currentPolygon=celijaTrTemena;
                    currentPolygonSet=true;
                }

            }
            std::vector<QPointF*> *finalnaCelija=new std::vector<QPointF*>();
            for (int l = 0; l < celijaTrTemena->size(); ++l) {
                finalnaCelija->push_back(new QPointF((*celijaTrTemena)[l]->x(),(*celijaTrTemena)[l]->y()));
            }

            mnogouglovi->insert(std::pair<int,std::vector<QPointF*>*>(i,finalnaCelija));


      }
     return mnogouglovi;
}

void VoronoiDiagram::runAlgorithm(){

    for (int i = 0; i < eventPoints.size(); ++i) { //stvaranje reda dogadjaja na osnovu sedista
        allEvents->addEvent(eventPoints[i]);
    }
    //runNaiveAlgorithm();
    //
    //naiveDebug=true;

    //
    //mapPointHull=naiveMap(false,0,0);
    //drawMap=true;

    int i=0;
    while(!allEvents->isEmpty()){ //i<eventPoints.size()){
        currentEvent=allEvents->getEventAt();
        lineY=currentEvent->getPoint().y();
        currentEventSet=true;
        if(currentEvent->getType()==VoronoiEventType::Site)
            processSiteEvent(eventPoints,allEvents);
        if(currentEvent->getType()==VoronoiEventType::Circle){
            processCircleEvent(eventPoints,allEvents);
        }

        AlgorithmBase_updateCanvasAndBlock();
        i++;
    }


    double maxX=0,maxY=0;
    for (int i = 0; i < points.size(); ++i) {
        if(points[i].x()>maxX)
            maxX=points[i].x();
        if(points[i].y()>maxY)
            maxY=points[i].y();
    }

        //izvlacenje jedinstvenih temena unutar ekrana
        for (int j = 0; j < tackeVoronoiDijagrama.size(); ++j) {
            if(tackeVoronoiDijagrama[j]->x()>0 && tackeVoronoiDijagrama[j]->x()<maxX && tackeVoronoiDijagrama[j]->y() >0 && tackeVoronoiDijagrama[j]->y()<maxY){
                unitForcen.push_back(*tackeVoronoiDijagrama[j]);
            }
        }
        //qDebug()<<unitForcen.size();

    emit animationFinished();

}



void VoronoiDiagram::drawAlgorithm(QPainter &painter) const
{
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPen p = painter.pen();

    //points
    p.setWidth(7);
    p.setCapStyle(Qt::RoundCap);
    p.setColor(Qt::red);
    painter.setPen(p);

    if(currentPolygonSet && naiveDebug){
        QPolygonF poly;
        for (int j = 0; j < currentPolygon->size(); ++j) {
                p.setWidth(5);
                p.setCapStyle(Qt::RoundCap);
                p.setColor(Qt::red);
                painter.setPen(p);
                poly<< *(*currentPolygon)[j];
        }
        painter.drawPolygon(poly);

    }

    for (int i = 0; i < eventPoints.size() ; ++i) {
        VoronoiEventPoint* currDrawPoint = eventPoints[i];
        QPointF pt = currDrawPoint->getPoint();
        p.setColor(Qt::red);
        painter.setPen(p);
        painter.drawPoint(pt);
        painter.drawText(pt.x()+10,pt.y()+15,QString::number(currDrawPoint->getIndex()));
    }

    for (int i = 0; i < allEvents->size(); ++i) {
        VoronoiEventPoint* currDrawPoint = allEvents->readEventAt(i);
        QPointF pt = currDrawPoint->getPoint();
        if(currDrawPoint->getType()==VoronoiEventType::Circle){
            p.setColor(Qt::blue);
            painter.setPen(p);
            painter.drawPoint(pt);
            painter.drawText(pt.x()+10,pt.y()+15,currDrawPoint->getCircleText());
        }

    }

    if(drawMap){
        for (auto const& x : (*mapPointHull))
        {
            p.setWidth(7);
            p.setCapStyle(Qt::RoundCap);
            p.setColor(Qt::magenta);
            painter.setPen(p);

            std::vector<QPointF*>* poligon= x.second;
            for (int j = 0; j < poligon->size(); ++j) {
                p.setWidth(4);
                p.setCapStyle(Qt::RoundCap);
                p.setColor(Qt::green);
                painter.setPen(p);
                if(j==poligon->size()-1){
                    painter.drawLine((*poligon)[j]->x(),(*poligon)[j]->y(),(*poligon)[0]->x(),(*poligon)[0]->y());

                }else{
                    painter.drawLine((*poligon)[j]->x(),(*poligon)[j]->y(),(*poligon)[j+1]->x(),(*poligon)[j+1]->y());
                }

            }
        }
    }

    //sweep line
    if(currentEventSet){
        p.setColor(Qt::green);
        p.setWidth(2);
        painter.setPen(p);
        painter.drawLine(0, currentEvent->getPoint().y(), _pRenderer->width(), currentEvent->getPoint().y());


        //parabole
        if(!allEvents->isEmpty()){
            p.setColor(Qt::gray);
            p.setWidth(1);
            painter.setPen(p);
            QPointF currentEventPoint = currentEvent->getPoint();

            for (int k = 0; k < points.size(); ++k) {
                QPointF currEvPoint=points[k];
                if(currentEventPoint.y()>currEvPoint.y()){
                    for (float i = 0; i < _pRenderer->width(); i+=10) {
                        int cPointY = currEvPoint.y();
                        float y1 = (1.0/(2*(cPointY-currentEventPoint.y())))*(i-currEvPoint.x())*(i-currEvPoint.x())+
                                (1.0/2)*(cPointY+currentEventPoint.y());

                        float y2 = (1.0/(2*(cPointY-currentEventPoint.y())))*((i+10)-currEvPoint.x())*((i+10)-currEvPoint.x())+
                                (1.0/2)*(cPointY+currentEventPoint.y());
                        if(y1>-20 && y2>-20){
                            painter.drawLine(i,y1,i+10,y2);
                        }
                    }
                }
            }
        }

            //Preseci
            if(frontPreseci.size()>0){
                for (auto const& x : frontPreseci)
                {
                    p.setWidth(7);
                    p.setCapStyle(Qt::RoundCap);
                    p.setColor(Qt::magenta);
                    painter.setPen(p);

                    QPointF * currIntersection =  x.first->getIntersectionPoint();
                    painter.drawPoint(currIntersection->x(),currIntersection->y());

                    painter.drawText(currIntersection->x()+10,currIntersection->y()+15,x.first->getIndexes());
                    p.setWidth(2);
                    painter.setPen(p);
                    painter.drawLine(x.first->startPoint->x(),x.first->startPoint->y()
                                     ,currIntersection->x(),currIntersection->y());
                }
            }


        if(tackeVoronoiDijagrama.size()>0){
            for (int i = 0; i < tackeVoronoiDijagrama.size(); ++i) {
                QPointF* tackaDijagrama = tackeVoronoiDijagrama[i];
                p.setWidth(7);
                p.setCapStyle(Qt::RoundCap);
                p.setColor(Qt::black);
                painter.setPen(p);
                painter.drawPoint(tackaDijagrama->x(),tackaDijagrama->y());

            }
        }

        if(linijeVoronoiDijagrama.size()>0){
            for (int i = 0; i < linijeVoronoiDijagrama.size(); ++i) {
                Intersection* currIntersection=linijeVoronoiDijagrama[i];
                p.setWidth(2);
                p.setCapStyle(Qt::RoundCap);
                p.setColor(Qt::black);
                painter.setPen(p);
                painter.drawLine(currIntersection->startPoint->x(),currIntersection->startPoint->y()
                                 ,currIntersection->endPoint->x(),currIntersection->endPoint->y());

            }
        }

    }

}

float distance(QPointF pt, int inX,int inY){
    return qSqrt(qPow(pt.x()-inX,2)+qPow(pt.y()-inY,2));
}

void VoronoiDiagram::runNaiveAlgorithm(){
    double maxX=0,maxY=0;
    for (int i = 0; i < points.size(); ++i) {
        if(points[i].x()>maxX)
            maxX=points[i].x();
        if(points[i].y()>maxY)
            maxY=points[i].y();
    }


    mapPointHull=naiveMap(true,maxX,maxY);

    for (auto const& x : (*mapPointHull))
    {
        std::vector<QPointF*>* poligon= x.second;
        for (int j = 0; j < poligon->size(); ++j) {
            if((*poligon)[j]->x() >0 && (*poligon)[j]->x()<maxX && (*poligon)[j]->y() >0 && (*poligon)[j]->y()<maxY){
                tackeNaivnogSet.insert((*(*poligon)[j]));
                //qDebug()<< QString::number((*poligon)[j]->x())+","+QString::number((*poligon)[j]->y());
            }
        }
    }
    //qDebug()<<tackeNaivnogSet.size();
}
