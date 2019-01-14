/*
Autor: Ozren Demonja
Godina: 2018
Kratak opis problema: Odredjivanje preseka duzi na pravoj koriscenjem strukture interval search tree
*/

#ifndef GA11_INTERVALSEARCHTREE_H
#define GA11_INTERVALSEARCHTREE_H

#include "algorithmbase.h"
#include <fstream>

/* Representing line with left and right x coordinate(low, high) */
class Interval{
public:
    Interval(int low, int high);

    bool operator==(const Interval other) const{
        return high == other.getHigh() && low == other.getLow();
    }

    int getLow() const;
    int getHigh() const;
private:
    int low, high;
};

class IntervalSearchTree : public AlgorithmBase{
public:
    IntervalSearchTree(QWidget* pRenderer, int delayMs, std::string inputFilename = "", int intervalSize = DEFAULT_LINES_NUM, int inputSize = DEFAULT_LINES_NUM);

    void runAlgorithm();
    void drawAlgorithm(QPainter &painter) const;
    void runNaiveAlgorithm();

    void insert(const Interval &line);
    void findOverlap();

    /* Setters for line and intervals (used only for testing) */
    void setLine(const Interval &line);
    void setInputLines(std::vector<Interval> &inputlLine);
    void setLineIntervals(std::vector<Interval> &intervalLine);

    /* Getters for naive and optimal result (used only for testing) */
    std::vector<Interval> getOverlapVector();
    std::vector<Interval> getNaiveOverlapVector();

    ~IntervalSearchTree(){
       deleteTree(root);
    }

    /* Node of red black tree with maximum high value in subtree rooted with this node */
    class Node {
    public:
        enum Color {RED, BLACK};

        Node(const Color color, const Interval &line, Node *parent)
            : color(color), line(line), parent(parent),
              left(nullptr), right(nullptr)
        {
            max = line.getHigh();
        }

        Color color;
        Interval line;
        int max;

        Node *parent;
        Node *left;
        Node *right;
    };

private:
    Node *root;                               //tree root
    std::vector<Interval> inputLineVector;    //input lines
    std::vector<Interval> overlapVector;      //optimal algorithm result
    std::vector<Interval> naiveOverlapVector; //naive algorithm result
    std::vector<Interval> intervalLineVector; //lines for which will be checked overlap
    Node *currentSearchNode;                  //only used for animation current search node
    size_t currentIntervalNum;                //only used for animation current number in vector
    static int constexpr DRAWING_BORDER = 10;

    /* Generate random intervals and lines */
    void generateRandomLines(int intervalNum, int linesNum);

    /* Read intervals and lines from files in format
     * n1       - represent start of lines
     * x1 x2    - line given by low and high x value
     * ...
     * n2       - represent start of intervals
     * x1 x2
     * ...
     */
    void readLinesFromFile(std::string fileName);

    /* Check if is given properly number of intervals and lines */
    bool checkAlgorithmCondition() const;

    /* A recursive algorithm that insert new line in tree
     *  node    - node use to navigate thru tree
     *  parent  - parent of new node
     *  line    - line that need to be insert in tree
     */
    void insert(Node *node, Node *parent, const Interval &line);

    /* A recursive algorithm that delete tree
     *  root    - root of subtree that will be deleted
     */
    void deleteTree(Node *root);

    /* A recursive algorithm that try to find overlap in tree
     *  line    - line for witch we search segments that overlap
     *  root    - root of subtree where is search overlap
     */
    inline void findOverlap(const Interval &line, Node *root);

    /* Check whether line1 and line2 overlap */
    inline bool existOverlap(const Interval &line1, const Interval &line2) const;

    /* Check if node is black (node is null ptr or node color are black) */
    bool isNodeBlack(const Node *node) const;

    /* Handle double red problem after insert new node */
    void fixInsertTree(Node *child);

    /* Double rotation subtree rooted with grandparent
     * grandparent - node used to rotate around in second rotation
     * leftRotation - rotate first over left or right grandparent son
     */
    void doubleRotation(Node *grandparent, bool leftRotation);

    /* Left rotate subtree rooted with parent */
    void rotateLeft(Node *parent);

    /* Right rotate subtree rooted with parent */
    void rotateRight(Node *parent);

    /* Update maximum high value in subtree rooted with node after rotation is finish */
    void updateMaxNode(Node *node);

    /* Update color of node, node sibling and node parent after rotation is finish */
    void rotationFixColor(Node *node);

    /* Update conection between new parent and child
     * parent       - new parent for given child
     * child        - new child for given parent
     * setLeftChild - connect child as left or right son
     */
    void reconectChild(Node *parent, Node *child, bool setLeftChild);

    /* Return the node sibling */
    Node* getSibling(const Node *child);

    /* A recursive algorithm that used to draw tree
     * painter   - a reference to QPainter object
     * x         - x position of subtree rooted with root
     * y         - y position of subtree rooted with root
     * root      - new current root of tree
     */
    void drawTree(QPainter &painter, int x, int y, Node *root) const;

    /* Draw line and print on ends x value
     * painter   - a reference to QPainter object
     * line      - line for draw
     * y         - y-coordinate of a line
     */
    void drawLine(QPainter &painter, const Interval &line, int y) const;
};

#endif // GA11_INTERVALSEARCHTREE_H
