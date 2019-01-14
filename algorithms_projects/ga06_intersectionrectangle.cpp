#include "ga06_intersectionrectangle.h"
#include <QPainter>

IntersectionRectangle::IntersectionRectangle(QWidget *pRenderer, int delayMs, std::string filename, int rectanglesNum)
    :AlgorithmBase (pRenderer, delayMs), _eventQueue(), _statusQueue(),_naiveIntersectionRectangles() ,_advancedIntersectionRectangles()
{

if(filename == "")
    _rectangles = generateRandomRectangles(rectanglesNum);
else
    _rectangles = readRectanglesFromFile(filename);

 _sweepLine = 0; // inicijalizacija brisuce prave

/*dodavanje tacke dogadjaja u _eventQueue
 * */
    for(unsigned i = 0; i < _rectangles.size(); i++)
    {
        _eventQueue.insert({_rectangles[i].topLeft().x(),_rectangles[i].topRight().x(),_rectangles[i].topLeft().y(),_rectangles[i] ,UPPER_P}); //gornja
        _eventQueue.insert({_rectangles[i].bottomLeft().x(),_rectangles[i].bottomRight().x() ,_rectangles[i].bottomLeft().y(),_rectangles[i], LOWER_P}); //donja
    }

}
void IntersectionRectangle::runAlgorithm()
{
    _statusQueue.inorder(_statusQueue.root);

    while(!_eventQueue.empty())
    {

        IntervalRect eventNode = *_eventQueue.begin();
        _eventQueue.erase(_eventQueue.begin());

        if(eventNode.type == UPPER_P)
        {
            //pomeranje brisuce prave na bas to teme koje se obradjuje
            _sweepLine = eventNode.height;
            AlgorithmBase_updateCanvasAndBlock();
            _statusQueue.overlapSearch(_statusQueue.root,eventNode);
            _statusQueue.insert(eventNode);
        }
       //ukoliko je donje teme, samo se izbrise pravougaonik iz statusa
       //i on se  vise ne proverava sa drugim pravougaonicima
        else if (eventNode.type == LOWER_P)
        {
            _sweepLine = eventNode.height;
            AlgorithmBase_updateCanvasAndBlock();
           _statusQueue.deleteIntervalRect(eventNode);

        }
    }

    _sweepLine = 0;
    AlgorithmBase_updateCanvasAndBlock();
    emit animationFinished();

}

