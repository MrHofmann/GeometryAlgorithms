#include "ga24_unionofrectangles.h"
#include <stdio.h>
#include <iostream>
#include <cmath>


/* Checking if element is in left diagonal part of "tree", elements that are 2**n
  we need this check for tree propagation*/
bool ga24_unionofrectangles::IsLeftMost(int i)
{
    bool isLeftMost = false;
    int pom =1;

    while(!((pow(2,pom)/2)>=(2*_numberOfRect)))
    {
        if(i == (pow(2,pom)-1))
            isLeftMost = true;
        pom++;
    }

    if (i == 0)
    {
        isLeftMost = true;
    }

    return isLeftMost;
}

/* Propagating the value of new leaf to the parents and nodes that should be above it */
void ga24_unionofrectangles::PropagateTree(SNode* tree, int i)
{
    do
    {
        i = (i - 1) / 2;
        tree[i].initilized = tree[i * 2 + 1].initilized && tree[i * 2 + 2].initilized;
        tree[i].y1 = !IsLeftMost(i) ? tree[i - 1].y2 : tree[i * 2 + 1].y1;
        tree[i].y2 = tree[i * 2 + 2].y2 > tree[i * 2 + 1].y2 ? tree[i * 2 + 2].y2 : tree[i * 2 + 1].y2;
    } while (i > 0);
}

/* When adding a new leaf at the place of already initialaized leaf, we need to shift nodes to make space */
void ga24_unionofrectangles::ShiftToRight(SNode* tree, int i)
{
    if (tree[i + 1].initilized == true)
    {
        ShiftToRight(tree, i + 1);
    }

    tree[i + 1].initilized = tree[i].initilized;
    tree[i + 1].y1 = tree[i].y1;
    tree[i + 1].y2 = tree[i].y2;
    PropagateTree(tree, i + 1);
    tree[i].initilized = false;
}

/* Adding y point to the tree */
int ga24_unionofrectangles::AddPointToTree(SNode* tree, int y)
{
    int i = 0;
    int n = 1;

    while(!((pow(2,n)/2)>=(2*_numberOfRect)))
    {
        n++;
    }

    while (i < (pow(2,n-1)-1))
    {
        if ((tree[i].y2 >= y && (tree[i * 2 + 1].y2 >= y)) || tree[i * 2 + 1].initilized == false)
        {
            i = i * 2 + 1;
        }
        else
        {
            i = i * 2 + 2;
        }
    }

    if (tree[i].initilized == false)
    {
        tree[i].initilized = true;
        tree[i].y1 = tree[i].y2 = y;
    }
    else if(tree[i].y1 != y)
    {
        ShiftToRight(tree, i);
        tree[i].initilized = true;
        tree[i].y1 = tree[i].y2 = y;
    }

    ////std::cout << "Position: " << i << std::endl;
    return i;
}

/* Adding point to tree with additional bookeeping */
void ga24_unionofrectangles::AddPointToTree(SNode* tree, int y, int rectangleIndex)
{
    ////std::cout << "Started:" << y << std::endl;
    int i = AddPointToTree(tree, y);
    for (int j = 0; j < g_sizeOfTree; j++)
    {
        if (tree[j].initilized == true){
            ////std::cout << j << ": " << tree[j].y1 << " " << tree[j].y2 << std::endl;
            }
    }
    ////std::cout << "Added:" << y << std::endl;
    ////std::cout << "Propagate: " << i << std::endl;
    PropagateTree(tree, i);
    ////std::cout << "Propagated:" << y << std::endl;

    for (int j = 0; j < g_sizeOfTree; j++)
    {
        if (tree[j].initilized == true){
            ////std::cout << j << ": " << tree[j].y1 << " " << tree[j].y2 << std::endl;
            }
    }
}

/* Adding x point to array */
void ga24_unionofrectangles::AddPointToX(int p, int index, int* xPoints, int* indexOfRectangles)
{
    int i = 0;
    while (i <= last && xPoints[i] <= p)
    {
        i++;
    }

    if (i <= last)
    {
        /* we are also keeping track of index of rectangle */
        for (int j = last + 1; j > i; j--)
        {
            xPoints[j] = xPoints[j - 1];
            indexOfRectangles[j] = indexOfRectangles[j-1];
        }
    }

    xPoints[i] = p;
    indexOfRectangles[i] = index;
    last++;
}

