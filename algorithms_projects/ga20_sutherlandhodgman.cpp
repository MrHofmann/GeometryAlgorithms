#include "ga20_sutherlandhodgman.h"
#include "algorithms_practice/ga04_dcel.h"
#include "algorithms_practice/ga04_dceldemo.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include "utils.h"
#include <QFile>
#include <QTextStream>
#include <iostream>

using namespace std;

#define INF 10000

SutherlandHodgman::SutherlandHodgman(QWidget *pRender, int delayMs, std::string fileName, int n, int m)
    :AlgorithmBase(pRender, delayMs)
{

    subjectPolygonEdges.clear();
    subjectPolygonVertices.clear();
    clipPolygonEdges.clear();
    clipPolygonVertices.clear();

    if (fileName!="")
    {
        input = new DCEL(fileName);
    }
    else
    {
        vector<QPoint> subjectPolygonQPoints = AlgorithmBase::generateRandomSimplePoly(n);
        vector<QPoint> clipPolygonQPoints = AlgorithmBase::generateRandomSimplePoly(m);

        QFile out("./random.off");
        if (!out.open(QIODevice::ReadWrite | QIODevice::Truncate))
            return;
        QTextStream stream(&out);
        stream<<"OFF"<<endl;
        stream<<n+m<<" "<<"2"<<" 20"<<endl;
        stream<<endl;
        for (int i=0; i<n; i++)
        {
            double tmpx = subjectPolygonQPoints[i].x();
            double tmpy = subjectPolygonQPoints[i].y();
            double x = 2.0*tmpx/500.0-1;
            double y = 2.0*tmpy/500.0-1;
            stream<<x<<" "<<y<<" 0"<<endl;
        }
        stream<<endl;
        for (int i=0; i<m; i++)
        {
            double tmpx = clipPolygonQPoints[i].x();
            double tmpy = clipPolygonQPoints[i].y();
            double x = 2.0*tmpx/500.0-1;
            double y = 2.0*tmpy/500.0-1;
            stream<<x<<" "<<y<<" 0"<<endl;
        }
        stream<<endl;
        stream<<n<<" ";
        for(int i=0;i<n;i++)
            stream<<i<<" ";
        stream<<endl;
        stream<<m<<" ";
        for(int i=n;i<n+m;i++)
            stream<<i<<" ";
        out.close();
        input = new DCEL("./random.off");
    }
    subjectPolygon = input->fields()[0];
    clipPolygon = input->fields()[1];

    DCELHalfEdge* begin1 = subjectPolygon->outerComponent();
    DCELHalfEdge* curr1 = begin1->next();
    subjectPolygonVertices.push_back(begin1->origin());
    subjectPolygonEdges.push_back(begin1);
    while (curr1 != begin1)
    {
        subjectPolygonVertices.push_back(curr1->origin());
        subjectPolygonEdges.push_back(curr1);
        curr1 = curr1->next();
    }

    DCELHalfEdge* begin2 = clipPolygon->outerComponent();
    DCELHalfEdge* curr2 = begin2->next();
    clipPolygonVertices.push_back(begin2->origin());
    clipPolygonEdges.push_back(begin2);
    while (curr2 != begin2)
    {
        clipPolygonVertices.push_back(curr2->origin());
        clipPolygonEdges.push_back(curr2);
        curr2 = curr2->next();
    }

    clipEdgeTest = nullptr;
    outputList = subjectPolygonVertices;
}

DCELVertex* SutherlandHodgman::ComputeIntersection(DCELVertex* a, DCELVertex* b, DCELHalfEdge* edge)
{
    double xa, ya, xb, yb, xc, yc, xd, yd;
    xa = a->coordinates().x();
    ya = a->coordinates().y();
    xb = b->coordinates().x();
    yb = b->coordinates().y();
    xc = edge->origin()->coordinates().x();
    yc = edge->origin()->coordinates().y();
    xd = edge->twin()->origin()->coordinates().x();
    yd = edge->twin()->origin()->coordinates().y();

    double a1 = yb - ya;
    double b1 = xa - xb;
    double c1 = a1*xa + b1*ya;

    double a2 = yd - yc;
    double b2 = xc - xd;
    double c2 = a2*xc+ b2*yc;

    double determinanta = a1*b2 - a2*b1;

    if (determinanta == 0)
    {
        return nullptr;
    }

    double x = (b2*c1 - b1*c2)/determinanta;
    double y = (a1*c2 - a2*c1)/determinanta;
    return new DCELVertex(QPoint(x, y), nullptr);

}

