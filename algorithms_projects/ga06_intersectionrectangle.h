/*
Autor: Ana Stankovic
Godina: 2018
Kratak opis problema: Odredjivanje svih preseka pravougaonika pomocu brisuce prave i IntervalRect search tree-a, gde su ivice paralelne x i y osama
*/

#ifndef GA06_INTERSECTIONRECTANGLE_H
#define GA06_INTERSECTIONRECTANGLE_H

#include "algorithmbase.h"
#include <fstream>
#include <algorithm>
#include <iostream>
#include <set>
#include "utils.h"
#include <vector>
#include <QDebug>

enum TypeOfEvent {UPPER_P, LOWER_P};


// Struktura koja predstavlja duz koja se stavlja u status i proverava
struct IntervalRect
{
public:
    int low, high; // manja i veca x koordinata
    int height; // y koordinata
    TypeOfEvent type;
    QRect rect; // pamti se i ceo pravougaonik jedino zbog crtanja preseka
    IntervalRect(int l, int h, int he,QRect r, TypeOfEvent t1)
        : low(l), high(h), height(he),rect(r), type(t1)
    {
    }

};

// cvor interval search tree
struct ITNode
{
public:
    IntervalRect i;
    int max, balance;
    ITNode *left,*right, *parent;

    ITNode(IntervalRect i1, ITNode *p): i(i1),max(i1.high), balance(0), left(NULL), right(NULL), parent(p)
    {

    }

    ~ITNode() {
        delete left;
        delete right;
    }

};


/*pravljenje fje koja poredi  2 tacke dogadjaja,
posto ce presecajuca prava ici odozgo nagore, tacke dogadjaja su sortirane tako da se prvo obradjuju
one sa manjom y koordinatom
*/

struct EventNodeComp{
    bool operator()( IntervalRect l,  IntervalRect r) const
    {
    if(l.height != r.height)
        return l.height < r.height;
    else if(l.low != r.low)
        return l.low < r.low;
    else if(l.low == r.low && l.high != r.high)
        return l.high < r.high;
    else
        return l.type < r.type;
    }

};

//interval search tree
struct ITree {
public:
    ITNode *root;
    std::vector<QRect> _overlap;
    ITree(void): root(NULL) {}
    ~ITree(void) {
        delete root;
    }
    bool insert(IntervalRect i){
        if (root == NULL) {
            root = new ITNode(i, NULL);
        }
        else {
            ITNode *n = root, *parent;


            while (true) {
                int l = (n->i).low;
                int d = (n->i).high;
                //ako vec postoji taj IntervalRect, vrati false
                if (n->i.low == i.low && n->i.high == i.high && n->i.rect == i.rect)
                    return false;

                parent = n;

                bool goLeft = false;
                if(i.low < l || (i.low == l && i.high < d) || (i.low == l && i.high == d && i.height < n->i.height))
                    goLeft = true;

                if(n!=NULL){
                    if (n->max < i.high)
                        n->max = i.high;
                }

                n = goLeft ? n->left : n->right;


                if (n == NULL) {
                    if (goLeft) {
                        parent->left = new ITNode(i, parent);

                    }
                    else {
                        parent->right = new ITNode(i, parent);
                    }

                    rebalance(parent);
                    break;
                }
            }

        }

        return true;
    }
    void deleteIntervalRect(IntervalRect delI){
        if (root == NULL)
            return;

        ITNode
            *n       = root,
            *parent  = root,
            *delNode = NULL,
            *temp = NULL,
            *child   = root;


        while (child != NULL) {
            parent = n;
            n = child;


            child = delI.low > n->i.low ? n->right : n->left;

            if(delI.low == n->i.low){
                if(delI.high > n->i.high)
                    child = n->right;
                else if(delI.high < n->i.high)
                    child = n->left;
                else if(delI.high == n->i.high && delI.rect == n->i.rect)
                    child = n->right;
                else if(delI.high == n->i.high && delI.height < n->i.height)
                    child = n->left;
                else if(delI.high == n->i.high && delI.height >= n->i.height)
                    child = n->right;
            }

            if (delI.low == n->i.low && delI.high == n->i.high && delI.rect == n->i.rect){
                delNode = n;
                //ako nemamo desno podstablo, treba da trazimo najdesniji u levom podstablu
                if(n->right == NULL)
                    child = n->left;

                //ZA APDEJTOVANJE MAKSA se penjemo gore uz stablo
                temp = delNode;

                if(temp!=NULL){
                    while(temp->parent != NULL && temp->parent->max == delNode->i.high){
                        //treba da se odredi  da li je temp levi sin ili desni sin
                        bool left = false;
                        if(temp->parent->left != NULL && temp->parent->left->i.low == temp->i.low && temp->parent->left->i.high == temp->i.high && temp->parent->left->i.rect == temp->i.rect)
                            left = true;
                        //ako je levi sin onda treba uporediti high od parenta i od njegovog desnog sina i apdejtovati maks
                        if(left){
                            //treba proveriti da li desni sin postoji
                            int maxRight = -1;
                            if(temp->parent->right != NULL)
                                maxRight = temp->parent->right->max;

                            temp->parent->max = temp->parent->i.high > maxRight ? temp->parent->i.high : maxRight;

                        }
                        else{
                            //treba proveriti da li levi sin postoji
                            int maxLeft = -1;
                            if(temp->parent->left != NULL)
                                 maxLeft = temp->parent->left->max;

                            temp->parent->max = temp->parent->i.high > maxLeft ? temp->parent->i.high : maxLeft;
                        }

                        temp = temp->parent;
                    }
                    //zato sto cemo taj cvor da brisemo, njegov maks postavljamo na -1 zbog sledeceg koda
                    delNode->max  = -1;
                    delNode->i.high = -1;
                    //onde dokle smo stigli, moramo da apdejtujemo njegov maks
                    int leftMax, rightMax;
                    leftMax = temp->left == NULL ? -1 : temp->left->max;
                    rightMax = temp->right == NULL ? -1 : temp->right->max;

                    if(leftMax > rightMax)
                        temp->max = temp->i.high > leftMax ? temp->i.high: leftMax;
                    else
                        temp->max = temp->i.high > rightMax ? temp->i.high: rightMax;

                }
            }
        }

        if (delNode != NULL) {
            bool balance = false;
            delNode->i = n->i;


            //apdejtujemo maks od del-noda
            child = n->left != NULL ? n->left : n->right;

            if (root->left == NULL && root->right == NULL) {
               root = NULL; //ispitivanje da li maks treba da apdejtujemo

            }
            else {
               if (parent->left == n) {
                    parent->left = child;
                    if(child!=NULL)
                        child->parent = parent;
                }
                else {
                    parent->right = child;
                    if(child!=NULL)
                        child->parent = parent;
                }

                balance = true;

            }
            n->left = NULL;
            n->right = NULL;
            delete n;

            int leftMax, rightMax;
            leftMax = parent->left == NULL ? -1 : parent->left->max;
            rightMax = parent->right == NULL ? -1 : parent->right->max;

            if(leftMax > rightMax)
                parent->max = parent->i.high > leftMax ? parent->i.high : leftMax;
            else
                parent->max = parent->i.high > rightMax ? parent->i.high : rightMax;

            if(balance)
                rebalance(parent);

        }
    }
    void printBalance(){
        printBalance(root);
        std::cout << std::endl;
    }


