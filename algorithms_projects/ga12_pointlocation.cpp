#include "ga12_pointlocation.h"
#include <iostream>
#include <algorithm>
#include <set>
#include <QProcess>

#define MINX 0.0
#define MAXX 922.0
#define MINY 0.0
#define MAXY 525.0


void TrapezoidMap::makeInputFromFile(std::string filename) {
    DCEL myDcel(filename);
    for(DCELHalfEdge* he : myDcel.edges()) {
        if(he->origin()->coordinates().x() > he->twin()->origin()->coordinates().x()) {
            bool alreadyExists = false;
            std::string id;

            //check whether field above current segment has already been added
            for(Segment segAdded : _segments) {

                if(segAdded.fieldAbove() == he->incidentFace()) {
                    id = segAdded._fieldName;
                    alreadyExists = true;
                    break;
                }
            }


            if(!alreadyExists && he->incidentFace() == nullptr) {
                alreadyExists = true;
                id = "0F";
            }


            if(!alreadyExists) {
                id = std::string(std::to_string(++_fieldNumber).append("F"));
            }
            _segments.push_back(Segment(
                  QLineF(he->origin()->coordinates(),
                         he->twin()->origin()->coordinates()),
                 he->incidentFace(), id));
        }
    }

}


void TrapezoidMap::makeRandomInput(int dimension) {
    QString outputFilename = "random_segments.txt";

    const QString command =
            QString("python ../algorithms/input_files/ga12_pointlocation/random_gen.py ")
            .append(QString::number(dimension))
            .append(" ").append(outputFilename);

    QProcess::execute(command);
    std::ifstream f(outputFilename.toStdString());
    if(!f.is_open()) {
        std::cout << "Neuspjesno otvaranje fajla!" << std::endl;
        return;
    }
    float x1,y1,x2,y2;
    while(f >> x1 >> y1 >> x2 >> y2)
        _segments.push_back(Segment(QLineF(QPointF(x1,y1),QPointF(x2,y2))));

}

bool TrapezoidMap::checkInputSegments() {

    for (auto seg1 = _segments.begin(); seg1 != _segments.end(); seg1++) {
        for (auto seg2 = seg1; seg2 != _segments.end(); seg2++) {
            if (seg1 == seg2)
                continue;
            if (seg1->intersects(*seg2))
                return true;
        }
    }
    return false;
}

TrapezoidMap::TrapezoidMap(QWidget *pRenderer, int delayMs,  QPointF locateMe,
                      std::string filename, int dimension)
    : AlgorithmBase(pRenderer, delayMs), _pointLocation(locateMe),
                                        _root(nullptr), _ready(false)
{
    _nextSeg = true;
    _currentNode = false;
    _currentNodeSearching = nullptr;
    _currentEndPoint = false;

    if(filename != "") {
        makeInputFromFile(filename);
        std::random_shuffle(std::begin(_segments), std::end(_segments));
    }
    else {
        if(dimension <= 0)
            _status = AlgorithmStatus::INVALID_INPUT;
        else {
            _status = AlgorithmStatus::CORRECT_INPUT;
            makeRandomInput(dimension);
        }
    }
    if(checkInputSegments() || _segments.size() == 0)
        _status = AlgorithmStatus::INVALID_INPUT;
    else
        _status = AlgorithmStatus::CORRECT_INPUT;

    _segments.push_back(Segment(QLineF(QPointF(MINX, MINY), QPointF(MAXX, MINY))));
    _segments.push_back(Segment(QLineF(QPointF(MINX, MAXY), QPointF(MAXX, MAXY))));
}

std::unordered_set<SearchNode*> _visited;

void searchGraphHelper(SearchNode* node, std::function<void(SearchNode*)> f) {

        if(_visited.find(node) != _visited.end())
            return;
        _visited.insert(node);
        if(node->left != nullptr)
            searchGraphHelper(node->left, f);
        if(node->right != nullptr)
            searchGraphHelper(node->right, f);
        f(node);
}

void TrapezoidMap::makeEmpty() {
    if(!_ready)
        return;
    _segments.clear();
    _ready = false;

    std::unordered_set<SearchNode*> toDeleteNodes;
    auto deleteFun = [&toDeleteNodes](SearchNode *node) { toDeleteNodes.insert(node); };

    _visited.clear();
    searchGraphHelper(_root, deleteFun);

    for(auto node : toDeleteNodes)
        delete node;
    _root = nullptr;
}

