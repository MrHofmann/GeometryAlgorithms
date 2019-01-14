#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QBoxLayout>

#include "timemeasurementthread.h"

/*QChart*/
#include <QtCharts/QLineSeries>
QT_CHARTS_USE_NAMESPACE

#include "renderarea.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    enum Practice_AlgorithmType {
        EMPTY_PRACTICE = 0,
        DRAW_POLYGON,
        SWEEP_LINE,
        GRAHAM_SCAN,
        LINE_SEGMENT_INTERSECTION,
        DCEL_DEMO,
        TRIANGULATION,
        MAX_PRACTICE = 99
    };
    enum Projects_AlgorithmType {
        EMPTY_PROJECTS = 100,
        NEAREST_POINTS,
        SENTRY_PLACEMENT,
        INCREMENTAL_INSERTION,
        QUICK_HULL,
        CHANS_ALGORITHM,
        CIRCLE_INTERSECTION,
        POINT_ROBOT_SHORTEST_PATH,
        QUADTREE,
        SMALLEST_ENCLOSING_CIRCLE,
        FIXEDRADIUSCIRCLE,
        CONVEXHULLFORDISKS,
        INTERVAL_SEARCH_TREE,
        MINKOWSKISUMS,
        POINT_LOCATION,
        INTERSECTION_RECTANGLE,
        SUTHERLAND_HODGMAN,
        VORONOI_DIAGRAM,
        LARGEST_EMPTY_RECTANGLE,
        GIFTWRAP,
        MONOTONECHAIN,
        NEAREST_POINTS_VORONOI,
        UNION_OF_RECTANGLES,
        ROTATING_CALIPERS,
        /* Ovde se ubacuju enum-i za studentske projekte
         * (nije potrebno navoditi konkretne brojeve)
        */
        MAX_PROJECTS = 199
    };
private:
    void makeNewAlgotirhm(std::string filename = "");
    void addAditionalParams(int algorithmType);
    void removeAdditionalParams();

private slots:
    void on_algorithmType_currentIndexChanged(int index);
    void on_importDataFromFile_clicked();
    void on_generateRandomData_clicked();
    void on_clean_clicked();
    void on_start_clicked();
    void on_pause_clicked();
    void on_next_clicked();
    void on_restart_clicked();

    void on_startMeasurement_clicked();
    void on_lineSeriesChange(double dim, double optimal, double naive);

    void on_animationSpeed_valueChanged(int value);

    void on_tabWidget_currentChanged(int index);

public slots:
    void on_animationFinished();

private:
    Ui::MainWindow *ui;
    RenderArea *_renderArea;
    AlgorithmBase* _pAlgorithm;
    int _delayMs;
    std::string _filename;
    QChart* _chart;

    QLineSeries* _naiveSeries;
    QLineSeries* _optimalSeries;
    QLineSeries* _naiveSeriesTimedOut;

    TimeMeasurementThread* _mThread;
};

#endif // MAINWINDOW_H
