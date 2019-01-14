#ifndef GA24_UNIONOFRECTANGLES_H
#define GA24_UNIONOFRECTANGLES_H

#include "algorithmbase.h"

#include <vector>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <set>
#include "utils.h"
#include <vector>
#include <QDebug>

using namespace std;


/* Pointer to next so we can create a link list for naive algorithm, bottom-left and top-right points used for creation */
struct Rectangle
{
    int x1, y1, x2, y2;
    Rectangle* next;

    Rectangle()
    {
        x1 = 0;
        y1 = 0;
        x2 = 0;
        y2 = 0;
        next = nullptr;
    }

    Rectangle(int x1In, int y1In, int x2In, int y2In)
    {
        x1 = x1In;
        y1 = y1In;
        x2 = x2In;
        y2 = y2In;
        next = nullptr;
    }
};

/* List of active rectangles used for advance algorithm */
struct ActiveX
{
    int x;
    ActiveX* next;

    ActiveX(int xIn)
    {
        x = xIn;
        next = nullptr;
    }
};

/* Node of a segement tree */
struct SNode
{
    bool initilized;
    int y1, y2;
    int counter;

    SNode()
    {
        initilized = false;
        y1 = y2 = counter = 0;
    }
};



class ga24_unionofrectangles : public AlgorithmBase
{
public:
    ga24_unionofrectangles(QWidget* pRenderer, int delayMs, std::string filename = "", int numberOfRect = 0);
    std::vector<QRect> generateRandomRectangles(int numberOfRect);
    std::vector<QRect> readRectanglesFromFile(std::string filename);

public:
    void runAlgorithm();
    void drawAlgorithm(QPainter &painter) const;
    void runNaiveAlgorithm();
    int _naivArea;
    int _advanceArea;
    int _numberOfRect;
    std::vector<QRect> _rectangles;
    int _yPos;
    int last;
    //int g_numerOfRectangles = 0;
    int g_sizeOfTree = 0;

    /* Functions for advance algorithm */
    bool IsLeftMost(int i);
    void PropagateTree(SNode* tree, int i);
    void ShiftToRight(SNode* tree, int i);
    int AddPointToTree(SNode* tree, int y);
    void AddPointToTree(SNode* tree, int y, int rectangleIndex);
    void AddPointToX(int p, int index, int* xPoints, int* indexOfRectangles);
    void AddRectangleToTree(struct Rectangle rectangle, int recIndex, SNode* tree, int* xPoints, int* indexOfRectangles);
    void CreateTree(struct Rectangle* rectangles, SNode* tree, int* xPoints, int* indexOfRectangles);
    int CalculateMultiplier(SNode* tree);
    void IncrementTree(SNode* tree, int x, struct Rectangle rec);
    int CalculateArea(SNode* tree, struct Rectangle* rectangles, int* xPoints, int* indexOfRectangles);

    /* Functions for naive algorithm */
    bool DoRectanglesIntersect(struct Rectangle* first, struct Rectangle* second);
    bool DoRectanglesIntersect(struct Rectangle first, struct Rectangle second);
    void SplitRectangleAndAddToList(struct Rectangle first, struct Rectangle* second);
    void AddRectangleToList(struct Rectangle rec);
    void CreateList(struct Rectangle* rectangles);
    void FreeList(struct Rectangle* head);
    int CalculateArea();

    /* For drawing out sweep line and sum of total area */
    int drawX1;
    int drawX2;
    bool lastX=false;

};

#endif // GA24_UNIONOFRECTANGLES_H