void TrapezoidMap::getTrapezoids(std::vector<Trapezoid*>& traps) const {

    std::unordered_set<Trapezoid*> tmpTrapezoids;
    auto trapezoidFun = [&tmpTrapezoids](SearchNode *node){
        if(node->getTrapezoid() != nullptr)
            tmpTrapezoids.insert(node->getTrapezoid());
    };

    _visited.clear();
    searchGraphHelper(_root, trapezoidFun);

    std::copy(std::begin(tmpTrapezoids), std::end(tmpTrapezoids),
              std::back_inserter(traps));

}

void TrapezoidMap::updateMyDrawing() {
    AlgorithmBase_updateCanvasAndBlock();
}


SearchNode* TrapezoidMap::query(QPointF p1, QPointF p2) {
    SearchNode *current = _root;
    _currentNode = true;
    _currentNodeSearching = _root;
    if(_destroyAnimation)
        return nullptr;
    updateMyDrawing();
    _currentNode = false;

    while(current->getTrapezoid() == nullptr) {
        current = current->nextNode(p1, p2);
        _currentNode = true;
        _currentNodeSearching = current;
        if(_destroyAnimation)
            return nullptr;
        updateMyDrawing();
        _currentNode = false;

    }

    _currentNode = false;

    return current;
}


void TrapezoidMap::simpleCase(SearchNode *trapNode, Segment *segment)
{

    //trapez se dijeli na 4 trapeza: newLeft, newRight, newTop, newBottom
    Trapezoid* newLeft = new Trapezoid(*trapNode->getTrapezoid());
    newLeft->setRightP(segment->_segment.p1());

    Trapezoid* newRight = new Trapezoid(*trapNode->getTrapezoid());
    newRight->setLeftP(segment->_segment.p2());

    Trapezoid* newTop = new Trapezoid(*trapNode->getTrapezoid());
    newTop->setLeftP(segment->_segment.p1());
    newTop->setRightP(segment->_segment.p2());
    newTop->setBottom(segment);

    Trapezoid* newBottom = new Trapezoid(*trapNode->getTrapezoid());
    newBottom->setLeftP(segment->_segment.p1());
    newBottom->setRightP(segment->_segment.p2());
    newBottom->setTop(segment);

    //azurirati susjede novih trapeza
    newLeft->setUpperR(newTop);
    newLeft->setLowerR(newBottom);
    newRight->setUpperL(newTop);
    newRight->setLowerL(newBottom);

    newTop->setOneLeft(newLeft);
    newTop->setOneRight(newRight);
    newBottom->setOneLeft(newLeft);
    newBottom->setOneRight(newRight);

    trapNode->getTrapezoid()->changeLeftWithGiven(newLeft);
    trapNode->getTrapezoid()->changeRightWithGiven(newRight);

    //azurirati searchStructure

    SearchNode *newRoot = new XNode(segment->_segment.p1());
    newRoot->addLeft(new LeafNode(newLeft));

    SearchNode *rightX = new XNode(segment->_segment.p2());
    newRoot->addRight(rightX);
    rightX->addRight(new LeafNode(newRight));

    SearchNode* leftY = new YNode(segment);
    rightX->addLeft(leftY);
    leftY->addLeft(new LeafNode(newTop));
    leftY->addRight(new LeafNode(newBottom));

    if(trapNode == _root)
        _root = newRoot;
    else trapNode->replaceNode(newRoot);

    delete trapNode;

}

    //ova funkcija vraca sljedeci trapez u odnosu na trapez trap
    //u "nizu" trapeza koje sijece duz segment
Trapezoid* getNext(Segment * segment, Trapezoid * trap) {
        Trapezoid* trapNext;

        if(trap->upperR() != nullptr) {

            if(trap->lowerR() != nullptr) {

            QPointF point =
                    segment->pointWithX(trap->upperR()->leftP().x());
            if(trap->upperR()->bottom()->isAbove(point, segment->_segment.p1()))
                trapNext = trap->upperR();
            else
                trapNext = trap->lowerR();

            }
            else
                trapNext = trap->upperR();
        }

        else
            trapNext = trap->lowerR();

        return trapNext;
}