void IntersectionRectangle::drawAlgorithm(QPainter &painter) const
{
    /*plavu boju koriscena za crtanje svih pravougaonika koji su u statusu trenutno*/
    QPen blue = painter.pen();
    blue.setColor(Qt::blue);
    blue.setWidth(2);
    /*zelenu boju koriscena za crtanje brisuce prave*/
    QPen green = painter.pen();
    green.setColor(Qt::green);
    green.setWidth(2);
    /* */
    QPen red = painter.pen();
    red.setColor(Qt::red);
    red.setWidth(1);

    /*crtanje pravougaonika*/
    for(size_t i = 0; i < _rectangles.size(); i++)
    {
        painter.drawRect(_rectangles[i]);
    }
    /*crtanje brisuce prave*/
    painter.setPen(green);
    painter.drawLine(0, _sweepLine,2000,_sweepLine);
    /*crtanje pravougaonika koji su u statusu plavom bojom*/
    painter.setPen(blue);


    //crtanje drveta, tj. koji su u statusu trenutno
    drawRectangleStatus(_statusQueue.root, painter);

    /*crtanje presecnih tacki pravougaonika crvenom bojom*/
    painter.setPen(red);
    for(size_t i = 0; i < _statusQueue._overlap.size(); i++)
    {

        painter.drawRect(_statusQueue._overlap[i]);
    }

}
/*u naivnom algoritmu koriscena grubu silu - proveravanje presek pravouganika svaki sa svakim*/
void IntersectionRectangle::runNaiveAlgorithm()
{

    for(int i = 0; i < _rectangles.size(); i++)
    {
        for(int j = i+1; j < _rectangles.size(); j++)
        {
            if(_rectangles[i].topLeft().x() > _rectangles[j].bottomRight().x()
                    || (_rectangles[i].bottomRight().x() < _rectangles[j].topLeft().x())
                    || (_rectangles[i].topLeft().y() > _rectangles[j].bottomRight().y())
                    || (_rectangles[i].bottomRight().y() < _rectangles[j].topLeft().y()))
            {
            }
            else
            {
                QRect newRect;
                findIntersectionRec(_rectangles[i],_rectangles[j],&newRect);
                _naiveIntersectionRectangles.push_back(newRect);
            }
        }
    }


}
std::vector<QRect> IntersectionRectangle::generateRandomRectangles(int rectanglesNum)
{

    srand(static_cast<unsigned>(time(0)));
    std::vector<QRect> vectorRandomRectangles;

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

    int wMin = 10;
    int wMax = 150;
    int wDiff = wMax - wMin;

    int hMin = 10;
    int hMax = 150;
    int hDiff = hMax - hMin;

    int xMin = DRAWING_BORDER;
    int xMax = width - DRAWING_BORDER - wMax;
    int xDiff = xMax - xMin;

    int yMin = DRAWING_BORDER;
    int yMax = height - DRAWING_BORDER - hMax;
    int yDiff = yMax - yMin;


    for(int i = 0; i < rectanglesNum; i++) {
        int x  = xMin + rand()%xDiff;
        int y = yMin + rand()%yDiff;
        int w = wMin + rand()%wDiff;
        int h = hMin + rand()%hDiff;

        QRect rectangle(QPoint(x,y), QSize(w,h)); //pravljenje pravougaonika sa gornjom levom tackom i visinom i sirinom pravougaonika


        vectorRandomRectangles.emplace_back(rectangle);

    }

    return vectorRandomRectangles;
}

std::vector<QRect> IntersectionRectangle::readRectanglesFromFile(std::string fileName)
{

    std::ifstream inputFile(fileName);
    std::vector<QRect> vectorRectangles;
    int x,y,w,h;
    while(inputFile >> x >> y >> w >> h)
    {
        QRect rectangle(QPoint(x,y), QSize(w,h)); //pravljene pravougaonika sa gornjom levom tackom i visinom i sirinom pravougaonika
        vectorRectangles.emplace_back(rectangle);
    }

    return vectorRectangles;
}


/*fja za proveru preseka pravougaonika*/
void IntersectionRectangle::findIntersectionRec(const QRect current,const QRect prev, QRect* interectRec)
{

   if (prev.topLeft().x() > current.topRight().x() || current.topLeft().x() > prev.topRight().x())
    {
    }
    else
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

}

//funkcija za crtanje pravougoanika koji su u statusu
void IntersectionRectangle::drawRectangleStatus(ITNode *root, QPainter &painter) const
{
    if(root == NULL) return;

   painter.drawRect(root->i.rect);
   drawRectangleStatus(root->left, painter);
   drawRectangleStatus(root->right, painter);
}

std::vector<QRect> IntersectionRectangle::naiveIntersectionRectangles() const
{
    return _naiveIntersectionRectangles;
}


std::vector<QRect> IntersectionRectangle::advancedIntersectionRectangles() const
{

     return _statusQueue._overlap;
}

void IntersectionRectangle::setRectangle(const std::vector<QRect> &rect)
{
    for(QRect r:rect)
    {
        _eventQueue.insert({r.topLeft().x(),r.topRight().x(),r.topLeft().y(),r ,UPPER_P}); //gornja
        _eventQueue.insert({r.bottomLeft().x(),r.bottomRight().x() ,r.bottomLeft().y(),r, LOWER_P}); //donja
    }
    _rectangles = rect;
}
