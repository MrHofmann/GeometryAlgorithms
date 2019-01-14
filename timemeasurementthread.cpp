#include "timemeasurementthread.h"

#include "config.h"
#include "mainwindow.h"
#include "algorithmbase.h"
#include "algorithms_practice/ga02_grahamscan.h"
#include "algorithms_practice/ga01_sweepline.h"
#include "algorithms_practice/ga00_drawpolygon.h"
#include "algorithms_practice/ga03_linesegmentintersection.h"

#include "algorithms_projects/ga03_nearestpoints.h"
#include "algorithms_projects/ga04_sentryplacement.h"
#include "algorithms_projects/ga05_incrementalinsertion.h"
#include "algorithms_projects/ga05_quickhull.h"
#include "algorithms_projects/ga07_chansalgorithm.h"
#include "algorithms_projects/ga10_circleintersection.h"
#include "algorithms_projects/ga15_pointrobotshortestpath.h"
#include "algorithms_projects/ga16_quadtree.h"
#include "algorithms_projects/ga17_minkowskisums.h"
#include "algorithms_projects/ga21_fixedradiuscircle.h"
#include "algorithms_projects/ga11_intervalsearchtree.h"
#include "algorithms_projects/ga12_pointlocation.h"
#include "algorithms_projects/ga06_intersectionrectangle.h"
#include "algorithms_projects/ga08_giftwrap.h"
#include "algorithms_projects/ga08_monotonechain.h"
#include "algorithms_projects/ga18_smallestenclosingdisk.h"
#include "algorithms_projects/ga20_sutherlandhodgman.h"
#include "algorithms_projects/ga23_largestemptyrectangle.h"
#include "algorithms_projects/ga09_nearestpointsvoronoi.h"
#include "algorithms_projects/ga02_rotatingcalipers.h"
#include "algorithms_projects/ga24_unionofrectangles.h"
#include "algorithms_projects/ga14_voronoidiagram.h"


TimeMeasurementThread::TimeMeasurementThread(int algorithmType, int minValue, int step, int maxValue)
    :QThread (), _algorithmType(algorithmType), _minValue(minValue), _step(step), _maxValue(maxValue)
{
}

void TimeMeasurementThread::run()
{
    clock_t begin, end;
    double optimalTime, naiveTime;

    AlgorithmBase* pAlgorithm = nullptr;

    for(int i= _minValue; i <= _maxValue; i += _step)
    {
        switch (_algorithmType)
        {
            case MainWindow::EMPTY_PRACTICE:
            case MainWindow::EMPTY_PROJECTS:
                break;
            case MainWindow::DRAW_POLYGON:
                 pAlgorithm = new DrawPolygon(nullptr, 0, "");
                 break;
            case MainWindow::SWEEP_LINE:
                pAlgorithm = new SweepLine(nullptr, 0, "");
                break;
            case MainWindow::SENTRY_PLACEMENT:
                pAlgorithm = new SentryPlacement(nullptr, 0, "", false, 0.5, i);
                break;
            case MainWindow::GRAHAM_SCAN:
                pAlgorithm = new GrahamScan(nullptr, 0, "", i);
                break;
            case MainWindow::LINE_SEGMENT_INTERSECTION:
                pAlgorithm = new LineSegmentIntersection(nullptr, 0, "", i);
                break;
            case MainWindow::NEAREST_POINTS:
                pAlgorithm = new NearestPoints(nullptr, 0, "", i);
                break;
            case MainWindow::INCREMENTAL_INSERTION:
                pAlgorithm = new IncrementalInsertion(nullptr, 0, "", i);
                break;
            case MainWindow::QUICK_HULL:
                pAlgorithm = new QuickHull(nullptr, 0, "", i);
                break;
            case MainWindow::CHANS_ALGORITHM:
                 pAlgorithm = new ChansAlgorithm(nullptr, 0, "", i);
                 break;
            case MainWindow::CIRCLE_INTERSECTION:
                 pAlgorithm = new CircleIntersection(nullptr, 0, "", i);
                 break;
            case MainWindow::POINT_ROBOT_SHORTEST_PATH:
                pAlgorithm = new PointRobotShortestPath(nullptr, 0, "", A_STAR, i);
                break;
            case MainWindow::QUADTREE:
                pAlgorithm = new Quadtree(nullptr, 0, "", i);
                break;
            case MainWindow::FIXEDRADIUSCIRCLE:
                pAlgorithm = new FixedRadiusCircle(nullptr, 0, 70, "", i);
                break;
            case MainWindow::INTERVAL_SEARCH_TREE:
                pAlgorithm = new IntervalSearchTree(nullptr, 0, "", 5000, i);
                break;
            case MainWindow::SUTHERLAND_HODGMAN:
                pAlgorithm = new SutherlandHodgman(nullptr, 0, "", i, 3);
                break;
            case MainWindow::MINKOWSKISUMS:
                pAlgorithm = new MinkowskiSums(nullptr, 0, "", i, i);
                break;
            case MainWindow::INTERSECTION_RECTANGLE:
                             pAlgorithm = new IntersectionRectangle(nullptr, 0, "", i);
                             break;
            case MainWindow::GIFTWRAP:
                pAlgorithm = new GiftWrap(nullptr, 0, "", i);
                break;
            case MainWindow::VORONOI_DIAGRAM:
                pAlgorithm = new VoronoiDiagram(nullptr,0,"",i);
            break;
            case MainWindow::MONOTONECHAIN:
                pAlgorithm = new MonotoneChain(nullptr, 0, "", i);
                break;
            case MainWindow::POINT_LOCATION:
                pAlgorithm = new TrapezoidMap(nullptr, 0, QPointF(100,100), "", i);
                break;
            case MainWindow::LARGEST_EMPTY_RECTANGLE:
                pAlgorithm = new LargestEmptyRectangle(nullptr, 0, "", i, i, i);
                break;
            case MainWindow::SMALLEST_ENCLOSING_CIRCLE:
                pAlgorithm = new MinEnclosingCircle(nullptr, 0, "", i);
                break;
            case MainWindow::UNION_OF_RECTANGLES:
                pAlgorithm = new ga24_unionofrectangles(nullptr, 0, "", i);
                break;
//            case MainWindow::NEAREST_POINTS_VORONOI:
//                pAlgorithm = new NearestPointsVoronoi(nullptr, 0, "", i);
//                break;
            case MainWindow::ROTATING_CALIPERS:
//                pAlgorithm = new RotatingCalipers(nullptr, 0, "", i);
                break;
        }

        if(pAlgorithm)
        {
#ifndef SKIP_OPTIMAL
            begin = clock();
            pAlgorithm->runAlgorithm();
            end = clock();
            optimalTime = double(end - begin) / CLOCKS_PER_SEC;

#else

            optimalTime = 0;
#endif

#ifndef SKIP_NAIVE
            begin = clock();
            pAlgorithm->runNaiveAlgorithm();
            end = clock();
            naiveTime = double(end - begin) / CLOCKS_PER_SEC * (pAlgorithm->isLastExecutionTimedOut() ? -1 : 1);

#else
            naiveTime = 0;
#endif
            emit updateChart(i, optimalTime, naiveTime);
            delete pAlgorithm;
            pAlgorithm = nullptr;
        }
    }
}