/* Adds y coordinates of a rectangle to tree and x coordinates to the array */
void ga24_unionofrectangles::AddRectangleToTree(Rectangle rectangle, int recIndex, SNode* tree, int* xPoints, int* indexOfRectangles)
{
    AddPointToTree(tree, rectangle.y1, recIndex);
    AddPointToTree(tree, rectangle.y2, recIndex);

    AddPointToX(rectangle.x1, recIndex, xPoints, indexOfRectangles);
    AddPointToX(rectangle.x2, recIndex, xPoints, indexOfRectangles);
}

/* Add all rectangles to tree */
void ga24_unionofrectangles::CreateTree(Rectangle* rectangles, SNode* tree, int* xPoints, int* indexOfRectangles)
{
    for (int i = 0; i < _numberOfRect; i++)
    {
        AddRectangleToTree(rectangles[i], i, tree, xPoints, indexOfRectangles);
    }
}

/* Calucalating vertical multiplier, we will muliplie it with distance between two active x's (segment lines) */
int ga24_unionofrectangles::CalculateMultiplier(SNode* tree)
{
    int multiplier = 0;

    int FirstLeaf;
    int sizeOfTree;
    int n=1;

    /* We are searching for n, that depends from number of rectangles, so we can find leftmost leaf and size of a segment tree */
    while( !((pow(2,n)/2) >= (2*_numberOfRect)) )
    {
        n++;
    }

    /* Leftmost leaf  and size of a segment tree */
    FirstLeaf = pow(2,n-1)-1;
    sizeOfTree = pow(2,n)-1;


    /* we are checking for all y's from active rectangles (ones between two active x's), so we can find the area  */
    for (int i = FirstLeaf; i < sizeOfTree; i++)
    {
        if (tree[i].initilized == false)
        {
            return multiplier;
        }

        /* if counter is bigger then zero, we have y distance from active rectangles */
        if (tree[i].counter > 0)
        {
            multiplier += tree[i + 1].y1 - tree[i].y1;
        }
    }

    return multiplier;
}

/* Increment segment tree interval for active rectangles */
void ga24_unionofrectangles::IncrementTree(SNode* tree, int x, Rectangle rec)
{

    int i = 0;
    bool becomingActive = x == rec.x1;
    while (true)
    {
        if (tree[i].y1 == rec.y1 && tree[i].y2 == rec.y1)
        {
            while (tree[i].y1 != rec.y2)
            {
                becomingActive ? tree[i].counter++ : tree[i].counter--;
                i++;
            }

            return;
        }
        else
        {
            if (tree[i * 2 + 1].y1 <= rec.y1 && tree[i * 2 + 1].y2 >= rec.y1)
            {
                i = i * 2 + 1;
            }
            else
            {
                i = i * 2 + 2;
            }
        }
    }
}

/* Multiples tree intervals that are active with difference between two x's, adds that area to total area and incremet needed parts */
int ga24_unionofrectangles::CalculateArea(SNode* tree, Rectangle* rectangles, int* xPoints, int* indexOfRectangles)
{
    int area = 0;
    for (int i = 0; i < 2*_numberOfRect; i++)
    {
        if (i > 0)
        {
            area += CalculateMultiplier(tree) * (xPoints[i] - xPoints[i-1]);
        }

        IncrementTree(tree, xPoints[i], rectangles[indexOfRectangles[i]]);
    }

    return area;
}

/* List of rectangles for naiv algorithm */
Rectangle* First;

/* Checks if rectangles intersect, we have two off them, depends of type of input which will be used, they have the same logic */
bool ga24_unionofrectangles::DoRectanglesIntersect(Rectangle* first, Rectangle* second)
{
    bool intersectsX = false;
    if(first->x1 == second->x1 && first->y1 == second->y1 && first->x2 == second->x2 && first->y2 == second->y2)
    {
        return true;
    }

    if (first->x1 > second->x1 && first->x1 < second->x2)
    {
        intersectsX = true;
    }

    if (first->x2 > second->x1 && first->x2 < second->x2)
    {
        intersectsX = true;
    }

    if (second->x1 > first->x1 && second->x1 < first->x2)
    {
        intersectsX = true;
    }

    if (second->x2 > first->x1 && second->x2 < first->x2)
    {
        intersectsX = true;
    }

    if (intersectsX && first->y1 > second->y1 && first->y1 < second->y2)
    {
        return true;
    }

    if (intersectsX && first->y2 > second->y1 && first->y2 < second->y2)
    {
        return true;
    }

    if (intersectsX && second->y1 > first->y1 && second->y1 < first->y2)
    {
        return true;
    }

    if (intersectsX && second->y2 > first->y1 && second->y2 < first->y2)
    {
        return true;
    }

    return false;
}