void TrapezoidMap::generalCase(SearchNode *leftMost, SearchNode *rightMost, Segment *segment)
{

    Trapezoid* trapFirst = leftMost->getTrapezoid();
    Trapezoid* trapLast = rightMost->getTrapezoid();
    std::vector<Trapezoid*> toDelete;

    //pravimo prvi novi trapez
    Trapezoid *leftTrap = new Trapezoid(*trapFirst);
    leftTrap->setRightP(segment->_segment.p1());

    //pravimo i 2 nova trapeza, jedan ispod duzi, jedan iznad duzi
    //oni nisu zavrseni, mozda ce se spajati sa nekima, zato ih zovemo toMerge
    Trapezoid *toMergeTop = new Trapezoid(*trapFirst);
    toMergeTop->setLeftP(segment->_segment.p1());
    toMergeTop->setBottom(segment);
    SearchNode *leafTop = new LeafNode(toMergeTop);

    Trapezoid *toMergeBottom = new Trapezoid(*trapFirst);
    toMergeBottom->setLeftP(segment->_segment.p1());
    toMergeBottom->setTop(segment);
    SearchNode *leafBottom = new LeafNode(toMergeBottom);

    //azuriramo susjede
    leftTrap->setLowerR(toMergeBottom);
    leftTrap->setUpperR(toMergeTop);
    trapFirst->changeLeftWithGiven(leftTrap);
    toMergeBottom->setOneLeft(leftTrap);
    toMergeTop->setOneLeft(leftTrap);

    //azuriramo searchStructure
    SearchNode* newLeftX = new XNode(segment->_segment.p1());
    SearchNode* newYNode = new YNode(segment);

    newLeftX->addLeft(new LeafNode(leftTrap));
    newLeftX->addRight(newYNode);
    newYNode->addLeft(leafTop);
    newYNode->addRight(leafBottom);

    Trapezoid* trapPrev = trapFirst;
    Trapezoid* trapCurr = getNext(segment, trapFirst);
    trapFirst->searchNode()->replaceNode(newLeftX);
    toDelete.push_back(trapFirst);

    while(true) {
        bool makeNewTop = false;
        if(trapPrev->lowerR() && trapPrev->upperR())
            makeNewTop = (trapPrev->lowerR() == trapCurr);
        else
            makeNewTop = (trapCurr->lowerL() == trapPrev);

        if(makeNewTop) {
            toMergeTop->setRightP(trapCurr->leftP());
            Trapezoid *oldMergeTop = toMergeTop;
            toMergeTop = new Trapezoid(*trapCurr);
            leafTop = new LeafNode(toMergeTop);
            toMergeTop->setBottom(segment);

            //
            if(trapCurr->lowerL() && trapCurr->upperL()) {
                oldMergeTop->setOneRight(toMergeTop);
                toMergeTop->setLowerL(oldMergeTop);
                toMergeTop->setUpperL(trapCurr->upperL());
                trapCurr->upperL()->setOneRight(toMergeTop);

            }
            else {
                oldMergeTop->setLowerR(toMergeTop);
                oldMergeTop->setUpperR(trapPrev->upperR());
                trapPrev->upperR()->setOneLeft(oldMergeTop);
                toMergeTop->setOneLeft(oldMergeTop);

            }
            //
        }
        //end if(makeNewTop)

        else {

            toMergeBottom->setRightP(trapCurr->leftP());
            Trapezoid* oldMergeBottom = toMergeBottom;
            toMergeBottom = new Trapezoid(*trapCurr);
            leafBottom = new LeafNode(toMergeBottom);
            toMergeBottom->setTop(segment);

            //
            if(trapCurr->lowerL() && trapCurr->upperL()) {

                oldMergeBottom->setOneRight(toMergeBottom);
                toMergeBottom->setUpperL(oldMergeBottom);
                toMergeBottom->setLowerL(trapCurr->lowerL());
                trapCurr->lowerL()->setOneRight(toMergeBottom);

            }
            else {

                oldMergeBottom->setUpperR(toMergeBottom);
                oldMergeBottom->setLowerR(trapPrev->lowerR());
                trapPrev->lowerR()->setOneLeft(oldMergeBottom);
                toMergeBottom->setOneLeft(oldMergeBottom);
            }
            //

        }
        //end else for if(makeNewTop)

        if(trapCurr == trapLast)
            break;

        newYNode = new YNode(segment);
        newYNode->addLeft(leafTop);
        newYNode->addRight(leafBottom);
        trapCurr->searchNode()->replaceNode(newYNode);

        toDelete.push_back(trapCurr);
        trapPrev = trapCurr;
        trapCurr = getNext(segment, trapCurr);

    }
    //end while(true)

    //azurirati posljednji trapez koji duz sijece

    toMergeTop->setRightP(segment->_segment.p2());
    toMergeBottom->setRightP(segment->_segment.p2());

    Trapezoid* rightTrap = new Trapezoid(*trapLast);
    rightTrap->setLeftP(segment->_segment.p2());

    //azuriranje susjeda
    toMergeTop->setOneRight(rightTrap);
    toMergeBottom->setOneRight(rightTrap);
    rightTrap->setUpperL(toMergeTop);
    rightTrap->setLowerL(toMergeBottom);
    trapLast->changeRightWithGiven(rightTrap);

    //azuriranje searchStructure

    SearchNode* newRightX = new XNode(segment->_segment.p2());
    newYNode = new YNode(segment);
    newRightX->addRight(new LeafNode(rightTrap));
    newRightX->addLeft(newYNode);

    newYNode->addLeft(leafTop);
    newYNode->addRight(leafBottom);
    trapLast->searchNode()->replaceNode(newRightX);
    toDelete.push_back(trapLast);

}


