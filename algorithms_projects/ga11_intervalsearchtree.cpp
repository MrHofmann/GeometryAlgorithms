#include "ga11_intervalsearchtree.h"

#define K_MOVE 20
#define rx 45
#define ry 25
#define Y_MOVE 80
#define X_DOUBLE_RL 170
#define X_RLEFT 70
#define X_ROOT 200

IntervalSearchTree::IntervalSearchTree(QWidget* pRenderer, int delayMs, std::string inputFilename, int intervalSize, int inputSize)
    : AlgorithmBase(pRenderer, delayMs), root(nullptr), overlapVector({}), naiveOverlapVector({})
{
    if(inputFilename == ""){
        generateRandomLines(intervalSize, inputSize);
    }
    else{
        readLinesFromFile(inputFilename);
    }
    currentIntervalNum = -1;
}

void IntervalSearchTree::runAlgorithm(){
    if(!checkAlgorithmCondition()){
        emit animationFinished();
        return;
    }

    //Initialize structure
    for(auto const &line: inputLineVector) {
        //If animation thread are destroy exit
        if(_destroyAnimation)
            return;

        this->insert(line);

        AlgorithmBase_updateCanvasAndBlock();
    }

    //Find all overlaps of intervals and lines
    this->findOverlap();

    //If animation thread are destroy exit
    if(_destroyAnimation)
        return;
    AlgorithmBase_updateCanvasAndBlock();

    emit animationFinished();
}

void IntervalSearchTree::drawAlgorithm(QPainter &painter) const{
    //Set font and start y point (k)
    painter.setFont(QFont("times",11));
    size_t k = 20;

    //Draw interval lines
    QPen pen(Qt::red);
    pen.setWidth(2);
    painter.setPen(pen);
    for(size_t i = 0; i < intervalLineVector.size(); i++){
        //Green color for current active line
        if(currentIntervalNum == i){
            pen.setColor(Qt::green);
            painter.setPen(pen);
            drawLine(painter, intervalLineVector[i], k);
            pen.setColor(Qt::red);
            painter.setPen(pen);
        }
        else
            drawLine(painter, intervalLineVector[i], k);
        k += K_MOVE;
    }

    //Draw input lines
    pen.setColor(Qt::gray);
    painter.setPen(pen);
    for(auto const &line : inputLineVector){
        //Blue color for overlap lines
        if(std::find(overlapVector.begin(), overlapVector.end(), line) != overlapVector.end()){
            pen.setColor(Qt::blue);
            painter.setPen(pen);
            drawLine(painter, line, k);
            pen.setColor(Qt::gray);
            painter.setPen(pen);
        }
        else{
            drawLine(painter, line, k);
        }
        k += K_MOVE;
    }

    pen.setWidth(1);
    painter.setPen(pen);
    drawTree(painter, painter.device()->width() / 2, k + Y_MOVE, this->root);
}

void IntervalSearchTree::drawLine(QPainter &painter, const Interval &line, int y) const{
    QString nodeStr = QStringLiteral("%1").arg(line.getLow());
    painter.drawText(line.getLow() - 30, y + 5, nodeStr);
    nodeStr = QStringLiteral("%1").arg(line.getHigh());
    painter.drawText(line.getHigh() + 5, y + 5, nodeStr);
    painter.drawLine(QLineF(line.getHigh(), y, line.getLow(), y));
}

