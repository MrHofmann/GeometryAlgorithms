#include "ga08_giftwrap.h"

GiftWrap::GiftWrap(QWidget *pRender, int delayMs, std::string fileName, int inputSize)
    :ConvexHull(pRender, delayMs, fileName, inputSize)
{
    m_potentialNode = _points.end();
    m_workingNode = _points.end();
    m_result = {};
}

void GiftWrap::AddColinearPoints(Points &result, const Points source)
{
    Points newResulst = {};
    unsigned rFirst = 0;
    unsigned rSecond = 1;
    newResulst.push_back(result[rFirst]);
    for (; rSecond <= result.size()-2; )
    {
        for (unsigned sBegin = 0; sBegin < source.size()-1; ++sBegin)
        {
            if (PointALreadyIn(newResulst, source[sBegin]))
                continue;
            if (GetPointSide(result[rFirst], result[rSecond], source[sBegin]) == Sides::COLLINEAR &&
                    !PointALreadyIn(m_result, source[sBegin]))
            {
                    newResulst.push_back(source[sBegin]);
            }
            //we want to check last point with first as well
            if (rSecond == result.size()-2)
            {
                if (GetPointSide(result[0], result[rSecond], source[sBegin]) == Sides::COLLINEAR &&
                        !PointALreadyIn(m_result, source[sBegin]))
                {
                        newResulst.push_back(source[sBegin]);
                }
            }

        }
        newResulst.push_back(result[rSecond]);
        ++rFirst;
        ++rSecond;
    }

    newResulst.push_back(m_result[m_result.size()-1]);

    if (newResulst.size()!=m_result.size())
    {
        m_result.clear();
        m_result.assign(newResulst.begin(), newResulst.end());
    }
}

bool GiftWrap::CheckTest(const Points &A)
{
    for(unsigned i=1; i<A.size()-3; ++i)
    {
        if(GiftWrap::GetPointSide(A[0], A[A.size()-2], A[i])!=Sides::COLLINEAR)
           return true;
    }
    return false;
}

int GiftWrap::GetPointSide(QPoint A, QPoint B, QPoint P)
{

    int d = (P.x() - A.x())*(B.y() - A.y()) - (P.y() - A.y())*(B.x() - A.x());
    if (d == 0)
        return Sides::COLLINEAR;
    else if (d < 0)
        return Sides::LEFT;
    else
        return Sides::RIGHT;
}

bool GiftWrap::PointALreadyIn(const Points &source, const QPoint &A)
{
    return std::find(source.begin(), source.end(), A) != source.end();
}

void GiftWrap::runAlgorithm()
{

    QPoint StartingPoint;
    Points::const_iterator IteratorOfStart = std::min_element( _points.begin(), _points.end(),
                                 []( const QPoint &A, const QPoint &B )
                                 {
                                     return A.x() < B.x();
                                 } );
    if(IteratorOfStart!=_points.end())
        StartingPoint = *IteratorOfStart;
    else
    {
        std::cerr<< "Error in GiftWrap::runAlgorithm() - iteratorOfStart";
        exit(1);
    }
    AlgorithmBase_updateCanvasAndBlock();
    m_result.push_back(StartingPoint);
    m_potentialNode = IteratorOfStart;

    do
    {
        m_potentialNode++;
        if (m_potentialNode == _points.end())
          m_potentialNode = _points.begin();

        AlgorithmBase_updateCanvasAndBlock();

        for (m_workingNode = _points.begin(); m_workingNode != _points.end(); ++m_workingNode)
        {
            if ((PointALreadyIn(m_result, *m_workingNode) && m_workingNode!=IteratorOfStart) || m_workingNode == m_potentialNode )
                continue;
            if (GetPointSide(m_result.back(), *m_potentialNode, *m_workingNode) == Sides::LEFT)
            {
                m_potentialNode = m_workingNode;
            }
            AlgorithmBase_updateCanvasAndBlock();
        }
        if (!PointALreadyIn(m_result, *m_potentialNode) || m_potentialNode == IteratorOfStart)
        {
            m_result.push_back(*m_potentialNode);
            AlgorithmBase_updateCanvasAndBlock();
        }
    }while(m_result.front() != m_result.back());

    emit animationFinished();

    AddColinearPoints(m_result, _points);
    bool regularity = CheckTest(m_result);
    if(_points.size() <=2)
        regularity = false;
    if (!regularity)
        //we dont have a convex hull
        m_result.clear();

}
void GiftWrap::drawAlgorithm(QPainter &painter) const
{

    painter.setRenderHint(QPainter::Antialiasing, true);
    QPen p = painter.pen();
    // draw points
    p.setWidth(10);
    p.setCapStyle(Qt::RoundCap);
    //p.setColor(Qt::red);
    painter.setPen(p);
    for (const QPoint &pt : _points)
    {
        painter.drawPoint(pt);
    }

    if (m_result.size() != 0)
    {
        p.setColor(Qt::black);
        p.setWidth(4);
        painter.setPen(p);
        for(int i = 0; i<m_result.size()-1; ++i){
            painter.drawLine(m_result[i], m_result[i+1]);
        }

        if (m_potentialNode != _points.end())
        {
            p.setColor(Qt::red);
            painter.setPen(p);
            painter.drawLine(m_result.back(), *m_potentialNode);
        }

        if(m_workingNode !=_points.end() && *m_workingNode !=m_result.front() )
        {
            p.setColor(Qt::green);
            painter.setPen(p);
            painter.drawLine(m_result.back(), *m_workingNode);
        }
    }
}
void GiftWrap::runNaiveAlgorithm()
{

 ConvexHull::runNaiveAlgorithm();
}

Points GiftWrap::convexHull() const
{
    return m_result;
}
