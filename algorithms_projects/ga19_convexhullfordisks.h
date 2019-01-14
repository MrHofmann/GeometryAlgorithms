#ifndef GA19_CONVEXHULLFORDISKS_H
#define GA19_CONVEXHULLFORDISKS_H
#define PI 3.14159265


#include "algorithmbase.h"
#include <QLine>
#include <set>
#include <math.h>
#include "utils.h"
namespace ConvexHullForDisks {


class Disk {
public:
    Disk(QPointF* center, double radius);
    QLineF* tangent(Disk* other_circle);
    QLineF* tangent(QLineF* parallel_line);
    QPointF* center;
    qreal radius;
};


class ConvexHullForDisks : public AlgorithmBase
{
public:
    ConvexHullForDisks(QWidget *pRender, int delayMs, std::string fileName);

    void runAlgorithm();
    void drawAlgorithm(QPainter &painter) const;
    void runNaiveAlgorithm();
    bool dominates(QLineF* suport_line_P, QLineF* suport_line_Q);
    std::vector<Disk*> add_to_convex_hull(std::vector<Disk*> CH, Disk* disk);
    std::vector<Disk*> convexHull(std::vector<Disk*>);
    std::vector<Disk*> merge(std::vector<Disk*> CH_P, std::vector<Disk*> CH_Q);
    void advance(std::vector<Disk*>& CH_S,std::vector<Disk*> CH_X, std::vector<Disk*> CH_Y, int&x, int& y, int& x_count, int& y_count);

    QLineF* L_Star;

    std::vector<Disk*> disks;
    std::vector<Disk*> ConvexHull;
    std::vector<QLineF*> bridges;

};


class DiskSet {
public:
    DiskSet(std::vector<Disk*>);
    std::pair<DiskSet*, DiskSet*> split();
    std::vector<std::pair<Disk*, bool>> disk_visit;
    Disk* succ();
    Disk* current();
};

}
#endif // GA19_CONVEXHULLFORDISKS_H
