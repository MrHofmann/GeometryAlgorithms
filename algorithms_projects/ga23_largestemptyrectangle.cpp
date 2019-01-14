#include "ga23_largestemptyrectangle.h"
#include <iterator>
#include <algorithm>
#include <iostream>
#include <fstream>
bool poredjenje(const QPoint lhs, const QPoint rhs)
{
    if(lhs.x() != rhs.x())
        return lhs.x() < rhs.x();
    else
        return lhs.y() < rhs.y();
}

LargestEmptyRectangle::LargestEmptyRectangle(QWidget *pRenderer, int delayMs, std::string filename, int inputSize, int maxX, int maxY)
    :AlgorithmBase{pRenderer, 0}, _width{maxX}, _height{maxY}
{
    if(filename == "")
    {
        _points = generateRandomPoints(inputSize, _width, _height);
        std::sort(_points.begin(), _points.end(), poredjenje);
    }
    else
    {
        _points = readPointsFromFile(filename);
        std::sort(_points.begin(), _points.end(), poredjenje);
    }
}

void LargestEmptyRectangle::runAlgorithm()
{
    setBest_ll(QPoint(0,0));
    setBest_ur(QPoint(-1, -1));

    std::vector<std::vector<int>> matrix(getWidth(), std::vector<int>(_height, 1));
    updateMatrix(matrix);
    setBestArea(0);
    std::vector<int> cache(_width+1, 0);
    std::stack<QPoint> stack;
    for(int j=0; j < _height; ++j)
    {
        updateCashe(cache,  matrix, j);
        int currentWidth = 0;
        for(int i=0; i < _width+1; i++)
        {

            if(cache[i] > currentWidth)
            {
                stack.push(QPoint(i, currentWidth));
                currentWidth = cache[i];
            }
            else if(cache[i] < currentWidth)
            {
                QPoint temp;
                do
                {
                    temp = stack.top();
                    stack.pop();
                    int area = 0;
                    area = currentWidth*(i-temp.x());
                    if(area > getBestArea())
                    {
                        setBest_ll(QPoint(temp.x(), j));
                        setBest_ur(QPoint(i-1, j-currentWidth+1));
                        setBestArea(area);
                        AlgorithmBase_updateCanvasAndBlock();
                   }
                    currentWidth = temp.y();
                }while(cache[i] < currentWidth);
                currentWidth = cache[i];
                if(currentWidth != 0)
                    stack.push(temp);
            }
        }
    }
    emit animationFinished();
}

void LargestEmptyRectangle::drawAlgorithm(QPainter &painter) const
{
    QPen p = painter.pen();
    p.setColor(Qt::red);
    p.setWidth(5);
    painter.setPen(p);
    for(QPoint pt : _points) {
         painter.drawPoint(pt);
    }

    p.setWidth(2);
    p.setColor(Qt::blue);
    painter.setPen(p);
    QRect r(getBest_ll(), getBest_ur());
    painter.drawRect(r);
}

void LargestEmptyRectangle::runNaiveAlgorithm()
{
    setBest_ll(QPoint(0,0));
    setBest_ur(QPoint(-1, -1));
    for(int i=0; i < _height; i++)
    {
        for(int j=0; j < _height; j++)
        {
            for(int k=0; k < _width; k++)
            {
                for(int l=0; l < _width; l++)
                {
                    QPoint p1 = QPoint(k,j);
                    QPoint p2 = QPoint(l,i);
                    if((area(getBest_ll(), getBest_ur()) < area(p1, p2)) && areaIsEmpty(p1, p2))
                    {
                        setBest_ll(p1);
                        setBest_ur(p2);
                        setBestArea(area(p1, p2));
                        AlgorithmBase_updateCanvasAndBlock();
                    }
                }

            }

        }
    }
    emit animationFinished();
}
void LargestEmptyRectangle::updateCashe(std::vector<int>& cache, std::vector<std::vector<int>> &matrix, int x)
{
    for(int i = 0; i < cache.size()-1; ++i)
    {
    if(matrix[i][x] != 0)
        {
            ++cache[i];
        }
        else
            cache[i]=0;
    }
}

void LargestEmptyRectangle::updateMatrix(std::vector<std::vector<int> > &matrix)
{
    auto it = _points.begin();
    while(it!=_points.end())
    {
        matrix[it->x()][it->y()] = 0;
        it++;
    }
}
int LargestEmptyRectangle::area(QPoint ll, QPoint ur)
{
    if((ll.x() > ur.x()) || (ll.y()>ur.y()))
          return -1;
       else
          return (ur.x()-ll.x()+1)*(abs(ur.y()-ll.y())+1);
}

bool LargestEmptyRectangle::areaIsEmpty(QPoint ll, QPoint ur)
{
    auto it = _points.begin();
    while(it != _points.end())
    {
        if(ll.x() <= (*it).x() && ur.x() >= (*it).x())
            if(ll.y() <= (*it).y() && ur.y() >= (*it).y())
                return false;
        it++;
    }


    return true;
}

QPoint LargestEmptyRectangle::getBest_ll() const
{
    return best_ll;
}

void LargestEmptyRectangle::setBest_ll(const QPoint &value)
{
    best_ll = value;
}

QPoint LargestEmptyRectangle::getBest_ur() const
{
    return best_ur;
}

void LargestEmptyRectangle::setBest_ur(const QPoint &value)
{
    best_ur = value;
}


int LargestEmptyRectangle::getBestArea() const
{
    return bestArea;
}

void LargestEmptyRectangle::setBestArea(int value)
{
    bestArea = value;
}

int LargestEmptyRectangle::getWidth() const
{
    return _width;
}


int LargestEmptyRectangle::getHeight() const
{
    return _height;
}