const Trapezoid* TrapezoidMap::pointLocation(const QPointF& p) {
    return query(p,p)->getTrapezoid();
}

void TrapezoidMap::runAlgorithm()
{
    _segmentsAdded = 0;


    if(_status == AlgorithmStatus::INVALID_INPUT) {
        emit animationFinished();
        return;
    }

    if(_ready)
       makeEmpty();
    _ready = true;

    //bounding box
    Trapezoid* boundingBox = new Trapezoid;
    boundingBox->setLeftP(QPointF(MINX, MINY));
    boundingBox->setRightP(QPointF(MAXX, MAXY));


    int size = _segments.size();
    boundingBox->setTop(&_segments[size-1]);
    boundingBox->setBottom(&_segments[size-2]);

    _root = new LeafNode(boundingBox);

    std::vector<Segment> segs;
    std::copy(std::begin(_segments), std::end(_segments)-2, std::back_inserter(segs));

    for(auto& segment : segs) {
        _nextSeg = true;
        AlgorithmBase_updateCanvasAndBlock();
        addSegment(&segment);
        _segmentsAdded++;
        _nextSeg = false;
        if(_destroyAnimation)
            return;
        AlgorithmBase_updateCanvasAndBlock();

    }

    _locatingPoint = true;

    const Trapezoid* contains = pointLocation(_pointLocation);

    _fieldWithPoint = contains->bottom()->_fieldName;

    _locatingPoint = false;

    emit animationFinished();

}


void TrapezoidMap::drawAlgorithm(QPainter &painter) const
{
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPen cyan;
    cyan.setColor(Qt::cyan);
    cyan.setWidth(7);

    QPen green;
    green.setColor(Qt::green);
    green.setWidth(6);

    QPen blue;
    blue.setColor(Qt::blue);
    blue.setWidth(5);

    QPen red;
    red.setColor(Qt::red);
    red.setWidth(3);

    QPen black;
    black.setColor(Qt::black);
    black.setWidth(3);

    QPen yellow;
    yellow.setColor(Qt::yellow);
    yellow.setWidth(6);

    painter.setPen(black);

    std::vector<Trapezoid*> trapezoids;
    getTrapezoids(trapezoids);


    for(Trapezoid* trapezoid : trapezoids) {
        trapezoid->draw(painter);
        painter.setPen(yellow);
        painter.drawPoint(trapezoid->leftP().x(), trapezoid->leftP().y());
        painter.drawPoint(trapezoid->rightP().x(), trapezoid->rightP().y());
        painter.setPen(black);
    }

    painter.setPen(red);

    if(_nextSeg) {
        painter.drawLine(_segments[_segmentsAdded]._segment);
        painter.setPen(blue);
        painter.drawPoint(_segments[_segmentsAdded]._segment.p1());
        painter.drawPoint(_segments[_segmentsAdded]._segment.p2());
    }


    painter.setPen(cyan);
    if(_currentEndPoint)
        painter.drawPoint(_currentPointSearching);

    painter.setPen(green);

    if(_currentNode && _currentNodeSearching != nullptr)
        _currentNodeSearching->draw(painter);

    if(_locatingPoint) {
        QPen magenta;
        magenta.setColor(Qt::magenta);
        magenta.setWidth(6);
        painter.setPen(magenta);

        painter.drawPoint(_pointLocation);
    }

}


