#include "ga12_datastructures.h"

Segment::Segment(QLineF segment, DCELField* fieldAbove, std::string fieldName)
    : _segment(segment), _fieldAbove(fieldAbove), _fieldName(fieldName)
{
    //zelimo da p1 bude lijeva tacka a p2 desna, pa ako vec nisu razmijenimo njihove vrijednosti
    if(_segment.p1().x() > _segment.p2().x() ||
            (_segment.p1().x() == _segment.p2().x() && _segment.p1().y() > _segment.p2().y())) {

        _segment.setP1(segment.p2());

       _segment.setP2(segment.p1());
    }

}

QPointF Segment::pointWithX (qreal x) const {

    return QPointF(x, (_segment.p2().y() - _segment.p1().y()) /
                      (_segment.p2().x() - _segment.p1().x()) *
                      (x - _segment.p1().x()) +
                      _segment.p1().y());
}

bool Segment::isAbove(const QPointF & p, const QPointF &helper) {
    double det = pointPosition(p);
    double eps = 1e-3;
    return fabs(det) > eps ? det > 0 : pointPosition(helper)>0;
}


bool Segment::operator==(const Segment& s) const {
    return _segment.p1() == s._segment.p1()
           && _segment.p2() == s._segment.p2();
}

bool Segment::intersects(const Segment& s) const {
    QPointF intersectionP;
    if(_segment.intersect(s._segment, &intersectionP) == QLineF::BoundedIntersection)
    {
        if(intersectionP == s._segment.p1() || intersectionP == s._segment.p2())
            return false;
        return true;
    }
    return false;
}


double Segment::pointPosition(const QPointF& p) {
    return (p.y() - _segment.p1().y())*(_segment.p2().x()-_segment.p1().x())
            -(_segment.p2().y()-_segment.p1().y())*(p.x()-_segment.p1().x());
}

Trapezoid::Trapezoid() :
    _lowerL(nullptr), _upperL(nullptr), _lowerR(nullptr), _upperR(nullptr), _searchNode(nullptr)
{}

void Trapezoid::setOneLeft(Trapezoid *trap) {
    _upperL = trap;
    _lowerL = nullptr;
}

void Trapezoid::setOneRight(Trapezoid *trap) {
    _upperR = trap;
    _lowerR = nullptr;
}

void Trapezoid::changeLeftWithGiven(Trapezoid *tr) {
    if(_lowerL) {
        if(_lowerL->lowerR() == this)
            _lowerL->setLowerR(tr);
        else
            _lowerL->setUpperR(tr);
    }

    if(_upperL) {
        if(_upperL->lowerR() == this)
            _upperL->setLowerR(tr);
        else
            _upperL->setUpperR(tr);
    }

}

void Trapezoid::changeRightWithGiven(Trapezoid *tr) {
    if(_lowerR) {
        if(_lowerR->lowerL() == this)
            _lowerR->setLowerL(tr);
        else
            _lowerR->setUpperL(tr);
    }

    if(_upperR) {
        if(_upperR->lowerL() == this)
            _upperR->setLowerL(tr);
        else
            _upperR->setUpperL(tr);
    }

}

void Trapezoid::draw(QPainter &painter)
{
    //gornja i donja vertikalna ekstenznija iz lijeve tacke
    painter.drawLine(leftP(), top()->pointWithX(leftP().x()));
    painter.drawLine(leftP(), bottom()->pointWithX(leftP().x()));

    //gornja i donja vertikalna ekstenzija iz desne tacke
    painter.drawLine(rightP(), top()->pointWithX(rightP().x()));
    painter.drawLine(rightP(), bottom()->pointWithX(rightP().x()));

    //2 ne-vertikalne ivice trapeza
    painter.drawLine(top()->pointWithX(leftP().x()), top()->pointWithX(rightP().x()));
    painter.drawLine(bottom()->pointWithX(leftP().x()), bottom()->pointWithX(rightP().x()));

}

SearchNode::SearchNode() :
    left(nullptr), right(nullptr) {}

void SearchNode::addLeft(SearchNode *node) {
    left = node;
    node->_parents.push_back(this);
}

void SearchNode::addRight(SearchNode *node) {
    right = node;
    node->_parents.push_back(this);
}

void SearchNode::replaceNode(SearchNode *node) {
    for(auto p : _parents) {
        if(p->left == this)
            p->left = node;
        else
            p->right = node;
    }
}

XNode::XNode(QPointF point) : _point(point) {}

SearchNode* XNode::nextNode(QPointF p1, QPointF) {
    if(p1.x() < _point.x())
        return left;

    return right;
}

void XNode::draw(QPainter & p) const {
    p.drawPoint(_point);
}

YNode::YNode(Segment* segment) : _segment(segment)
{}

SearchNode* YNode::nextNode(QPointF p1, QPointF p2) {
    if(_segment->isAbove(p1, p2))
        return left;
    return right;
}

void YNode::draw(QPainter &p) const {
    p.drawLine(_segment->_segment.p1(), _segment->_segment.p2());
}

LeafNode::LeafNode(Trapezoid* trapezoid)
    : _trapezoid(trapezoid) {
    _trapezoid->setSearchNode(this);
}

LeafNode::~LeafNode() {
    delete _trapezoid;
}

SearchNode* LeafNode::nextNode(QPointF , QPointF ) {
    return this;
}

void LeafNode::draw(QPainter & p) const {
    this->getTrapezoid()->draw(p);
}

QPointF Trapezoid::leftP() const
{
    return _leftP;
}

void Trapezoid::setLeftP(const QPointF &leftP)
{
    _leftP = leftP;
}

QPointF Trapezoid::rightP() const
{
    return _rightP;
}

void Trapezoid::setRightP(const QPointF &rightP)
{
    _rightP = rightP;
}

Segment *Trapezoid::top() const
{
    return _top;
}

void Trapezoid::setTop(Segment *top)
{
    _top = top;
}

Segment *Trapezoid::bottom() const
{
    return _bottom;
}

void Trapezoid::setBottom(Segment *bottom)
{
    _bottom = bottom;
}

Trapezoid *Trapezoid::lowerL() const
{
    return _lowerL;
}

void Trapezoid::setLowerL(Trapezoid *lowerL)
{
    _lowerL = lowerL;
}

Trapezoid *Trapezoid::upperL() const
{
    return _upperL;
}

void Trapezoid::setUpperL(Trapezoid *upperL)
{
    _upperL = upperL;
}

Trapezoid *Trapezoid::lowerR() const
{
    return _lowerR;
}

void Trapezoid::setLowerR(Trapezoid *lowerR)
{
    _lowerR = lowerR;
}

Trapezoid *Trapezoid::upperR() const
{
    return _upperR;
}

void Trapezoid::setUpperR(Trapezoid *upperR)
{
    _upperR = upperR;
}

SearchNode *Trapezoid::searchNode() const
{
    return _searchNode;
}

void Trapezoid::setSearchNode(SearchNode *searchNode)
{
    _searchNode = searchNode;
}

Trapezoid *LeafNode::getTrapezoid() const
{
    return _trapezoid;
}

void LeafNode::setTrapezoid(Trapezoid *trapezoid)
{
    _trapezoid = trapezoid;
}

DCELField *Segment::fieldAbove() const
{
    return _fieldAbove;
}

void Segment::setFieldAbove(DCELField *fieldAbove)
{
    _fieldAbove = fieldAbove;
}