/* Checks if rectangles intersect */
bool ga24_unionofrectangles::DoRectanglesIntersect(Rectangle first, Rectangle second)
{
    bool intersectsX = false;

    /* If they are equal, we are saying that they do intersect, the area of new rectangle will be ignored by splitting function */
    if(first.x1 == second.x1 && first.y1 == second.y1 && first.x2 == second.x2 && first.y2 == second.y2)
    {
        return true;
    }

    /* Checking if there is intersection on horizontal line of rectangle */
    if (first.x1 > second.x1 && first.x1 < second.x2)
    {
        intersectsX = true;
    }

    if (first.x2 > second.x1 && first.x2 < second.x2)
    {
        intersectsX = true;
    }

    if (second.x1 > first.x1 && second.x1 < first.x2)
    {
        intersectsX = true;
    }

    if (second.x2 > first.x1 && second.x2 < first.x2)
    {
        intersectsX = true;
    }

    /* They have to intersect by y coordinates too */
    if (intersectsX && first.y1 > second.y1 && first.y1 < second.y2)
    {
        return true;
    }

    if (intersectsX && first.y2 > second.y1 && first.y2 < second.y2)
    {
        return true;
    }

    if (intersectsX && second.y1 > first.y1 && second.y1 < first.y2)
    {
        return true;
    }

    if (intersectsX && second.y2 > first.y1 && second.y2 < first.y2)
    {
        return true;
    }

    return false;
}

/* If there is intersaction, split rectangle and add parts of rectangle to the list */
void ga24_unionofrectangles::SplitRectangleAndAddToList(Rectangle first, Rectangle* second)
{

    /*This is the case of completely overlapping rectangles, we can ignore this new
     *  area, it is already in sum of areas, also in case of same rectangles */
    if (first.x1 >= second->x1 && first.x2 <= second->x2
        && first.y1 >= second->y1 && first.y2 <= second->y2)
    {
        return;
    }

    /* In case we have left part of rectangle to add */
    if (first.x1 < second->x1 && first.x2 > second->x1)
    {
        AddRectangleToList(Rectangle(first.x1, max(first.y1,second->y1), second->x1, min(first.y2,second->y2)));
    }

    /* In case we have right part of rectangle to add */
    if (first.x2 > second->x2 && first.x1 < second->x2)
    {
        AddRectangleToList(Rectangle(second->x2, max(first.y1,second->y1), first.x2, min(first.y2,second->y2)));
    }

    /* In case we have top part of rectangle to add */
    if (first.y1 < second->y2 && first.y2 > second->y2)
    {
        AddRectangleToList(Rectangle(first.x1, second->y2, first.x2, first.y2));
    }

    /* In case we have bottom part of rectangle to add */
    if (first.y2 > second->y1 && first.y1 < second->y1)
    {
        AddRectangleToList(Rectangle(first.x1, first.y1, first.x2, second->y1));
    }

}

/* If it has intersection he calls function to split rectangle into parts and add it to the list or just add the whole
 * rectangle if there is no intersection */
void ga24_unionofrectangles::AddRectangleToList(Rectangle rec)
{
    //std::cout << "Started: x1: " << rec.x1 << " y1: " << rec.y1 << " x2: " << rec.x2 << " y2: " << rec.y2 << std::endl;
    Rectangle* tmp = First;
    if (!tmp)
    {
        First = new Rectangle(rec.x1, rec.y1, rec.x2, rec.y2);
    }
    else
    {
        while (tmp)
        {
            if (DoRectanglesIntersect(&rec, tmp))
            {
                //std::cout << "Matched First: x1: " << rec.x1 << " y1: " << rec.y1 << " x2: " << rec.x2 << " y2: " << rec.y2 << std::endl;
                //std::cout << "Matched Second: x1: " << tmp->x1 << " y1: " << tmp->y1 << " x2: " << tmp->x2 << " y2: " << tmp->y2 << std::endl;
                SplitRectangleAndAddToList(rec, tmp);
                //std::cout << "Splited: x1: " << rec.x1 << " y1: " << rec.y1 << " x2: " << rec.x2 << " y2: " << rec.y2 << std::endl;
                return;
            }

            //std::cout << "Not Matched First: x1: " << rec.x1 << " y1: " << rec.y1 << " x2: " << rec.x2 << " y2: " << rec.y2 << std::endl;
            //std::cout << "Not Matched Second: x1: " << tmp->x1 << " y1: " << tmp->y1 << " x2: " << tmp->x2 << " y2: " << tmp->y2 << std::endl;

            if (tmp->next)
                tmp = tmp->next;
            else
                break;
        }

        tmp->next = new Rectangle(rec.x1, rec.y1, rec.x2, rec.y2);
    }

    //std::cout << "Added: x1: " << rec.x1 << " y1: " << rec.y1 << " x2: " << rec.x2 << " y2: " << rec.y2 << std::endl;
}

