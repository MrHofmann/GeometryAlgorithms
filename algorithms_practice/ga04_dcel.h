#ifndef GA04_DCEL_H
#define GA04_DCEL_H


#include<QPoint>
#include <QWidget>

class DCELVertex;
class DCELHalfEdge;
class DCELField;

///
/// \brief Teme poligona
///
///

enum DCELVertexType {START, SPLIT, MERGE, END, REGULAR};

class DCELVertex
{
public:
    DCELVertex();
    DCELVertex(QPoint components, DCELHalfEdge* incidentEdge);

    QPoint coordinates() const;
    void setCoordinates(const QPoint &coordinates);

    DCELHalfEdge *incidentEdge() const;
    void setIncidentEdge(DCELHalfEdge *incidentEdge);

    DCELVertexType type() const;
    void setType(const DCELVertexType &type);

private:
    QPoint _coordinates;
    DCELHalfEdge* _incidentEdge;

    //Additional data for triangulation
    DCELVertexType _type;
};

///
/// \brief Polustranica poligona
///
class DCELHalfEdge{
public:
    DCELHalfEdge();
    DCELHalfEdge(DCELVertex* origin, DCELHalfEdge* twin, DCELHalfEdge* next, DCELHalfEdge* prev, DCELField* incidentFace);

    DCELHalfEdge *twin() const;
    void setTwin(DCELHalfEdge *twin);

    DCELHalfEdge *next() const;
    void setNext(DCELHalfEdge *next);

    DCELHalfEdge *prev() const;
    void setPrev(DCELHalfEdge *prev);

    DCELField *incidentFace() const;
    void setIncidentFace(DCELField *incidentFace);

    DCELVertex *origin() const;
    void setOrigin(DCELVertex *origin);

    DCELVertex *helper() const;
    void setHelper(DCELVertex *helper);

    std::string name() const;
    void setName(const std::string &name);

private:
    DCELVertex* _origin;
    DCELHalfEdge* _twin;
    DCELHalfEdge* _next;
    DCELHalfEdge* _prev;
    DCELField* _incidentFace;

    //Additional data for triangulation
    DCELVertex* _helper;
    std::string _name;
};

///
/// \brief Polje
///
class DCELField{
public:
    DCELField();
    DCELField(DCELHalfEdge* outerComponent, DCELHalfEdge* innerComponent);

    DCELHalfEdge *outerComponent() const;
    void setOuterComponent(DCELHalfEdge *outerComponent);

    DCELHalfEdge *innerComponent() const;
    void setInnerComponent(DCELHalfEdge *innerComponent);

private:
    DCELHalfEdge* _outerComponent;
    DCELHalfEdge* _innerComponent;
};

///
/// \brief DCEL klasa
///
class DCEL
{
public:
    DCEL(std::string inputFileName);

    void runAlgorithm();

    std::vector<DCELVertex *> vertices() const;

    std::vector<DCELHalfEdge *> edges() const;

    std::vector<DCELField *> fields() const;
    void setFields(const std::vector<DCELField *> &fields);

    //TODO - test!
    void split(DCELVertex* v1, DCELVertex* v2);
    void split2(DCELVertex* v1, DCELVertex* v2, DCELHalfEdge* e1_next, DCELHalfEdge* e1_prev, DCELHalfEdge* e2_next, DCELHalfEdge* e2_prev);
    void addEdge(DCELHalfEdge* e);
private:
    DCELHalfEdge* findEgde(DCELVertex* start, DCELVertex* end);

    std::vector<DCELVertex*> _vertices;
    std::vector<DCELHalfEdge*> _edges;
    std::vector<DCELField*> _fields;
};

#endif // GA04_DCEL_H
