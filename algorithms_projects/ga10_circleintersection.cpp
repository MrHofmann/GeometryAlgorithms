#include "ga10_circleintersection.h"


CircleIntersection::CircleIntersection(QWidget *pRenderer, int delayMs, std::string filename, int circlesNum)
    :AlgorithmBase(pRenderer, delayMs), _intersections(), _intersectionsNaive() , _events(), _status(), _y{0}
{
    if(filename == "")
        _circles = generateRandomCircles(circlesNum);
    else
        _circles = readCirclesFromFile(filename);

    for(CircleI c: _circles) {
        // event point when line touches the circle for the first time
        QPoint p1(c.center().x(), c.center().y() - c.radius());
        EventPoint ep1(UPPER, p1, c);
        _events.insert(ep1);

        // event point when line touches the circle for the last time
        QPoint p2(c.center().x(), c.center().y() + c.radius());
        EventPoint ep2(LOWER, p2, c);
        _events.insert(ep2);
    }
}


std::vector<CircleI> CircleIntersection::generateRandomCircles(int circlesNum)
{
    srand(static_cast<unsigned>(time(0)));

    int width, height;
    if(this->_pRenderer)
    {
        width = _pRenderer->width();
        height = _pRenderer->height();
    }
    else
    {
        width = 1200;
        height = 500;
    }

    int rMax = 100;
    int xMax = width - getDRAWING_BORDER() - rMax;
    int yMax = height  - getDRAWING_BORDER() - rMax;

    int rMin = 10;
    int xMin = getDRAWING_BORDER() + rMax;
    int yMin = getDRAWING_BORDER() + rMax;

    std::vector<CircleI> randomCircles;

    int xDiff = xMax-xMin;
    int yDiff = yMax-yMin;
    int rDiff = rMax-rMin;
    int ind;
    for(int i=0; i < circlesNum; i++) {
        int x  = xMin + rand()%xDiff;
        int y = yMin + rand()%yDiff;
        int r = rMin + rand()%rDiff;
        QPoint q(x, y);
        CircleI c(q, r);

        // preprocessing data so there's no duplicates
        ind = 0;
        for(CircleI ci: randomCircles) {
            if(c.equal(ci)) {
                ind = 1;
                break;
            }

        }
        if(!ind)
            randomCircles.emplace_back(c);
    }

    return randomCircles;
}

std::vector<CircleI> CircleIntersection::readCirclesFromFile(std::string fileName)
{
    std::ifstream inputFile(fileName);
    std::vector<CircleI> circles;
    int x, y, r, ind;
    while(inputFile >> x >> y >> r)
    {
        QPoint p(x, y);
        CircleI c(p, r);

        // preprocessing data so there's no duplicates
        ind = 0;
        for(CircleI ci: circles) {
            if(c.equal(ci)) {
                ind = 1;
                break;
            }

        }
        if(!ind)
            circles.emplace_back(c);
    }
    return circles;
}

void CircleIntersection::runAlgorithm()
{
    if(_circles.size() < 2) {
        emit animationFinished();
        return;
    }

    while(!_events.empty()) {
        auto it = _events.begin();
        EventPoint p = *it;
        _y = p.point().y();

        if(p.type() == UPPER) {
            for(CircleI c : _status) {
                p.circle().intersections(c, &_intersections);
            }
            _status.insert(p.circle());
        }
        else {
            _status.erase(p.circle());
        }

        _events.erase(it);
        AlgorithmBase_updateCanvasAndBlock();
    }

    emit animationFinished();
}

void CircleIntersection::drawAlgorithm(QPainter &painter) const
{
    QPen p = painter.pen();
    p.setColor(Qt::darkYellow);
    p.setWidth(1);
    painter.setPen(p);

    // drawing every circle
    for(CircleI c: _circles){
        painter.drawEllipse(c.center(), c.radius(), c.radius());
    }


    // marking every circle in status
    p.setColor(Qt::blue);
    p.setWidth(2);
    painter.setPen(p);
    for(CircleI c : _status) {
        painter.drawEllipse(c.center(), c.radius(), c.radius());
    }


    // marking every intersection
    p.setColor(Qt::red);
    p.setWidth(4);
    painter.setPen(p);
    for(QPoint pt : _intersections) {
         painter.drawPoint(pt);
    }

    // drawing sweep line
    p.setColor(Qt::green);
    p.setWidth(2);
    painter.setPen(p);
    painter.drawLine(0, _y, _pRenderer->width(), _y);
}