void IntervalSearchTree::drawTree(QPainter &painter, int x, int y, Node *root) const{
    if(root == nullptr)
        return;

    //Draw lines between connected nodes
    if(root->parent != nullptr && root->parent->left == root){
        if(root->left != nullptr)
            painter.drawLine(QPoint(x, y), QPoint(x - X_DOUBLE_RL, y + Y_MOVE));
        if(root->right != nullptr)
            painter.drawLine(QPoint(x, y), QPoint(x + X_RLEFT, y + Y_MOVE));
    }
    else if (root->parent == nullptr){
        if(root->left != nullptr)
            painter.drawLine(QPoint(x, y), QPoint(x - X_ROOT, y + Y_MOVE));
        if(root->right != nullptr)
            painter.drawLine(QPoint(x, y), QPoint(x + X_ROOT, y + Y_MOVE));
    }
    else{
        if(root->left != nullptr)
            painter.drawLine(QPoint(x, y), QPoint(x - X_RLEFT, y + Y_MOVE));
        if(root->right != nullptr)
            painter.drawLine(QPoint(x, y), QPoint(x + X_DOUBLE_RL, y + Y_MOVE));
    }

    //Paint current analize node with different color
    if(currentSearchNode == root){
        painter.setBrush(QColor(226, 226, 226));
    }
    else{
        painter.setBrush(QColor(246, 246, 246));
    }

    //Draw node
    QRect rec(x - rx, y - ry, 2 * rx, 2 * ry);
    //Bold overlap nodes
    if(std::find(overlapVector.begin(), overlapVector.end(), root->line) != overlapVector.end()){
        QPen pen(Qt::red);
        pen.setWidth(3);
        painter.setPen(pen);
    }
    painter.drawEllipse(rec);
    painter.setPen(QPen(Qt::gray));
    QString nodeStr = QStringLiteral("(%1, %2)").arg(root->line.getLow()).arg(root->line.getHigh());
    painter.drawText(rec, Qt::AlignCenter, nodeStr);
    painter.drawText(x, y-30, QStringLiteral("%1").arg(root->max));

    if(root->parent != nullptr && root->parent->left == root){
        drawTree(painter, x - X_DOUBLE_RL, y + Y_MOVE, root->left);
        drawTree(painter, x + X_RLEFT, y + Y_MOVE, root->right);
    }
    else if (root->parent == nullptr){
        drawTree(painter, x - X_ROOT, y + Y_MOVE, root->left);
        drawTree(painter, x + X_ROOT, y + Y_MOVE, root->right);
    }
    else {
        drawTree(painter, x - X_RLEFT, y + Y_MOVE, root->left);
        drawTree(painter, x + X_DOUBLE_RL, y + Y_MOVE, root->right);
    }
}

void IntervalSearchTree::runNaiveAlgorithm(){
    if(!checkAlgorithmCondition()){
        return;
    }

    for(auto const &interval: intervalLineVector) {
        for(auto const &line: inputLineVector) {
            if (interval.getLow() <= line.getHigh() && interval.getHigh() >= line.getLow()){
                naiveOverlapVector.push_back(line);
            }
        }
    }
}

bool IntervalSearchTree::checkAlgorithmCondition() const{
    if(intervalLineVector.size() < 1 || inputLineVector.size() < 1){
        qWarning("Nisu uneti potrebni podaci");
        return false;
    }
    return true;
}

bool IntervalSearchTree::isNodeBlack(const Node *node) const{
    return node == nullptr || node->color == Node::BLACK;
}

void IntervalSearchTree::deleteTree(Node *root){
    if (root == nullptr)
        return;

    deleteTree(root->left);
    deleteTree(root->right);

    delete root;
}

void IntervalSearchTree::insert(const Interval &line) {
    if (root == nullptr) {
        root = new Node(Node::BLACK, line, nullptr);
    }
    else {
        insert(root, nullptr, line);
    }
}

void IntervalSearchTree::insert(Node *node, Node *parent, const Interval &line){
    if(node == nullptr){
        Node *newNode = new Node(Node::RED, line, parent);
        if (line.getLow() < parent->line.getLow())
            parent->left = newNode;
        else
            parent->right = newNode;

        fixInsertTree(newNode);
    }
    else{
        node->max = node->max > line.getHigh() ? node->max : line.getHigh();
        if (line.getLow() < node->line.getLow()){
            insert(node->left, node, line);
        }
        else{
            insert(node->right, node, line);
        }
    }
}