void TrapezoidMap::runNaiveAlgorithm()
{

    std::vector<qreal> verticalSlabs;
    std::set<qreal> slabsUnique;

    for(Segment s: _segments) {
        slabsUnique.insert(s._segment.p1().x());
        slabsUnique.insert(s._segment.p2().x());
    }
    std::copy(std::begin(slabsUnique), std::end(slabsUnique),
              std::back_inserter(verticalSlabs));

    int slabsNum = verticalSlabs.size() - 1;

    std::vector<std::vector<Segment>> segmentsInSlab(slabsNum);
    for(int i = 0; i < slabsNum; i++)
        for(Segment s : _segments)
            if(s._segment.p1().x() <= verticalSlabs[i] &&
                  s._segment.p2().x() >= verticalSlabs[i+1])
                segmentsInSlab[i].push_back(s);

    for(int i = 0; i < slabsNum; i++)
        std::sort(segmentsInSlab[i].begin(), segmentsInSlab[i].end(),
                  [&verticalSlabs, i](Segment& x, Segment& y) {
           qreal midX = (verticalSlabs[i] + verticalSlabs[i+1])/2;
           return x.pointWithX(midX).y() < y.pointWithX(midX).y();
        });


    auto it = std::find_if(std::begin(verticalSlabs), std::end(verticalSlabs),
                [this](qreal elem) {return elem > _pointLocation.x(); } );


    if(it == std::end(verticalSlabs)) {
        _fieldWithPoint = "0F";
        return;
    }

    int indexVertical = it - std::begin(verticalSlabs) - 1;

    auto it1 = std::find_if(std::begin(segmentsInSlab[indexVertical]), std::end(segmentsInSlab[indexVertical]),
        [this](const Segment& elem) {
           return elem.pointWithX(_pointLocation.x()).y() > _pointLocation.y();});

    if(it1 == std::end(segmentsInSlab[indexVertical])) {
        _fieldWithPoint = "0F";
        return;
    }

    int indexHorizontal = it1 - std::begin(segmentsInSlab[indexVertical]) - 1;

    _fieldWithPoint = segmentsInSlab[indexVertical][indexHorizontal]._fieldName;

}

void TrapezoidMap::addSegment(Segment* segment) {
    _currentEndPoint = true;
    _currentPointSearching = segment->_segment.p1();
    AlgorithmBase_updateCanvasAndBlock();
    SearchNode *nodel = this->query(segment->_segment.p1(), segment->_segment.p2());

    if(_destroyAnimation)
        return;
    AlgorithmBase_updateCanvasAndBlock();

    _currentEndPoint = false;

    _currentEndPoint = true;
    _currentPointSearching = segment->_segment.p2();

    AlgorithmBase_updateCanvasAndBlock();

    SearchNode *noder = this->query(segment->_segment.p2(), segment->_segment.p1());

    if(_destroyAnimation)
        return;
    AlgorithmBase_updateCanvasAndBlock();
    _currentEndPoint = false;

    Trapezoid *t1 = nodel->getTrapezoid();
    Trapezoid *t2 = noder->getTrapezoid();

    if(t1 == t2)
        simpleCase(nodel, segment);
    else
        generalCase(nodel, noder, segment);

}

TrapezoidMap::AlgorithmStatus TrapezoidMap::getStatus() const
{
    return _status;
}

void TrapezoidMap::setStatus(const AlgorithmStatus &status)
{
    _status = status;
}


std::vector<Segment> TrapezoidMap::getSegments() const
{
    return _segments;
}

void TrapezoidMap::setSegments(const std::vector<Segment> &segments)
{
    _segments = segments;
}

std::string TrapezoidMap::getFieldWithPoint() const
{
    return _fieldWithPoint;
}

void TrapezoidMap::setFieldWithPoint(const std::string &fieldWithPoint)
{
    _fieldWithPoint = fieldWithPoint;
}

QPointF TrapezoidMap::getPointLocation() const
{
    return _pointLocation;
}

void TrapezoidMap::setPointLocation(const QPointF &pointLocation)
{
    _pointLocation = pointLocation;
}