bool SutherlandHodgman::IsInFrontOf(DCELVertex* v, DCELHalfEdge* edge)
{
    DCELVertex* edgeStart = edge->origin();
    DCELVertex* edgeEnd = edge->twin()->origin();
    return !utils::negativeOrientation(edgeStart->coordinates(), v->coordinates(), edgeEnd->coordinates(), false);
}

bool SutherlandHodgman::IsPolygonConvex(vector<DCELVertex*>& poly)
{
    //provera za neispravan ulaz, mada ce se proveriti i pre toga
    if (poly.size() < 3)
    {
        return false;
    }

    if (!utils::negativeOrientation(poly[poly.size()-2]->coordinates(), poly[poly.size()-1]->coordinates(), poly[0]->coordinates()))
    {
        return false;
    }

    if (!utils::negativeOrientation(poly[poly.size()-1]->coordinates(), poly[0]->coordinates(), poly[1]->coordinates()))
    {
        return false;
    }

    for(size_t i=0; i< poly.size()-2; i++)
    {
        if (!utils::negativeOrientation(poly[i]->coordinates(), poly[i+1]->coordinates(), poly[i+2]->coordinates()))
        {
            return false;
        }
    }

    return true;
}

void SutherlandHodgman::runAlgorithm()
{   
    if (clipPolygonVertices.size() < 3 || subjectPolygonVertices.size() < 3)
    {
        outputList.clear();
        emit animationFinished();
        return;
    }
    if (!SutherlandHodgman::IsPolygonConvex(clipPolygonVertices))
    {
        outputList.clear();
        emit animationFinished();
        return;
    }

    vector<DCELVertex*> inputList;
    outputList = subjectPolygonVertices;
    AlgorithmBase_updateCanvasAndBlock();
    for (DCELHalfEdge* clipEdge : clipPolygonEdges)
    {
        clipEdgeTest = clipEdge;
        inputList = outputList;
        outputList.clear();
        DCELVertex* S = inputList.back();
        for (size_t i=0;i<inputList.size();i++)
        {
            DCELVertex* E = inputList[i];
            if (IsInFrontOf(E, clipEdge))
            {
                if (!IsInFrontOf(S, clipEdge))
                {
                    outputList.push_back(ComputeIntersection(S, E, clipEdge));
                }
                outputList.push_back(E);
            }
            else if (IsInFrontOf(S, clipEdge))
            {
                outputList.push_back(ComputeIntersection(S, E, clipEdge));
            }
            S = E;
            //AlgorithmBase_updateCanvasAndBlock();
        }
        AlgorithmBase_updateCanvasAndBlock();
    }

    AlgorithmBase_updateCanvasAndBlock();

    emit animationFinished();
}

void SutherlandHodgman::drawAlgorithm(QPainter &painter) const
{

    QPen p = painter.pen();

    p.setColor(Qt::red);
    p.setWidth(3);
    painter.setPen(p);
    QPainterPath path;
    if (outputList.size()!=0)
    {
        path.moveTo(outputList[0]->coordinates());
        for (size_t i=0; i<outputList.size()-1; i++)
        {
            painter.drawLine(outputList[i]->coordinates(), outputList[i+1]->coordinates());
            path.lineTo(outputList[i]->coordinates());
        }
        path.lineTo(outputList[outputList.size()-1]->coordinates());
        path.lineTo(outputList[0]->coordinates());
        painter.fillPath(path, Qt::red);
        painter.drawLine(outputList[outputList.size()-1]->coordinates(), outputList[0]->coordinates());
    }

    p.setColor(Qt::blue); 
    painter.setPen(p);
    if (clipPolygonVertices.size()!=0)
    {
        for (size_t i=0; i<clipPolygonVertices.size()-1; i++)
        {
            painter.drawLine(clipPolygonVertices[i]->coordinates(), clipPolygonVertices[i+1]->coordinates());
        }
        painter.drawLine(clipPolygonVertices[clipPolygonVertices.size()-1]->coordinates(), clipPolygonVertices[0]->coordinates());
    }

    p.setColor(Qt::yellow);
    painter.setPen(p);
    if(clipEdgeTest != nullptr)
        painter.drawLine(clipEdgeTest->origin()->coordinates(), clipEdgeTest->twin()->origin()->coordinates());

}

bool SutherlandHodgman::onSegment(DCELVertex* p, DCELVertex* q, DCELVertex* r)
{
    if (q->coordinates().x() <= max(p->coordinates().x(), r->coordinates().x()) && q->coordinates().x() >= min(p->coordinates().x(), r->coordinates().x()) &&
            q->coordinates().y() <= max(p->coordinates().y(), r->coordinates().y()) && q->coordinates().y() >= min(p->coordinates().y(), r->coordinates().y()))
        return true;
    return false;
}

