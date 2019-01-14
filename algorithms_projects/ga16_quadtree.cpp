#include "ga16_quadtree.h"
Quadtree *Node::parent;

Quadtree::Quadtree(QWidget *pRenderer, int delayMs, std::string filename, int inputSize)
    : AlgorithmBase(pRenderer, delayMs), collider(nullptr)
{
    std::vector<QPoint> points_;
    if(filename == "")
        points_ = generateRandomPoints(inputSize);
    else
        points_ = readPointsFromFile(filename);
    if (pRenderer){
        squareSize = _pRenderer->height() / 30;
        Bounds b(0, 0, _pRenderer->width(), pRenderer->height());
        root = new Node(b, 1);
    } else {
        squareSize = 1;
        Bounds b(0, 0, 1200, 500);
        root = new Node(b, 1);
    }

    Node::parent = this;
    setPoints(points_);
}

void Quadtree::setPoints(const std::vector<QPoint> &points_)
{
    points = points_;
    allItems.resize(0);
    allItems.reserve(points.size());
    hadCollision.resize(points.size());
    clearResult();

    int i = 0;
    for (QPoint &p : points){
        allItems.push_back(new Item(p.x(), p.y(), squareSize, squareSize, i++));
    }
}

void Quadtree::runAlgorithm()
{
    clearResult();
    // building a quadtree
    for (Item * item : allItems) { 
        root->insert(item, true);

        collider = item;
        AlgorithmBase_updateCanvasAndBlock();
    }

    emit animationFinished();
}

void Quadtree::drawAlgorithm(QPainter &painter) const
{
    root->drawSelf(painter);
    if (!collider)
        return;
    std::vector<Item*> collisionCandidates = root->retrieve(collider);
    // draw an item for which we are checking collision
    QPen p;
    p.setWidth(20);
    p.setColor(Qt::GlobalColor::green);
    painter.setPen(p);

    painter.fillRect(collider->x, collider->y, collider->h, collider->w,
                     Qt::GlobalColor::green);

    // draw all items that may be colliding with collider
    painter.setPen(p);
    for (auto c : collisionCandidates){
        if (c == collider)
            continue;
        if (collider->hasCollision(c))
            painter.fillRect(c->x, c->y, c->h, c->w, Qt::GlobalColor::red);
        else
            painter.fillRect(c->x, c->y, c->h, c->w, Qt::GlobalColor::yellow);
    }

    // draw number of colliding items
    p.setColor(Qt::GlobalColor::black);
    p.setWidth(10);
    painter.setPen(p);

    painter.fillRect(0, 0, 200, 35, Qt::GlobalColor::gray);
    painter.drawText(10, 25, "Kvadrata u koliziji: " + QString::number( result() ));
}

void Quadtree::getDepthColor(int depth, int &red, int &green, int &blue)
{
    switch(depth){
        case 1:
           red = 0, green = 10, blue = 200;
        break;
        case 2:
           red = 0, green = 200, blue = 100;
        break;
        case 3:
           red = 220, green = 220, blue = 40;
        break;
        case 4:
            red = 244, green = 111, blue = 11;
        break;
        case 5:
            red = 200, green = 11, blue = 11;
        default:
            red = 0, green = 0, blue = 0;
        break;
    }
}


void Node::drawSelf(QPainter &painter) const
{
    QPen p;
    int r, g, b;
    if (depth == 1){
        parent->getDepthColor(depth, r, g, b);
        p.setColor(QColor::fromRgb(r, g, b));
        p.setWidth(Quadtree::lineWidth);
        painter.setPen(p);

        painter.drawRect(bounds.x, bounds.y, bounds.w, bounds.h);
    }


    p.setColor(QColor::fromRgb(10, 30, 33));
    p.setWidth(5);
    painter.setPen(p);

    // draw children
    for (Item *c : children){
        painter.drawRect(c->x, c->y, c->w, c->h);
        // painter.drawPoint(c->x, c->y);
    }

    if (!isLeafNode){
        // recursively draw child subnodes
        for (int i=0; i<NUM_OF_SUBNODES; i++){
            nodes[i]->drawSelf(painter);
        }
        // draw inner bounds
        p.setWidth(Quadtree::lineWidth / (depth + 1 ));
        parent->getDepthColor(depth, r, g, b);
        p.setColor(QColor::fromRgb(r, g, b));
        painter.setPen(p);

        painter.drawLine(bounds.x, bounds.y + bounds.h / 2,
                         bounds.x + bounds.w,  bounds.y + bounds.h / 2);
        painter.drawLine(bounds.x + bounds.w / 2, bounds.y,
                         bounds.x + bounds.w / 2,  bounds.y + bounds.h);
    }
}