void ga24_unionofrectangles::CreateList(Rectangle* rectangles)
{
    for (int i = 0; i < _numberOfRect; i++)
    {
        AddRectangleToList(rectangles[i]);
    }
}

/* We are freeing up memory used by a list for rectangles in naive algorithm */
void ga24_unionofrectangles::FreeList(Rectangle *head)
{
  if (head != NULL)
  {
      Rectangle* next = head->next;
      free(head);
      FreeList(next);
  }
}

/* Calculates area by going trough list of rectangles and adding there area */
int ga24_unionofrectangles::CalculateArea()
{
    Rectangle* tmp = First;
    int area = 0;
    while (tmp)
    {
        area += (tmp->x2 - tmp->x1) * (tmp->y2 - tmp->y1);
        tmp = tmp->next;
    }

    /* We need to free the memory taken by the list */
    FreeList(First);

    return area;
}

ga24_unionofrectangles::ga24_unionofrectangles(QWidget *pRenderer, int delayMs, std::string filename, int numberOfRect)
    :AlgorithmBase{pRenderer, delayMs}, _yPos{0}
{
    if(filename == "")
    {

        /* Calling a functin to create random rectangles and put it in vector */
        _rectangles = generateRandomRectangles(numberOfRect);

        _numberOfRect = numberOfRect;


        /* Checking if choosen random number for generating random rectangles is positive and bigger than zero
            in case it's not we are putting area on zero  */
        if (numberOfRect <= 0)
        {
            _naivArea =0;
            _advanceArea =0;
        }

        //runNaiveAlgorithm();
        //runAlgorithm();
        //std::cout << "Advance area: " << _advanceArea << "Naive area: " << _naivArea << endl;
    }
    else
    {

        /* Calling a function to read rectangles from a file*/
        _rectangles = readRectanglesFromFile(filename);


        _numberOfRect = _rectangles.size();

        /* Checking if choosen random number for generating random rectangles is positive and bigger than zero
            in case it's not we are putting area on zero  */
        if (_numberOfRect <= 0)
        {
            _naivArea =0;
            _advanceArea =0;
        }

        //runNaiveAlgorithm();
        //runAlgorithm();
        //std::cout << "Advance area: " << _advanceArea << "Naive area: " << _naivArea << endl;
    }

}

/* Function for generating random rectangles */
std::vector<QRect> ga24_unionofrectangles::generateRandomRectangles(int numberOfRect)
{
    srand((int)time(0));

    std::vector<QRect> RandomRectangles;

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

    //Creation of rectangles using constructor with two points topLeft and bottomRight
    for(int i = 0; i < numberOfRect; i++)
    {
        int x1 = 10 + rand()%width;
        int y1 = 10 + rand()%height;
        int x2 = 10 + rand()%width;
        int y2 = 10 + rand()%height;

        /* We are making sure that left point is higher then right point */

        int tl_x = min(x1,x2); //Top-left x point
        int tl_y = max(y1,y2); //Top-left y point
        int br_x = max(x1,x2); //Bottom-right x point
        int br_y = min(y1,y2); //Bottom-right y point

        QRect rectangle(QPoint(tl_x,tl_y), QPoint(br_x,br_y));

        RandomRectangles.emplace_back(rectangle);

    }

    return RandomRectangles;
}

/* Function for reading rectangles from file */
std::vector<QRect> ga24_unionofrectangles::readRectanglesFromFile(std::string filename)
{
    std::ifstream inputFile(filename);
    std::vector<QRect> vectorRectangles;

    /* In file we first have coordinates of top-left point and then botton-right  */
    int tl_x,tl_y,br_x,br_y;
    while(inputFile >> tl_x >> tl_y >> br_x >> br_y)
    {

        QRect rectangle(QPoint(tl_x,tl_y), QPoint(br_x,br_y));
        vectorRectangles.emplace_back(rectangle);
    }

    return vectorRectangles;
}