int SutherlandHodgman::orientation(DCELVertex* p, DCELVertex* q, DCELVertex* r)
{
    int val = (q->coordinates().y() - p->coordinates().y()) * (r->coordinates().x() - q->coordinates().x()) -
              (q->coordinates().x() - p->coordinates().x()) * (r->coordinates().y() - q->coordinates().y());

    if (val == 0) return 0;  // colinear
    return (val > 0)? 1: 2; // clock or counterclock wise
}

bool SutherlandHodgman::doIntersect(DCELVertex* p1, DCELVertex* q1, DCELVertex* p2, DCELVertex* q2)
{
    // Find the four orientations needed for general and
    // special cases
    int o1 = orientation(p1, q1, p2);
    int o2 = orientation(p1, q1, q2);
    int o3 = orientation(p2, q2, p1);
    int o4 = orientation(p2, q2, q1);

    // General case
    if (o1 != o2 && o3 != o4)
        return true;

    // Special Cases
    // p1, q1 and p2 are colinear and p2 lies on segment p1q1
    if (o1 == 0 && onSegment(p1, p2, q1)) return true;

    // p1, q1 and p2 are colinear and q2 lies on segment p1q1
    if (o2 == 0 && onSegment(p1, q2, q1)) return true;

    // p2, q2 and p1 are colinear and p1 lies on segment p2q2
    if (o3 == 0 && onSegment(p2, p1, q2)) return true;

     // p2, q2 and q1 are colinear and q1 lies on segment p2q2
    if (o4 == 0 && onSegment(p2, q1, q2)) return true;

    return false; // Doesn't fall in any of the above cases
}

bool SutherlandHodgman::isInside( DCELVertex* p, vector<DCELVertex*> polygon)
{
    int n = polygon.size();

    if (n<3)
        return false;

    DCELVertex* extreme = new DCELVertex(QPoint(INF, p->coordinates().y()), nullptr);

    int count = 0, i=0;
    do
    {
        int next = (i+1)%n;
        if (doIntersect(polygon[i], polygon[next], p, extreme))
        {
            if (orientation(polygon[i], p, polygon[next])==0)
            {
                return onSegment(polygon[i], p, polygon[next]);
            }
            count++;
        }
        i=next;
    }
    while (i!=0);
    return (count%2==1);
}

bool SutherlandHodgman::IsOnLineSegment(DCELVertex* point, DCELVertex* start, DCELVertex* end)
{
    double minx = min(start->coordinates().x(), end->coordinates().x());
    double maxx = max(start->coordinates().x(), end->coordinates().x());
    double miny = min(start->coordinates().y(), end->coordinates().y());
    double maxy = max(start->coordinates().y(), end->coordinates().y());
    if (point->coordinates().x() >= minx && point->coordinates().x() <= maxx && point->coordinates().y() >= miny && point->coordinates().y() <= maxy)
        return true;
    else
        return false;
}

DCELVertex* SutherlandHodgman::ComputeIntersection1(DCELHalfEdge* e1, DCELHalfEdge* e2)
{
    DCELVertex* res = ComputeIntersection(e1->origin(), e1->twin()->origin(), e2);
    if (res == nullptr)
        return nullptr;
    if (IsOnLineSegment(res, e1->origin(), e1->twin()->origin()) && IsOnLineSegment(res, e2->origin(), e2->twin()->origin()))
        return res;
    else
        return nullptr;
}

void SutherlandHodgman::runNaiveAlgorithm()
{

    outputList.clear();
    if (clipPolygonVertices.size() < 3 || subjectPolygonVertices.size() < 3)
        return;

    if (!IsPolygonConvex(clipPolygonVertices))
        return;

    for (DCELHalfEdge* subjectEdge : subjectPolygonEdges)
    {
        for (DCELHalfEdge* clipEdge : clipPolygonEdges)
        {
            DCELVertex* tmp = ComputeIntersection1(subjectEdge, clipEdge);
            if (tmp != nullptr)
            {
                AlgorithmBase_updateCanvasAndBlock();
                outputList.push_back(tmp);
            }
        }
    }
    for (DCELVertex* v1 :subjectPolygonVertices)
    {
        if (isInside(v1, clipPolygonVertices))
        {
            outputList.push_back(v1);
        }
    }
    for (DCELVertex* v2 : clipPolygonVertices)
    {
        if (isInside(v2, subjectPolygonVertices))
        {
            outputList.push_back(v2);
        }
    }

    AlgorithmBase_updateCanvasAndBlock();

}
