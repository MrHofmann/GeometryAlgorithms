#include "ga08_monotonechain.h"

MonotoneChain::MonotoneChain(QWidget *pRender, int delayMs, std::string fileName, int inputSize)
    :ConvexHull(pRender, delayMs, fileName, inputSize)
{
    m_result = {};
    m_lastPointAdded=0;
}

bool MonotoneChain::CheckTest(const Points &A)
{
    unsigned begin = 1;
    unsigned end = A.size()-2;
    bool regular = false;
    for(; begin<A.size(); ++begin, --end)
    {
        if (A[begin] != A[end])
            regular = true;
    }
    return regular;
}

void static SortPoints (Points& points)
{
    std::sort(points.begin(), points.end(), []( const QPoint& A, const QPoint& B )
    {
       return (A.x() == B.x() ? A.y() < B.y() : A.x() < B.x());
    });
}

bool MonotoneChain::checkCondition(Sides side)
{
    return m_lastPointAdded>=2 &&
         GetPointSide(m_result[m_lastPointAdded-2], m_result[m_lastPointAdded-1], m_result[m_lastPointAdded]) ==side ;
}

int MonotoneChain::GetPointSide(const QPoint &A, const QPoint &B, const QPoint &P)
{

    int d = (P.x() - A.x())*(B.y() - A.y()) - (P.y() - A.y())*(B.x() - A.x());
    if (d == 0)
        return Sides::COLLINEAR;
    else if (d < 0)
        return Sides::LEFT;
    else
        return Sides::RIGHT;
}

void MonotoneChain::runAlgorithm()
{
    SortPoints(_points);
    int n=_points.size();
    unsigned i = 0;
    unsigned j = 0;

    //most left point/s;
    m_minXminY = _points[0];
    //most right point/s;
    m_maxXmaxY = _points[n-1];
    m_result.push_back(m_minXminY);
    ++i;
    AlgorithmBase_updateCanvasAndBlock();
    //upper hull
    for (; _points[i]!=m_maxXmaxY; ++i)
    {
    if (GetPointSide(m_minXminY, m_maxXmaxY, _points[i]) == Sides::LEFT)
        continue;
    m_result.push_back(_points[i]);
    m_lastPointAdded++;
    AlgorithmBase_updateCanvasAndBlock();
    while(checkCondition(Sides::RIGHT))
    {
        m_result.pop_back();
        m_result.pop_back();
        m_result.push_back(_points[i]);
        m_lastPointAdded--;
        AlgorithmBase_updateCanvasAndBlock();
    }
    }
    m_result.push_back(m_maxXmaxY);
    m_lastPointAdded++;
    AlgorithmBase_updateCanvasAndBlock();
    while(checkCondition(Sides::RIGHT)){
        m_result.pop_back();
        m_result.pop_back();
        m_result.push_back(m_maxXmaxY);
        m_lastPointAdded--;
    }
    AlgorithmBase_updateCanvasAndBlock();

    //lower hull
    j=n-2;
    AlgorithmBase_updateCanvasAndBlock();


    for (; _points[j]!=m_minXminY; --j)
    {
      if (GetPointSide(m_maxXmaxY, m_minXminY, _points[j]) == Sides::LEFT)
          continue;
      m_result.push_back(_points[j]);
      m_lastPointAdded++;
      AlgorithmBase_updateCanvasAndBlock();
      while(checkCondition(Sides::RIGHT))
      {
          m_result.pop_back();
          m_result.pop_back();
          m_result.push_back(_points[j]);
          m_lastPointAdded--;
          AlgorithmBase_updateCanvasAndBlock();
      }
    }
      m_result.push_back(m_minXminY);
      m_lastPointAdded++;
      AlgorithmBase_updateCanvasAndBlock();
      while(checkCondition(Sides::RIGHT)){
          m_result.pop_back();
          m_result.pop_back();
          m_result.push_back(m_minXminY);
          m_lastPointAdded--;
      }
      AlgorithmBase_updateCanvasAndBlock();
      emit animationFinished();

      bool regularity = CheckTest(m_result);
      if(_points.size() <=2)
          regularity = false;
      if (!regularity)
          //we dont have a convex hull
          m_result.clear();

}

void MonotoneChain::drawAlgorithm(QPainter &painter) const
{
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPen p = painter.pen();
    // draw points
    p.setWidth(10);
    p.setCapStyle(Qt::RoundCap);
    painter.setPen(p);
    for (const QPoint &pt : _points)
    {
        painter.drawPoint(pt);
    }
    p.setColor(Qt::black);
    p.setWidth(3);
    painter.setPen(p);
    painter.drawLine(m_minXminY, m_maxXmaxY);
    //draw results
    if (m_result.size() != 0){
        p.setColor(Qt::green);
        p.setWidth(4);
        painter.setPen(p);
        for(int i = 0; i<m_result.size()-1; ++i)
        {
            if(m_result[i]==m_maxXmaxY)
            {
                p.setColor(Qt::red);
                painter.setPen(p);
            }
            painter.drawLine(m_result[i], m_result[i+1]);
        }
    }
}

void MonotoneChain::runNaiveAlgorithm()
{
    ConvexHull::runNaiveAlgorithm();
}

Points MonotoneChain::convexHull() const
{
    return m_result;
}