IntervalSearchTree::Node* IntervalSearchTree::getSibling(const Node *child){
    Node *parent = child->parent;

    if (parent == nullptr){
        return nullptr;
    }
    else if (parent->right == child){
        return parent->left;
    }
    else {
        return parent->right;
    }
}

void IntervalSearchTree::fixInsertTree(Node *child) {
    if (isNodeBlack(child->parent) || isNodeBlack(child)) {
        return;
    }

    Node *parent = child->parent;
    Node *uncle = getSibling(parent);
    Node *grandparent = parent->parent;

    //Set parent and uncle color on black
    if (!isNodeBlack(uncle)) {
        parent->color = Node::BLACK;
        uncle->color = Node::BLACK;

        //Switch grandparent color and makes a recursive call if grandparent is not root
        if (grandparent->parent != nullptr) {
            grandparent->color = Node::RED;
            fixInsertTree(grandparent);
        }
    }
    else if (grandparent->left == parent) {
        //If parent is left son of grandparent and child is right son of parent than we need double left rotation
        if (parent->right == child) {
            doubleRotation(grandparent, true);
            updateMaxNode(child);
        }
        else{
            rotateRight(grandparent);
            updateMaxNode(grandparent);
            updateMaxNode(parent);
        }
        rotationFixColor(grandparent);
    }
    else {
        //If parent is right son of grandparent and child is left son of parent than we need double right rotation
        if (parent->left == child) {
            doubleRotation(grandparent, false);
            updateMaxNode(child);
        }
        else{
            rotateLeft(grandparent);
            updateMaxNode(grandparent);
            updateMaxNode(parent);
        }
        rotationFixColor(grandparent);
    }
}

void IntervalSearchTree::doubleRotation(Node *grandparent, bool leftRotation){
        Node *parent;
        if(leftRotation){
            parent = grandparent->left;
            rotateLeft(parent);
            rotateRight(grandparent);
        }
        else {
            parent = grandparent->right;
            rotateRight(parent);
            rotateLeft(grandparent);
        }
        updateMaxNode(grandparent);
        updateMaxNode(parent);
}

void IntervalSearchTree::updateMaxNode(Node *node){
    int leftMax = node->left == nullptr ? -1 : node->left->max;
    int rightMax = node->right == nullptr ? -1 : node->right->max;
    int currentMax = node->line.getHigh();
    node->max = std::max(std::max(currentMax, leftMax), rightMax);
}

void IntervalSearchTree::rotationFixColor(Node *node){
    node->color = Node::RED;
    getSibling(node)->color = Node::RED;
    node->parent->color = Node::BLACK;
}

void IntervalSearchTree::reconectChild(Node *parent, Node *child, bool setLeftChild){
    if(child != nullptr)
        child->parent = parent;

    if(setLeftChild){
        parent->left = child;
    }
    else {
        parent->right = child;
    }
}

void IntervalSearchTree::rotateLeft(Node *parent){
    Node *child = parent->right;
    Node *grandparent = parent->parent;

    //If exist grandparent connect grandparent and child
    if (grandparent != nullptr){
        if (parent == grandparent->right){
            reconectChild(grandparent, child, false);
        }
        else
            reconectChild(grandparent, child, true);
    }
    //If don't exist grandparent set child as root
    else{
        root = child;
        if (child != nullptr)
            child->parent = nullptr;
    }

    //Connect parent to left son of child
    reconectChild(parent, child->left, false);

    //Rotate child and parent so child become parent and parent left child
    reconectChild(child, parent, true);
}

void IntervalSearchTree::rotateRight(Node *parent){
    Node *child = parent->left;
    Node *grandparent = parent->parent;

    //If exist grandparent connect grandparent and child
    if (grandparent != nullptr){
        if (parent == grandparent->left)
            reconectChild(grandparent, child, true);
        else
            reconectChild(grandparent, child, false);
    }
    //If don't exist grandparent set child as root
    else{
        root = child;
        if (child != nullptr)
            child->parent = nullptr;
    }

    //Connect parent to right son of child
    reconectChild(parent, child->right, true);

    //Rotate child and parent so child become parent and parent child
    reconectChild(child, parent, false);
}