void Quadtree::runNaiveAlgorithm()
{
    clearResult();
    for(unsigned i = 0; i < allItems.size(); i++){
        for (unsigned j = i + 1; j < allItems.size(); j++){
            Item *a = allItems[i];
            Item *b = allItems[j];

            if (a->hasCollision(b)){
                addCollision(a, b);
            }
        }
    }
}


Node::Node(Bounds &bounds, int depth) : bounds(bounds), depth(depth), isLeafNode(true)
{}

Node::Node(int x, int y, int width, int height, int depth) : bounds(x, y, width, height),
    depth(depth), isLeafNode(true)
{}

std::vector<Item*> Node::retrieve(Item *item) const
{
    std::vector<Item*> result;
    retrieve(item, result);
    return result;
}

void Node::retrieve(Item *item, std::vector<Item*> &result) const
{
    if(isLeafNode){
        result.insert(result.end(), children.begin(), children.end());
        return;
    }
    std::vector <Node::Quadrant> qu = findQuadrants(item);
    for (auto q : qu){
        nodes[q]->retrieve(item, result);
    }
}

void Node::insert(Item *item, bool count)
{
    if (isLeafNode){
        if (depth < Quadtree::maxDepth && children.size() >= Quadtree::maxChildren){
            split();
            for (auto c : children){
                insert(c, false);
            }
            children.resize(0);
            insert(item, count);
        } else {
            if (count){
                for(auto c : children){
                    if (item->hasCollision(c)){
                        parent->addCollision(item, c);
                    }
                }
            }
            children.push_back(item);
        }
    } else {
        std::vector<Quadrant> qu = findQuadrants(item);
        for (auto q : qu){
            nodes[q]->insert(item, count);
        }
    }
}

void Node::split()
{
    isLeafNode = false;
    int width =  bounds.w / 2;
    int height = bounds.h / 2;
    // these two are needed in case bounds w/h are odd
    int rightWidth = bounds.w - width;
    int upHeight = bounds.h - height;
    int xMid = bounds.x + width;
    int yMid = bounds.y + height;

    nodes[UpRight] = new Node(xMid, bounds.y, rightWidth, upHeight, depth + 1);
    nodes[UpLeft] = new Node(bounds.x, bounds.y, width, upHeight, depth + 1);
    nodes[DownRight] = new Node(xMid, yMid, rightWidth, height, depth + 1);
    nodes[DownLeft] = new Node(bounds.x, yMid, width, height, depth + 1);
}

std::vector<Node::Quadrant> Node::findQuadrants(Item *item) const
{
    int midX = bounds.x + bounds.w / 2;
    int midY = bounds.y + bounds.h / 2;
    bool right = midX <= item->x + item->w;
    bool left = midX >= item->x;
    bool up = midY >= item->y;
    bool down = midY <= item->y + item->h;
    std::vector<Quadrant> result;
    if (right){
        if (up)
            result.push_back(Quadrant::UpRight);
        if (down)
            result.push_back(Quadrant::DownRight);
    }
    if (left){
        if (up)
            result.push_back(Quadrant::UpLeft);
        if (down)
            result.push_back(Quadrant::DownLeft);
    }

    return result;
}

bool Item::hasCollision(Item *i)
{
    return x <= i->x + i->w && x + w >= i->x
            && y <= i->y + i->h && y + h >= i->y;
}

void Quadtree::addCollision(Item *a, Item *b)
{
    // collisions.insert(std::make_pair(std::make_pair(a, b), 0));
    // collisions.insert(std::make_pair(a,b));
    hadCollision[a->number] = true;
    hadCollision[b->number] = true;
}

void Quadtree::setSquareSize(int size)
{
    squareSize = size;
    setPoints(points);
}

void Quadtree::clearResult()
{
    std::fill(hadCollision.begin(), hadCollision.end(), false);
}

// returns the number of items that are colliding
int Quadtree::result() const
{
    return std::count(hadCollision.begin(), hadCollision.end(), true);
}