    bool doOVerlap(IntervalRect i1, IntervalRect i2)
    {


      if (i1.low <= i2.high && i2.low <= i1.high)
        {

            return true;

        }
      else if (i1.low == i2.low || i1.high == i2.high)
        {
            return true;
        }
      else if(i2.low < i1.low && i1.high < i2.high)
      {
      return true;
      }

        return false;

    }

     void overlapSearch(ITNode *root2, IntervalRect i)
    {

        if (root2 == NULL) return;


        if (doOVerlap(root2->i, i))
        {
            QRect novi;
            findIntersectionRectanle(root2->i.rect,i.rect,&novi);
            _overlap.push_back(novi);
        }


        bool tmp = true;
        if (root2->left != NULL && root2->left->max > i.low){
            int x = _overlap.size();
            overlapSearch(root2->left,i);
            x-=_overlap.size();
            tmp = (x < 0) ? true : false;
        }

        if(tmp){
            overlapSearch(root2->right, i);
          }
    }
    void inorder(ITNode *root)
    {
        if (root == NULL) return;


        inorder(root->left);
        qDebug() << "desni sin";
        inorder(root->right);
    }
 void findIntersectionRectanle(const QRect current,const QRect prev, QRect* interectRec)
    {


            QPoint tl = QPoint (qMax(current.topLeft().x(),prev.topLeft().x()), qMax(current.topLeft().y(),prev.topLeft().y()));
            QPoint tr = QPoint (qMin(current.topRight().x(),prev.topRight().x()), qMax(current.topRight().y(),prev.topRight().y()));
            QPoint bl = QPoint (qMax(current.bottomLeft().x(),prev.bottomLeft().x()), qMin(current.bottomLeft().y(),prev.bottomLeft().y()));
            QPoint br = QPoint (qMin(current.bottomRight().x(),prev.bottomRight().x()), qMin(current.bottomRight().y(),prev.bottomRight().y()));
            interectRec->setTopLeft(tl);
            interectRec->setTopRight(tr);
            interectRec->setBottomLeft(bl);
            interectRec->setBottomRight(br);

    }
private:

    ITNode* rotateLeft( ITNode *a ){



        ITNode *b = a->right;

        //apdejtovanje maksa
        if(a->left != NULL)
            a->max = a->i.high > a->left->max ? a->i.high:a->left->max;
        else
            a->max = a->i.high;

        b->parent = a->parent;
        a->right = b->left;

        if (a->right != NULL){
            a->right->parent = a;
            //i apdejtujemo maks
            a->max = a->max > a->right->max ? a->max : a->right->max;
        }

        b->left = a;
        b->max = b->max > a->max ? b->max : a->max;
        a->parent = b;

        if (b->parent != NULL) {
            if (b->parent->right == a) {
                b->parent->right = b;
            }
            else {
                b->parent->left = b;
            }
            //apdejtovanje maksa
            //da li je vece max u b->parent->right ili u b->parent->left ili b->parent->i.high
            //prvo treba proveriti da li postoje b->parent->left i b->parent->right
            int leftMax, rightMax;
            leftMax = b->parent->left == NULL ? -1 : b->parent->left->max;
            rightMax = b->parent->right == NULL ? -1 : b->parent->right->max;

            if(leftMax > rightMax)
                b->parent->max = b->parent->i.high > leftMax ? b->parent->i.high: leftMax;
            else
                b->parent->max = b->parent->i.high > rightMax ? b->parent->i.high: rightMax;
            //
        }

        setBalance(a);
        setBalance(b);

        return b;
    }

    ITNode* rotateRight ( ITNode *a ){
        ITNode *b = a->left;

        //apdejtovanje maksa
        if(a->right != NULL)
            a->max = a->i.high > a->right->max ? a->i.high:a->right->max;
        else
            a->max = a->i.high;
        //

        b->parent = a->parent;
        a->left = b->right;

        if (a->left != NULL){
            a->left->parent = a;
            //i apdejtujemo maks
            a->max = a->max > a->left->max ? a->max : a->left->max;
            //
        }


        b->right = a;
        b->max = b->max > a->max ? b->max : a->max;
        a->parent = b;

        if (b->parent != NULL) {
            if (b->parent->right == a) {
                b->parent->right = b;
            }
            else {
                b->parent->left = b;
            }
            //apdejtovanje maksa
            //da li je vece max u b->parent->right ili u b->parent->left ili b->parent->i.high
            //prvo treba proveriti da li postoje b->parent->left i b->parent->right
            int leftMax, rightMax;
            leftMax = b->parent->left == NULL ? -1 : b->parent->left->max;
            rightMax = b->parent->right == NULL ? -1 : b->parent->right->max;

            if(leftMax > rightMax)
                b->parent->max = b->parent->i.high > leftMax ? b->parent->i.high: leftMax;
            else
                b->parent->max = b->parent->i.high > rightMax ? b->parent->i.high: rightMax;
            //

        }

        setBalance(a);
        setBalance(b);

        return b;
    }

    ITNode* rotateLeftThenRight ( ITNode *n ){
        n->left = rotateLeft(n->left);
        return rotateRight(n);
    }

    ITNode* rotateRightThenLeft ( ITNode *n ){
        n->right = rotateRight(n->right);
        return rotateLeft(n);
    }

    void rebalance( ITNode *n ){
        setBalance(n);

        if (n->balance == -2) {
            if (height(n->left->left) >= height(n->left->right))
                n = rotateRight(n);
            else
                n = rotateLeftThenRight(n);

        }
        else if (n->balance == 2) {
            if (height(n->right->right) >= height(n->right->left))
                n = rotateLeft(n);
            else
                n = rotateRightThenLeft(n);

        }

        if (n->parent != NULL)
            rebalance(n->parent);
        else
            root = n;

    }
    int height( ITNode *n ){
        if (n == NULL)
            return -1;
        return 1 + std::max(height(n->left), height(n->right));
    }

    void setBalance( ITNode *n ){
        n->balance = height(n->right) - height(n->left);
    }
    void printBalance( ITNode *n ){
        if (n != NULL) {
            printBalance(n->left);
            std::cout << n->balance << " ";
            printBalance(n->right);
        }
    }

};


class IntersectionRectangle : public AlgorithmBase
{
public:
    IntersectionRectangle(QWidget *pRenderer, int delayMs, std::string filename = "", int rectanglesNum = 15);


    std::vector<QRect> generateRandomRectangles(int rectanglesNum);
    std::vector<QRect> readRectanglesFromFile(std::string fileName);
    std::vector<QRect> advancedIntersectionRectangles() const;
    std::vector<QRect> naiveIntersectionRectangles() const;
    void setRectangle(const std::vector<QRect> &rect);
    void drawRectangleStatus(ITNode *root, QPainter &painter) const;

    void findIntersectionRec(const QRect current, const QRect prev, QRect* interectRec); // fja koja nalazi preseke pravougaonika
public:
    void runAlgorithm();
    void drawAlgorithm(QPainter &painter) const;
    void runNaiveAlgorithm();

private:

    std::set<IntervalRect,EventNodeComp> _eventQueue;
    ITree _statusQueue; // status, za skup duzi koji trenutno sece
    std::vector<QRect> _rectangles; //vektor sa pravoougaonicima koje treba ispitati
    std::vector<QRect> _naiveIntersectionRectangles; //vektor sa presecnim tackama za duzi - naivni algoritam
    std::vector<QRect> _advancedIntersectionRectangles; //vektor sa presecnim tackama za duzi - napredni algoritam

    int _sweepLine;
    int _pointIntersec;
    static int constexpr DRAWING_BORDER = 10;


};


#endif // GA06_INTERSECTIONRECTANGLE_H