void IntervalSearchTree::setLine(const Interval &line){
    intervalLineVector.push_back(line);
}

void IntervalSearchTree::findOverlap(){
    for(size_t i = 0; i < intervalLineVector.size(); i++) {
        //If animation thread are destroy exit
        if(_destroyAnimation)
            return;

        currentIntervalNum = i;
        AlgorithmBase_updateCanvasAndBlock();

        findOverlap(intervalLineVector[i], root);

        currentSearchNode = nullptr;
    }
    currentIntervalNum = -1;
}

inline void IntervalSearchTree::findOverlap(const Interval &line, Node *root){
    if(root == nullptr || _destroyAnimation)
        return;

    currentSearchNode = root;
    AlgorithmBase_updateCanvasAndBlock();

    //If exist overlap add line in result vector
    if(existOverlap(line, root->line)){
        overlapVector.push_back(root->line);
    }

    bool tmp = true;

    //Go left if exist left child and left child max is smaller than interval low x value
    if(root->left != nullptr && root->left->max >= line.getLow()){
        int x = overlapVector.size();
        findOverlap(line, root->left);
        x -= overlapVector.size();
        tmp = (x < 0)? true : false;
    }
    //Go right if dont exist left child or in left subtree is find overlap
    if(tmp){
        findOverlap(line, root->right);
    }
}

inline bool IntervalSearchTree::existOverlap(const Interval &line1, const Interval &line2) const{
    return line1.getLow() <= line2.getHigh() && line1.getHigh() >= line2.getLow();
}

void IntervalSearchTree::setLineIntervals(std::vector<Interval> &intervalLine){
    intervalLineVector = intervalLine;
}

void IntervalSearchTree::setInputLines(std::vector<Interval> &inputlLine){
    inputLineVector = inputlLine;
}

std::vector<Interval> IntervalSearchTree::getOverlapVector(){
    return overlapVector;
}

std::vector<Interval> IntervalSearchTree::getNaiveOverlapVector(){
    return naiveOverlapVector;
}

void IntervalSearchTree::generateRandomLines(int intervalNum, int linesNum){
    srand(static_cast<unsigned>(time(0)));

    int xMax = 1200 - DRAWING_BORDER;
    int xMin = DRAWING_BORDER;
    int xDiff = xMax-xMin;

    for(int i=0; i < linesNum; i++){
        int x1 = xMin + rand()%xDiff;
        int x2 = xMin + rand()%xDiff;
        if(x1 < x2)
            inputLineVector.emplace_back(Interval(x1, x2));
        else
            inputLineVector.emplace_back(Interval(x2, x1));
    }

    for(int i=0; i < intervalNum; i++){
        int x1 = xMin + rand()%xDiff;
        int x2 = xMin + rand()%xDiff;
        if(x1 < x2)
            intervalLineVector.emplace_back(Interval(x1, x2));
        else
            intervalLineVector.emplace_back(Interval(x2, x1));
    }
}

void IntervalSearchTree::readLinesFromFile(std::string fileName){
    std::ifstream inputFile(fileName);
    int x1, x2;
    std::string str;
    bool ok;


    inputFile >> str;
    if(str == "n1"){
        while(inputFile >> str){
            x1 = QString::fromStdString(str).toInt(&ok);
            if(!ok || str == "n2")
                break;
            inputFile >> x2;
            inputLineVector.emplace_back(Interval(x1, x2));
        }
    }
    while(inputFile >> x1 >> x2){
        intervalLineVector.emplace_back(Interval(x1, x2));
    }
}

Interval::Interval(int low, int high) :
    low(low), high(high)
{}

int Interval::getLow() const{
    return low;
}

int Interval::getHigh() const{
    return high;
}