void CircleIntersection::runNaiveAlgorithm()
{
    for(unsigned i=0; i<_circles.size(); i++) {
        for(unsigned j=0; j<_circles.size(); j++) {
            if(i != j)
                _circles[i].intersections(_circles[j], &_intersectionsNaive);
        }
    }
}

std::set<QPoint, PointComparator> CircleIntersection::intersections() const
{
    return _intersections;
}

void CircleIntersection::setIntersections(const std::set<QPoint, PointComparator> &intersections)
{
    _intersections = intersections;
}

std::set<QPoint, PointComparator> CircleIntersection::intersectionsNaive() const
{
    return _intersectionsNaive;
}

void CircleIntersection::setIntersectionsNaive(const std::set<QPoint, PointComparator> &intersectionsNaive)
{
    _intersectionsNaive = intersectionsNaive;
}

void CircleIntersection::setCircles(const std::vector<CircleI> &circles)
{
    for(CircleI c: circles) {
        // event point when line touches the circle for the first time
        QPoint p1(c.center().x(), c.center().y() - c.radius());
        EventPoint ep1(UPPER, p1, c);
        _events.insert(ep1);

        // event point when line touches the circle for the last time
        QPoint p2(c.center().x(), c.center().y() + c.radius());
        EventPoint ep2(LOWER, p2, c);
        _events.insert(ep2);
    }

    _circles = circles;
}


CircleI::CircleI(const QPoint p, int r)
    :_center{p}, _radius{r}
{

}

CircleI::CircleI(int x, int y, int r)
    :_radius{r}
{
    QPoint p(x, y);
    _center = p;
}

QPoint CircleI::center() const
{
    return _center;
}

void CircleI::setCenter(const QPoint &center)
{
    _center = center;
}

int CircleI::radius() const
{
    return _radius;
}

void CircleI::setRadius(int radius)
{
    _radius = radius;
}

bool CircleI::equal(const CircleI c) const
{
    if(this->center().x() != c.center().x()) {
        return false;
    }
    else if(this->center().y() != c.center().y()) {
        return false;
    }
    else if(this->radius() != c.radius()) {
        return false;
    }
    else {
        return true;
    }
}

int CircleI::intersecting(const CircleI c) const
{
    double d = utils::distance(this->center(), c.center());
    if(d < this->radius() + c.radius()) {
        return 2;
    }
    else if(d == this->radius() + c.radius()) {
        return 1;
    }
    else {
        return 0;
    }
}

void CircleI::intersections(const CircleI c, std::set<QPoint, PointComparator> *intersectionSet)
{
    int n = this->intersecting(c);
    QPoint p1, p2;
    if(n) {
        int r1, r2, x1, x2, y1, y2;
        if(this->radius() < c.radius()) {
            r2 = this->radius();
            x2 = this->center().x();
            y2 = this->center().y();
            r1 = c.radius();
            x1 = c.center().x();
            y1 = c.center().y();
        }
        else {
            r1 = this->radius();
            x1 = this->center().x();
            y1 = this->center().y();
            r2 = c.radius();
            x2 = c.center().x();
            y2 = c.center().y();
        }


        double d = utils::distance(this->center(), c.center());
        double a = (r1*r1 - r2*r2 + d*d)/(2*d);
        double h = sqrt(r1*r1-a*a);

        if(n==1) {
            p1.setX(x1 + a*(x2-x1)/d + h*(y2-y1)/d);
            p1.setY(y1 + a*(y2-y1)/d - h*(x2-x1)/d);

            if(p1.x() > 0)
                intersectionSet->insert(p1);
        }
        else if(n==2) {
            p1.setX(x1 + a*(x2-x1)/d + h*(y2-y1)/d);
            p1.setY(y1 + a*(y2-y1)/d - h*(x2-x1)/d);
            p2.setX(x1 + a*(x2-x1)/d - h*(y2-y1)/d);
            p2.setY(y1 + a*(y2-y1)/d + h*(x2-x1)/d);
            if(p1.x() > 0)
                intersectionSet->insert(p1);
            if(p2.x() > 0)
                intersectionSet->insert(p2);
        }
    }
}




EventPoint::EventPoint(const EType et, const QPoint p, const CircleI c)
    :_type{et}, _point{p}, _circle{c}
{

}

EType EventPoint::type() const
{
    return _type;
}

void EventPoint::setType(const EType &type)
{
    _type = type;
}

QPoint EventPoint::point() const
{
    return _point;
}

void EventPoint::setPoint(const QPoint &point)
{
    _point = point;
}

CircleI EventPoint::circle() const
{
    return _circle;
}

void EventPoint::setCircle(const CircleI &circle)
{
    _circle = circle;
}

