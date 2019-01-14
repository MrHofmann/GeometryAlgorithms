/*
Autor: Nikola Dimitrijevic 1086/2017
Godina: 2017
Kratak opis problema: Dato je n kvadrata. Naci sve kvadrate koji su u koliziji.
*/
#ifndef GA16_QUADTREE_H
#define GA16_QUADTREE_H

#include "algorithmbase.h"
#include <QDebug>
#include <algorithm>

struct Bounds
{
public:
    Bounds(int x, int y, int width, int height) : x(x), y(y),
        w(width), h(height) {}
    int x;
    int y;
    int w;
    int h;
};

class Item : public Bounds
{
public:
    Item(int x, int y, int width, int height, int number) :
        Bounds(x, y, width, height), number(number) {}
    int number;
    bool hasCollision(Item *i);
};
class Quadtree;

class Node{
public:
    enum Quadrant{UpRight, DownRight, DownLeft, UpLeft};
    static const int NUM_OF_SUBNODES = 4;
    static Quadtree *parent;
    Node *nodes[NUM_OF_SUBNODES];
    Node(int x, int y, int width, int height, int depth);
    Node(Bounds &bounds, int depth);
    Bounds bounds;
    std::vector<Item*> retrieve(Item *item) const;
    std::vector<Node::Quadrant> findQuadrants(Item *item) const;
    void retrieve(Item *item, std::vector<Item*> &result) const;
    void insert(Item *item, bool count);
    void drawSelf(QPainter &painter) const;
    int depth;
private:
    void split();
    bool isLeafNode;
    std::vector<Item*> children;

};

class Quadtree : public AlgorithmBase
{
public:
    Quadtree(QWidget* pRenderer, int delayMs, std::string filename = "",
             int inputSize = DEFAULT_POINTS_NUM);
    // AlgorithmBase interface
public:
    void runAlgorithm();
    void drawAlgorithm(QPainter &painter) const;
    void runNaiveAlgorithm();
    static const int maxChildren = 2; // Max number of children a node can contain before it's split
    static const int maxDepth = 5; // Max number of levels a quadtree can create
    static const int lineWidth = 25;
    void getDepthColor(int depth, int &red, int &green, int &blue);
    void setPoints(const std::vector<QPoint> &points);
    void setSquareSize(int size);
    void addCollision(Item *a, Item *b);
    int result() const;
private:
    void clearResult();
    std::vector<bool> hadCollision; // if i-th element is colliding
    std::vector<QPoint> points; // algorithm input
    std::vector<Item*> allItems; // items made from points
    // std::set<std::pair<Item*, Item*>> collisions;
    Item *collider; // focused item on last draw
    Node *root; // Root node of a quadtree
    int squareSize = 30;

};

#endif // GA16_QUADTREE_H
