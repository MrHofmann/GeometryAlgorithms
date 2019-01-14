/***
 * Autor: Marinela Parovic
 * Broj indeksa: 1008/2017
 * Godina: 2018
 * Kratak opis problema: Dato je n nepresijecajucih duzi u ravni koje indukuju planarno
 * razlaganje ravni na obasti. Potrebno je za zadatu tacku odrediti kojoj od oblasti
 * pripada.
 */

#ifndef GA12_DATASTRUCTURES_H
#define GA12_DATASTRUCTURES_H

#include <algorithm>
#include <QPointF>
#include <QLineF>
#include <fstream>
#include <QPainter>
#include "algorithms_practice/ga04_dcel.h"
#include <math.h>

//klasa kojom je predstavljena duz
struct Segment {

    QLineF _segment;

    //pokazivac na polje iz DCEL strukture datog razlaganja ravni
    DCELField* _fieldAbove;
    //jedinstveni identifikator polja u formatu redni_brojF(redni brojevi pocinju od 0)
    std::string _fieldName;

    //konstruktor za Duz
    Segment(QLineF segment, DCELField* fieldAbove = nullptr, std::string fieldName = "0F");

    //funkcija koja racuna tacku na datoj duzi sa x koordinatom koja se proslijedi kao argument
    //pretpostavka: vertikalna prava kroz x sijece duz za koju se metod poziva
    QPointF pointWithX (qreal x) const;

    //da li je tacka p koja se proslijedi iznad duzi
    bool isAbove(const QPointF & p, const QPointF &helper);

    //duzi su jednake ako su im jednaki krajevi
    bool operator==(const Segment& s) const;

    //true ako se duzi sijeku, inace false
    bool intersects(const Segment& s) const;

    //getters and setters
    DCELField *fieldAbove() const;
    void setFieldAbove(DCELField *fieldAbove);

private:
    //pomocna funkcija za odredjivanje pozicije tacke u odnosu na duz
    double pointPosition(const QPointF& p);

};

class SearchNode;

//klasa kojom se predstavlja Trapez
class Trapezoid {
public:

    //konstruktor bez argumenata
    Trapezoid();

    //postavljanje lijevog susjeda trapezima koji imaju samo jednog lijevog susjeda
    void setOneLeft(Trapezoid *trap);

    //postavljanje desnog susjeda trapezima koji imaju samo jednog desnog susjeda
    void setOneRight(Trapezoid *trap);

    //kada se trapez zamijeni novim, tim novim se azuriraju njegovi lijevi susjedi
    void changeLeftWithGiven(Trapezoid *tr);

    //kada se trapez zamijeni novim, tim novi se azuriraju njegovi desni susjedi
    void changeRightWithGiven(Trapezoid *tr);

    //metod za crtanje trapeza
    void draw(QPainter& painter);

    //getters and setters
    QPointF leftP() const;
    void setLeftP(const QPointF &leftP);

    QPointF rightP() const;
    void setRightP(const QPointF &rightP);

    Segment *top() const;
    void setTop(Segment *top);

    Segment *bottom() const;
    void setBottom(Segment *bottom);

    Trapezoid *lowerL() const;
    void setLowerL(Trapezoid *lowerL);

    Trapezoid *upperL() const;
    void setUpperL(Trapezoid *upperL);

    Trapezoid *lowerR() const;
    void setLowerR(Trapezoid *lowerR);

    Trapezoid *upperR() const;
    void setUpperR(Trapezoid *upperR);

    SearchNode *searchNode() const;
    void setSearchNode(SearchNode *searchNode);
private:
    //4 podatka kojima je trapez jedinstveno odredjen
    QPointF _leftP;
    QPointF _rightP;
    Segment *_top;
    Segment *_bottom;

    //4 susjeda trapeza
    Trapezoid*  _lowerL;
    Trapezoid*  _upperL;
    Trapezoid*  _lowerR;
    Trapezoid*  _upperR;

    //pokazivac na list stukture pretrage koji odgovara trapezu
    SearchNode* _searchNode;

};


//bazna klasa za cvor u strukturi pretrage
class SearchNode {
public:
    //konstruktor bez argumenata
    SearchNode();

    //virtual destruktor
    virtual ~SearchNode(){}

    //metod koji vraca trapez u cvoru, a null ako cvor ne sadrzi trapez(nije list)
    virtual Trapezoid* getTrapezoid() const {
        return nullptr;
    }

    //sljedeci cvor prilikom lociranja tacke
    virtual SearchNode* nextNode(QPointF, QPointF) {
        return nullptr;
    }

    //iscrtavanje cvora prilikom lociranja tacke
    virtual void draw(QPainter&) const {
    }

    //dodavanje lijevog potomka cvoru nad kojim se metod poziva
    void addLeft(SearchNode *node);

    //dodavanje desnog potomka cvoru nad kojim se metod poziva
    void addRight(SearchNode *node);

    //metod koji pri zamjeni cvora novim cvorom u strukturi pretrage
    //sve njegove roditelje preusmjeri da pokazuju na novi cvor
    void replaceNode(SearchNode *node);

    //lijevi i desni potomak u strukturi pretrage
    SearchNode *left;
    SearchNode *right;
private:
    //lista roditelja cvora
    std::list<SearchNode*> _parents;
};

//XNode je cvor u strukturi pretrage koji sadrzi tacku
class XNode : public SearchNode {
public:
    //konstruktor
    XNode(QPointF point);

    //sljedeci cvor u sturkturi pretrage kada se trazi tacka p1
    virtual SearchNode* nextNode(QPointF p1, QPointF);

    //iscrtavanje tacke
    virtual void draw(QPainter & p) const;

private:
    //u X cvoru se cuva tacka
    QPointF _point;
};

//YNode je cvor u strukturi pretrage koji sadrzi duz
class YNode : public SearchNode {
public:
    //konstruktor
    YNode(Segment* segment);

    //sljedeci cvor u strukturi pretrage kada se trazi p1
    virtual SearchNode* nextNode(QPointF p1, QPointF p2);

    //iscrtavanje duzi
    virtual void draw(QPainter &p) const;

private:
    //Y cvor sadrzi pokazivac na duz
    Segment* _segment;
};


//list strukture pretrage je cvor koji sadrzi trapez
class LeafNode : public SearchNode {
public:
    //konstruktor
    LeafNode(Trapezoid* trapezoid);

    //destruktor
    virtual ~LeafNode();

    //kada je dodje do lista - to je cvor koji se trazi
    virtual SearchNode* nextNode(QPointF , QPointF );

    //iscrtavanje trapeza na koji pokazuje _trapezod
    virtual void draw(QPainter & p) const;

    //getter and setter
    virtual Trapezoid *getTrapezoid() const;
    void setTrapezoid(Trapezoid *trapezoid);

private:
    //pokazivac na trapez sadrzan u listu
    Trapezoid* _trapezoid = nullptr;
};

#endif