void ga24_unionofrectangles::runAlgorithm()
{

    Rectangle rectangles[_numberOfRect];

    for (int i = 0; i < _numberOfRect; i++)
    {
        int lb_x = min(_rectangles[i].topRight().x(),_rectangles[i].bottomLeft().x());
        int lb_y = min(_rectangles[i].topRight().y(),_rectangles[i].bottomLeft().y());
        int tr_x = max(_rectangles[i].topRight().x(),_rectangles[i].bottomLeft().x());
        int tr_y = max(_rectangles[i].topRight().y(),_rectangles[i].bottomLeft().y());

        rectangles[i]= Rectangle(lb_x,lb_y,tr_x,tr_y);
    }

    /* We are searching for size of a tree, it's going to be 2^n-1  */
    int n=1;
    while( !( (pow(2,n)/2) >= (2*_numberOfRect) ) )
    {
        n++;
    }

    /* initialization of variable with size of a segment tree */
    g_sizeOfTree = pow(2,n)-1;

    /* Initialization of array for segment tree, xPoints and indexOfRectangles */
    SNode tree[g_sizeOfTree];
    int xPoints[2*_numberOfRect];
    int indexOfRectangles[2*_numberOfRect];

    last = -1;

    /* Creating a segment tree */
    CreateTree(rectangles, tree, xPoints, indexOfRectangles);

    /* Call for a function for calculating the area for advance alg */
    int area = CalculateArea(tree, rectangles, xPoints, indexOfRectangles);

    _advanceArea = area;
    //std::cout << "Advance area: "  << _advanceArea << std::endl;


    /* drawing of the area that we are focusing on with in different periods, movement of sweep "lines" */
    for (int i = 0; i < 2*_numberOfRect; i++) {

        drawX1 = xPoints[i];
        drawX2 = xPoints[i+1];

        /* if it's sweep line going over biggest x, we are writting out area size */
        if( i+1 == 2*_numberOfRect )
        {
            lastX = true;
        }

        AlgorithmBase_updateCanvasAndBlock();

    }

}

void ga24_unionofrectangles::drawAlgorithm(QPainter &painter) const
{

    QPen blue = painter.pen();
    blue.setColor(Qt::blue);
    blue.setWidth(2);

    QPen green = painter.pen();
    green.setColor(Qt::green);
    green.setWidth(2);

    /* Drawing rectangles */
    for(size_t i = 0; i < _rectangles.size(); i++)
    {
        painter.setBrush(QBrush(QColor(247, 209, 131, 128)));
        painter.drawRect(_rectangles[i]);
    }

    painter.setPen(green);

    /* Drawing out sweep line */
    painter.drawLine(drawX1,0,drawX1,_pRenderer->height());
    painter.drawLine(drawX2,0,drawX2,_pRenderer->height());

    /* Writting out the area size */
    if( lastX )
    {

        QPen black = painter.pen();
        black.setWidth(2);
        black.setColor(Qt::black);
        painter.setPen(black);
        QString str = QString::number(_advanceArea);
        painter.drawText(drawX1, 20, str);
    }

    QPen p = painter.pen();
    p.setColor(Qt::green);
    painter.setPen(p);

    QPen p3 = painter.pen();
    p3.setWidth(0.1);
    p3.setColor(Qt::black);
    //p3.setDashPattern({1});
    painter.setPen(p3);


    /* Drawing out the horizontal y lines */
    for (int var = 0; var <_rectangles.size(); ++var) {
        painter.drawLine(0,_rectangles[var].topLeft().y(),_pRenderer->width(),_rectangles[var].topLeft().y());
        painter.drawLine(0,_rectangles[var].bottomLeft().y(),_pRenderer->width(),_rectangles[var].bottomLeft().y());
    }

}

void ga24_unionofrectangles::runNaiveAlgorithm()
{
    Rectangle rectangles[_numberOfRect];

    //////std::cout << "Naive: " << endl;
    //we are making sure x and y are right ones


    for (int i = 0; i < _numberOfRect; i++) {
        int lb_x = min(_rectangles[i].topRight().x(),_rectangles[i].bottomLeft().x());
        int lb_y = min(_rectangles[i].topRight().y(),_rectangles[i].bottomLeft().y());
        int tr_x = max(_rectangles[i].topRight().x(),_rectangles[i].bottomLeft().x());
        int tr_y = max(_rectangles[i].topRight().y(),_rectangles[i].bottomLeft().y());

        rectangles[i]= Rectangle(lb_x,lb_y,tr_x,tr_y);
    }

    //////std::cout << "Creating list:" <<endl;

    /* Creating a list that will have all rectangles from which we will calculate area */
    CreateList(rectangles);

    /* call of a function that will return area calculated with naive alg */
    _naivArea = CalculateArea();

    //std::cout <<"Naiv area: " << CalculateArea();

}

