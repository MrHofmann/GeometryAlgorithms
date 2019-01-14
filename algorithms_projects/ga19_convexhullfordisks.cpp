#include "ga19_convexhullfordisks.h"
namespace ConvexHullForDisks {
ConvexHullForDisks::ConvexHullForDisks(QWidget *pRender, int delayMs, std::string fileName)
    :AlgorithmBase (pRender, delayMs)
{
//     if(fileName==""){
//         srand(static_cast<unsigned>(time(0)));
//         for(int i = 0; i<5; i++) {
//             disks.push_back(new Disk(new QPointF(rand()% 400, rand()% 400), rand()%100));
//         }
//     }

//     std::sort(disks.begin(), disks.end(), [](Disk *d1, Disk * d2) {return d2->center->x() - d1->center->x();});


//   disks.push_back(new Disk(new QPointF(100, 500), 20));
//   disks.push_back(new Disk(new QPointF(150, 150), 90));
//   disks.push_back(new Disk(new QPointF(200, 250), 90));
//   disks.push_back(new Disk(new QPointF(540, 200), 100));
//   disks.push_back(new Disk(new QPointF(400, 50), 20));


    // non working example
    disks.push_back(new Disk(new QPointF(100, 100), 20));
    disks.push_back(new Disk(new QPointF(200, 200), 80));
    disks.push_back(new Disk(new QPointF(400, 400), 200));
    disks.push_back(new Disk(new QPointF(600, 600), 20));
    disks.push_back(new Disk(new QPointF(600, 100), 20));



    L_Star = new QLineF(100, 0, 0, 0);
}

std::vector<Disk*> ConvexHullForDisks::convexHull(std::vector<Disk*> disk_set)
{
    if(disk_set.size()<=1){
        return disk_set;
    }

//    std::sort(disk_set.begin(),disk_set.end(),[] (Disk* a, Disk* b) { return (b->center->x()+b->radius) - (a->center->x()+a->radius);});
    std::vector<Disk*> v1;
    std::vector<Disk*> v2;
    for(int i = 0; i< disk_set.size(); i++) {
        if(i < disk_set.size()/2) {
            v2.push_back(disk_set[i]);
        }
        else {
            v1.push_back(disk_set[i]);
        }
    }
    this->ConvexHull = merge(convexHull(v1), convexHull(v2));

    return this->ConvexHull;
}

bool ConvexHullForDisks::dominates(QLineF* suport_line_P, QLineF* suport_line_Q)
{
    return !utils::negativeOrientation(suport_line_P->p1(), suport_line_P->p2(), suport_line_Q->p1());
}
std::vector<Disk*> ConvexHullForDisks::add_to_convex_hull(std::vector<Disk*> CH, Disk* disk)
{
    if(CH.size()==0){
        CH.push_back(disk);
        return CH;
    }

    Disk* last = CH[CH.size() - 1];
    if(last!= disk){
        CH.push_back(disk);
    }
    return CH;
}

double alpha_angle(QLineF* L1, QLineF* L2)
{
    if(L2 == NULL){
        return 360;
    }
    return L2->angleTo(*L1);
}

Disk * succ(std::vector<Disk*> CH, int curr_index)
{
    // return CH[(curr_index + 1) % CH.size()];
    if(++ curr_index < CH.size())
        return CH[curr_index];
    else if(CH.size() > 1)
        return CH[1];
    return CH[0];
}

void ConvexHullForDisks::advance(std::vector<Disk*>& CH_S,std::vector<Disk*> CH_X, std::vector<Disk*> CH_Y, int&x, int& y, int& x_count, int& y_count)
{
    double a1, a2, a3, a4;
    a1 = alpha_angle(L_Star, CH_X[x]->tangent(CH_Y[y]));
    a2 = alpha_angle(L_Star, CH_X[x]->tangent(succ(CH_X, x)));
    a3 = alpha_angle(L_Star, CH_Y[y]->tangent(succ(CH_Y, y)));
    a4 = alpha_angle(L_Star, CH_Y[y]->tangent(CH_X[x]));

    if(a1 == std::min({a1, a2, a3}) && a1!= 360) {
        CH_S = this->add_to_convex_hull(CH_S, CH_Y[y]);
        if(a4 == std::min({a4, a2, a3}) && a4!=360) {
            CH_S = this->add_to_convex_hull(CH_S, CH_X[x]);
        }
    }

    if(a2 < a3){
        L_Star = CH_X[x]->tangent(succ(CH_X, x));
        x++;
    }
    if(a3 < a2) {
        L_Star = CH_Y[y]->tangent(succ(CH_Y, y));
        y++;
    }

    if(a3==360 && a2 == 360)
    {
        x = CH_Y.size();
        y = CH_X.size();
    }
}


std::vector<Disk*> ConvexHullForDisks::merge(std::vector<Disk*> CH_P, std::vector<Disk*> CH_Q)
{
    L_Star = new QLineF(100, 0, 0, 0);
    std::vector<Disk*> CH_S;

    int p = 0;
    int q = 0;
    int p_count =0, q_count=0;
    QLineF* Lp = CH_P[0]->tangent(L_Star);
    QLineF* Lq = CH_Q[0]->tangent(L_Star);

    do
    {
        if(dominates(Lp, Lq)){
            CH_S = this->add_to_convex_hull(CH_S, CH_P[p]);
            advance(CH_S, CH_P, CH_Q, p, q, p_count, q_count);
        }
        else {
            CH_S = this->add_to_convex_hull(CH_S, CH_Q[q]);
            advance(CH_S, CH_Q, CH_P, q, p, q_count, p_count);
        }
        if(p < CH_P.size())
            Lp = CH_P[p]->tangent(L_Star);
        if(q < CH_Q.size())
            Lq = CH_Q[q]->tangent(L_Star);
        if(p >= CH_P.size() || q >= CH_Q.size())
            break;
    } while(p_count< (CH_P.size())|| q_count< (CH_Q.size()));

    return this->add_to_convex_hull(CH_S, CH_S[0]);
}

void ConvexHullForDisks::runAlgorithm()
{
    this->ConvexHull = convexHull(this->disks);
    AlgorithmBase_updateCanvasAndBlock();
}

void ConvexHullForDisks::drawAlgorithm(QPainter &painter) const
{
    QPen p = painter.pen();
    p.setWidth(2);
    p.setColor(QColor::fromRgb(44, 44, 44));
    painter.setPen(p);

    for(int i = 0; i < this->disks.size(); i ++)
    {
        painter.drawEllipse(*this->disks[i]->center, this->disks[i]->radius, this->disks[i]->radius);
    }
    p.setColor(QColor::fromRgb(250, 128, 128));
    painter.setPen(p);

    for(int i = 0; i < this->ConvexHull.size(); i ++)
    {
        painter.drawEllipse(*this->ConvexHull[i]->center, this->ConvexHull[i]->radius, this->ConvexHull[i]->radius);
        auto bridge = this->ConvexHull[i]->tangent(this->ConvexHull[(i+1)% this->ConvexHull.size()]);
        if(bridge!= NULL)
        painter.drawLine(bridge->p1(), bridge->p2());
    }
}

void ConvexHullForDisks::runNaiveAlgorithm()
{

}

Disk::Disk(QPointF* center, double radius)
{
    this->center = center;
    this->radius = radius;
}

QLineF* Disk::tangent(Disk* other_disk) {

    double x1 = this->center->x();
    double y1 = this->center->y();
    double r = this -> radius;

    double x2 = other_disk->center->x();
    double y2 = other_disk->center->y();
    double R = other_disk->radius;

    if(abs(y2-y1)<0.001 && abs(x2-x1)<0.001){
        return NULL;
    }
    double gama = atan((y2 - y1)/(x2 - x1));


    double beta = asin((R - r)/sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2)));

    double alpha = beta-gama;
    if(x2 >= x1) {
        gama= gama+PI/2;
    }
    if(x2>=x1)
        alpha+=PI;
    double x3 = x1 + r * cos(PI/2 - alpha);
    double y3 = y1 + r * sin(PI/2 - alpha);
    double x4 = x2 + R * cos(PI/2 - alpha);
    double y4 = y2 + R * sin(PI/2 - alpha);

    return new QLineF(x3, y3, x4, y4);
}

QLineF* Disk::tangent(QLineF* parallel_line)
{
    if(parallel_line == NULL) {
        NULL;
    }
    qreal angle = parallel_line->angle();
    double x_touch = center->x() + cos((angle*PI)/180 + PI/2) * this->radius;
    double y_touch = center->y() - sin((angle*PI)/180 + PI/2) * this->radius;
    QPointF tangent_touch_point = QPointF(x_touch, y_touch);
    QLineF* tangent = new QLineF(tangent_touch_point, QPoint());
    tangent->setAngle(angle);
    return tangent;
}

}


