#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QRadioButton>
#include <iostream>
#include <fstream>

/*QChart*/
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>

QT_CHARTS_USE_NAMESPACE

#include "algorithmbase.h"
#include "algorithms_practice/ga01_sweepline.h"
#include "algorithms_practice/ga00_drawpolygon.h"
#include "algorithms_practice/ga02_grahamscan.h"
#include "algorithms_practice/ga03_linesegmentintersection.h"
#include "algorithms_practice/ga04_dceldemo.h"
#include "algorithms_practice/ga05_triangulation.h"
#include "algorithms_projects/ga23_largestemptyrectangle.h"
#include "algorithms_projects/ga03_nearestpoints.h"
#include "algorithms_projects/ga04_sentryplacement.h"
#include "algorithms_projects/ga05_incrementalinsertion.h"
#include "algorithms_projects/ga05_quickhull.h"
#include "algorithms_projects/ga07_chansalgorithm.h"
#include "algorithms_projects/ga10_circleintersection.h"
#include "algorithms_projects/ga14_voronoidiagram.h"
#include "algorithms_projects/ga15_pointrobotshortestpath.h"
#include "algorithms_projects/ga16_quadtree.h"
#include "algorithms_projects/ga18_smallestenclosingdisk.h"
#include "algorithms_projects/ga21_fixedradiuscircle.h"
#include "algorithms_projects/ga19_convexhullfordisks.h"
#include "algorithms_projects/ga11_intervalsearchtree.h"
#include "algorithms_projects/ga17_minkowskisums.h"
#include "algorithms_projects/ga12_pointlocation.h"
#include "algorithms_projects/ga06_intersectionrectangle.h"
#include "algorithms_projects/ga20_sutherlandhodgman.h"
#include "algorithms_projects/ga08_giftwrap.h"
#include "algorithms_projects/ga08_monotonechain.h"
#include "algorithms_projects/ga09_nearestpointsvoronoi.h"
#include "algorithms_projects/ga24_unionofrectangles.h"
#include "algorithms_projects/ga02_rotatingcalipers.h"
#include "config.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _pAlgorithm{nullptr}, _delayMs{500}     // (default: 500)
{
    ui->setupUi(this);
    setWindowTitle("Geometrijski algoritmi @ MATF");

    /* Algorithm type */
    ui->algorithmType->addItem("SA ČASOVA VEŽBI:", QVariant(EMPTY_PRACTICE));
    ui->algorithmType->addItem("Demonstacija iscrtavanja", QVariant(DRAW_POLYGON));
    ui->algorithmType->addItem("Brišuća prava (mini demo)", QVariant(SWEEP_LINE));
    ui->algorithmType->addItem("Konveksni omotač (Graham scan)", QVariant(GRAHAM_SCAN));
    ui->algorithmType->addItem("Preseci duži", QVariant(LINE_SEGMENT_INTERSECTION));
    ui->algorithmType->addItem("DCEL (demo)", QVariant(DCEL_DEMO));
    ui->algorithmType->addItem("Triangulacija", QVariant(TRIANGULATION));

    ui->algorithmType->insertSeparator(MAX_PRACTICE);

    ui->algorithmType->addItem("STUDENTSKI PROJEKTI:", QVariant(EMPTY_PROJECTS));
    /* Ovde se ubacuju opcije za izbor studentskih projekata [START]*/
    ui->algorithmType->addItem("Određivanje dve najbliže tačke u ravni", QVariant(NEAREST_POINTS));
    ui->algorithmType->addItem("Određivanje rasporeda čuvara u poligonalnom dvodimenzionom prostoru", QVariant(SENTRY_PLACEMENT));
    ui->algorithmType->addItem("Konveksni omotač (Incremental insertion)", QVariant(INCREMENTAL_INSERTION));
    ui->algorithmType->addItem("Konveksni omotač (QuickHull)", QVariant(QUICK_HULL));
    ui->algorithmType->addItem("Konveksni omotač (Chan's algorithm)", QVariant(CHANS_ALGORITHM));
    ui->algorithmType->addItem("Odredjivanje preseka kruznica metodom brisuce prave", QVariant(CIRCLE_INTERSECTION));
    ui->algorithmType->addItem("Najkraći put između dve tačke sa obilaženjem prepreka za tačkastog robota", QVariant(POINT_ROBOT_SHORTEST_PATH));
    ui->algorithmType->addItem("Algoritam za određivanje kolizija korišćenjem strukture Quadtree", QVariant(QUADTREE));
    ui->algorithmType->addItem("Odredjivanje diska najmanjeg poluprecnika koji pokriva sve tacke u ravni", QVariant(SMALLEST_ENCLOSING_CIRCLE));
    ui->algorithmType->addItem("Pozicioniranje kruga fiksnog precnika u ravni - maksimizovanje tacaka u njegovoj unutrasnjosti", QVariant(FIXEDRADIUSCIRCLE));
    ui->algorithmType->addItem("Convex hull for disks", QVariant(CONVEXHULLFORDISKS));
    ui->algorithmType->addItem("Stablo pretrage duzi", QVariant(INTERVAL_SEARCH_TREE));
    ui->algorithmType->addItem("Određivanje sume Minkovskog dva konveksna poligona", QVariant(MINKOWSKISUMS));
    ui->algorithmType->addItem("Lociranje tačke u ravni", QVariant(POINT_LOCATION));
    ui->algorithmType->addItem("Određivanje preseka pravougaonika", QVariant(INTERSECTION_RECTANGLE));
    ui->algorithmType->addItem("Sutherland-Hodgman", QVariant(SUTHERLAND_HODGMAN));
    ui->algorithmType->addItem("Konstrukcija Vornoj dijagrama", QVariant(VORONOI_DIAGRAM));
    ui->algorithmType->addItem("Najveci prazan pravugaonik", QVariant(LARGEST_EMPTY_RECTANGLE));
    ui->algorithmType->addItem("Algoritam Monotone chain", QVariant(MONOTONECHAIN));
    ui->algorithmType->addItem("Algoritam Gift wrap", QVariant(GIFTWRAP));
    ui->algorithmType->addItem("Najbliže tačke preko Voronoj dijagrama", QVariant(NEAREST_POINTS_VORONOI));
    ui->algorithmType->addItem("Unija pravougaonika", QVariant(UNION_OF_RECTANGLES));
    ui->algorithmType->addItem("Minimalni obuhvatajuci pravougaonik", QVariant(ROTATING_CALIPERS));
    /* Ovde se ubacuju opcije za izbor studentskih projekata [END]*/

    ui->algorithmType->insertSeparator(MAX_PROJECTS);

    /* Render area setup */
    QBoxLayout* renderBoxLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    _renderArea = new RenderArea(this);
    renderBoxLayout->addWidget(_renderArea);
    ui->tab->setLayout(renderBoxLayout);

    /* Chart */
    _optimalSeries = new QLineSeries();
    _naiveSeries = new QLineSeries();
    _naiveSeriesTimedOut = new QLineSeries();

    _chart = new QChart();
    _naiveSeriesTimedOut->setColor(Qt::red);

    _optimalSeries->append(0,0);
    _naiveSeries->append(0,0);

    _optimalSeries->setName("Napredni algoritam");
    _optimalSeries->setPointsVisible();
    _naiveSeries->setName("Naivni algoritam");
    _naiveSeries->setPointsVisible();
    _naiveSeriesTimedOut->setName("Naivni algoritam - prekoračeno vreme");
    _naiveSeriesTimedOut->setPointsVisible();

    _chart->addSeries(_optimalSeries);
    _chart->addSeries(_naiveSeries);
    //_chart->addSeries(_naiveSeriesTimedOut);

    _chart->legend()->show();

    _chart->createDefaultAxes();
    _chart->setTitle("Poredjenje efikasnosti");

    if(_chart->axisX())
        _chart->axisX()->setRange(0, X_MAX_VAL);

    if(_chart->axisY())
        _chart->axisY()->setRange(0, Y_MAX_VAL);

    // Same formatting
    _chart->setBackgroundVisible(false);
    _chart->setPlotAreaBackgroundVisible(true);

    QChartView *chartView = new QChartView(_chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    QBoxLayout* chartBoxLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    chartBoxLayout->addWidget(chartView);

    ui->tab_3->setLayout(chartBoxLayout);

    /* Initial animation speed */
    _delayMs = (12-ui->animationSpeed->value())*100;

    /* Left side options */
    ui->importDataFromFile->setVisible(false);
    ui->generateRandomData->setVisible(false);
    ui->gb2_animation->setVisible(false);
    ui->gb3_params->setVisible(false);
    ui->startMeasurement->setVisible(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::makeNewAlgotirhm(std::string filename)
{
    delete _pAlgorithm;
    _pAlgorithm = nullptr;

    int currentIndex = ui->algorithmType->currentIndex();
    int currentAlgorithm = ui->algorithmType->itemData(currentIndex).toInt();
    int inputDim;
    bool checker;
    bool isNaive;

    _filename = filename;
    _delayMs = (12-ui->animationSpeed->value())*100;

    switch (currentAlgorithm)
    {
        case EMPTY_PRACTICE:
        case EMPTY_PROJECTS:
            break;
        case DRAW_POLYGON:
             _pAlgorithm = new DrawPolygon(_renderArea, _delayMs, _filename);
             break;
        case SWEEP_LINE:
            _pAlgorithm = new SweepLine(_renderArea, _delayMs, _filename);
            break;
        case SENTRY_PLACEMENT:
            inputDim = ui->gb3_params->findChild<QLineEdit*>("gui_numCorners")->text().toInt(&checker);
            isNaive = ui->gb3_params->findChild<QRadioButton*>("gui_divertToNaiveAlg")->isChecked();
            if(checker)
                _pAlgorithm = new SentryPlacement(_renderArea, _delayMs, _filename, isNaive, 10.0, inputDim);
            else
                _pAlgorithm = new SentryPlacement(_renderArea,_delayMs, _filename, isNaive, 10.0);
            break;
        case GRAHAM_SCAN:
            _pAlgorithm = new GrahamScan(_renderArea, _delayMs, _filename);
            break;
        case LINE_SEGMENT_INTERSECTION:
            inputDim = ui->gb3_params->findChild<QLineEdit*>("gui_inputDim")->text().toInt(&checker);
            if(checker)
                _pAlgorithm = new LineSegmentIntersection(_renderArea, _delayMs, _filename, inputDim);
            else
                _pAlgorithm = new LineSegmentIntersection(_renderArea, _delayMs);
            break;
        case DCEL_DEMO:
            _pAlgorithm = new DCELDemo(_renderArea, _delayMs, filename);
            break;
        case TRIANGULATION:
            _pAlgorithm = new Triangulation(_renderArea, _delayMs, filename);
            break;
        case NEAREST_POINTS:
            inputDim = ui->gb3_params->findChild<QLineEdit*>("gui_inputDim")->text().toInt(&checker);
            if(checker)
                _pAlgorithm = new NearestPoints(_renderArea, _delayMs, _filename, inputDim);
            else
                _pAlgorithm = new NearestPoints(_renderArea, _delayMs, _filename);
            break;
        case INCREMENTAL_INSERTION:
            inputDim = ui->gb3_params->findChild<QLineEdit*>("gui_inputDim")->text().toInt(&checker);
            if (checker)
                _pAlgorithm = new IncrementalInsertion(_renderArea, _delayMs, _filename, inputDim);
            else
                _pAlgorithm = new IncrementalInsertion(_renderArea, _delayMs, _filename);
            break;
        case QUICK_HULL:
            inputDim = ui->gb3_params->findChild<QLineEdit*>("gui_inputDim")->text().toInt(&checker);
            if (checker)
                _pAlgorithm = new QuickHull(_renderArea, _delayMs, _filename, inputDim);
            else
                _pAlgorithm = new QuickHull(_renderArea, _delayMs, _filename);
            break;
        case CHANS_ALGORITHM:
            inputDim = ui->gb3_params->findChild<QLineEdit*>("gui_inputDim")->text().toInt(&checker);
            if(checker)
               _pAlgorithm = new ChansAlgorithm(_renderArea, _delayMs, _filename, inputDim);
            else
               _pAlgorithm = new ChansAlgorithm(_renderArea, _delayMs, _filename);
        break;
        case CIRCLE_INTERSECTION:
            inputDim = ui->gb3_params->findChild<QLineEdit*>("gui_inputDim")->text().toInt(&checker);
            if(checker)
                _pAlgorithm = new CircleIntersection(_renderArea, _delayMs, _filename, inputDim);
            else
                _pAlgorithm = new CircleIntersection(_renderArea, _delayMs, _filename);
            break;
        case POINT_ROBOT_SHORTEST_PATH:
        {
            QPoint start, end;
            searchType search;
            bool checker1, checker2, checker3, checker4;

            checker = ui->gb3_params->findChild<QRadioButton*>("gui_searchType1")->isChecked();
            if(checker)
                search = DIJKSTRA;
            else
                search = A_STAR;

            int x1 = ui->gb3_params->findChild<QLineEdit*>("gui_line1")->text().toInt(&checker1);
            int y1 = ui->gb3_params->findChild<QLineEdit*>("gui_line2")->text().toInt(&checker2);
            int x2 = ui->gb3_params->findChild<QLineEdit*>("gui_line3")->text().toInt(&checker3);
            int y2 = ui->gb3_params->findChild<QLineEdit*>("gui_line4")->text().toInt(&checker4);
            if(checker1 && checker2 && checker3 && checker4)
            {
                start = QPoint(x1, y1);
                end = QPoint(x2, y2);
            }
            else
            {
                start = QPoint(0, 0);
                end = QPoint(0, 0);
            }
            _pAlgorithm = new PointRobotShortestPath(_renderArea, _delayMs, _filename, search, start, end);
            break;
        }
        case QUADTREE:{
            int inputSize = 300;
            int squareSize = 30;
            //if(checker){
                squareSize = ui->gb3_params->findChild<QLineEdit*>("gui_quadtree_square_size")->text().toInt(&checker);
                inputSize = ui->gb3_params->findChild<QLineEdit*>("gui_quadtree_input_size")->text().toInt(&checker);
            //}
            Quadtree *q = new Quadtree(_renderArea, _delayMs, _filename, inputSize);
            q->setSquareSize(squareSize);
            _pAlgorithm = q;
            }
            break;
        case SMALLEST_ENCLOSING_CIRCLE:
            {
                int sec_inputSize = 20;
                if(checker){
                    sec_inputSize = ui->gb3_params->findChild<QLineEdit*>("gui_inputSize")->text().toInt(&checker);
                }
                _pAlgorithm = new MinEnclosingCircle(_renderArea, _delayMs, _filename, sec_inputSize);
            }
            break;
        case FIXEDRADIUSCIRCLE:
        {
            int radius = ui->gb3_params->findChild<QLineEdit*>("gui_radius")->text().toInt(&checker);
            if(!checker)
                radius = 70;

            int inputSize = ui->gb3_params->findChild<QLineEdit*>("gui_inputSize")->text().toInt(&checker);
            if(!checker)
                inputSize = 20;
            _pAlgorithm = new FixedRadiusCircle(_renderArea, _delayMs, radius, _filename, inputSize);
            break;
        }
        case CONVEXHULLFORDISKS:
            _pAlgorithm = new ConvexHullForDisks::ConvexHullForDisks(_renderArea, _delayMs, _filename);
            break;
        case GIFTWRAP:
            inputDim = ui->gb3_params->findChild<QLineEdit*>("gui_inputDim")->text().toInt(&checker);
            if(checker)
                _pAlgorithm = new GiftWrap(_renderArea, _delayMs, _filename, inputDim);
            else
                _pAlgorithm = new GiftWrap(_renderArea, _delayMs);
            break;
        case MONOTONECHAIN:
            inputDim = ui->gb3_params->findChild<QLineEdit*>("gui_inputSize")->text().toInt(&checker);
            if(checker)
                _pAlgorithm = new MonotoneChain(_renderArea, _delayMs, _filename, inputDim);
            else
                _pAlgorithm = new MonotoneChain(_renderArea, _delayMs);
            break;
        case SUTHERLAND_HODGMAN:
        {
            int nn = ui->gb3_params->findChild<QLineEdit*>("gui_nn")->text().toInt(&checker);
            if(!checker)
                nn=10;

            int mm = ui->gb3_params->findChild<QLineEdit*>("gui_mm")->text().toInt(&checker);
            if(!checker)
                mm=3;
            _pAlgorithm = new SutherlandHodgman(_renderArea, _delayMs, _filename, nn, mm);
            break;
        }
        case MINKOWSKISUMS:
        {
            int n = ui->gb3_params->findChild<QLineEdit*>("gui_n")->text().toInt(&checker);
            if(!checker)
                n = 3;

            int m = ui->gb3_params->findChild<QLineEdit*>("gui_m")->text().toInt(&checker);
            if(!checker)
                m = 3;
            _pAlgorithm = new MinkowskiSums(_renderArea, _delayMs, _filename, n, m);
            break;
        }
    case POINT_LOCATION: {
        inputDim = ui->gb3_params->findChild<QLineEdit*>("gui_inputDim")->text().toInt(&checker);
        float xLocate = ui->gb3_params->findChild<QLineEdit*>("gui_x")->text().toFloat(&checker);
        float yLocate = ui->gb3_params->findChild<QLineEdit*>("gui_y")->text().toFloat(&checker);
        QPointF locateMe(xLocate, yLocate);
        if(checker)
            _pAlgorithm = new TrapezoidMap(_renderArea, _delayMs, locateMe, _filename, inputDim);
        else
            _pAlgorithm = new TrapezoidMap(_renderArea, _delayMs, locateMe);
        break;
    }
        case VORONOI_DIAGRAM:
        {
           _pAlgorithm= new VoronoiDiagram(_renderArea, _delayMs, _filename);
           break;
        }
        case LARGEST_EMPTY_RECTANGLE:
        {
            inputDim = ui->gb3_params->findChild<QLineEdit*>("gui_inputDim")->text().toInt(&checker);
            int width = ui->gb3_params->findChild<QLineEdit*>("gui_widthSize")->text().toInt(&checker);
            int height = ui->gb3_params->findChild<QLineEdit*>("gui_heightSize")->text().toInt(&checker);

           _pAlgorithm= new LargestEmptyRectangle(_renderArea, _delayMs, _filename, inputDim, width, height);
           break;
        }
        case INTERSECTION_RECTANGLE:
           {
               inputDim = ui->gb3_params->findChild<QLineEdit*>("gui_inputDim")->text().toInt(&checker);
               if(checker)
                   _pAlgorithm = new IntersectionRectangle(_renderArea, _delayMs, _filename, inputDim);
               else
                   _pAlgorithm = new IntersectionRectangle(_renderArea, _delayMs, _filename);
               break;
           }
        case INTERVAL_SEARCH_TREE: {
            IntervalSearchTree *newTree;
            QString line = ui->gb3_params->findChild<QLineEdit*>("gui_interval")->text();
            QStringList fields = line.split(",");

            if(fields.size() == 2){
                bool checker1, checker2;

                int x1 = fields.at(0).toInt(&checker1);
                int x2 = fields.at(1).toInt(&checker2);
                if(checker1 && checker2){
                    newTree = new IntervalSearchTree(_renderArea, _delayMs, _filename, -1);
                    newTree->setLine(Interval(x1, x2));
                }
                else{
                    qFatal("Nisu uneti ispravni brojevi");
                    newTree = new IntervalSearchTree(_renderArea, _delayMs, _filename);
                }
            }
            else{
                newTree = new IntervalSearchTree(_renderArea, _delayMs, _filename);
            }
            _pAlgorithm  = newTree;
            break;
        }
        case NEAREST_POINTS_VORONOI:
            inputDim = ui->gb3_params->findChild<QLineEdit*>("gui_inputDim")->text().toInt(&checker);
            if (checker) {
//                _pAlgorithm = new NearestPointsVoronoi(_renderArea, _delayMs, _filename, inputDim);
            } else {
//                _pAlgorithm = new NearestPointsVoronoi(_renderArea, _delayMs, _filename);
            }
            break;
         case ROTATING_CALIPERS:
            inputDim = ui->gb3_params->findChild<QLineEdit*>("gui_inputDim")->text().toInt(&checker);
            if (checker) {
//                _pAlgorithm = new RotatingCalipers(_renderArea, _delayMs, _filename, inputDim);
            } else {
//                _pAlgorithm = new RotatingCalipers(_renderArea, _delayMs, _filename);
            }
            break;

         case UNION_OF_RECTANGLES:
            inputDim = ui->gb3_params->findChild<QLineEdit*>("gui_inputDim")->text().toInt(&checker);
            if(checker) {
                _pAlgorithm = new ga24_unionofrectangles(_renderArea, _delayMs, _filename, inputDim);
            } else {
                _pAlgorithm = new ga24_unionofrectangles(_renderArea, _delayMs, _filename);
            }
            break;

    }

    if(_pAlgorithm)
    {
        _renderArea->setPAlgorithmBase(_pAlgorithm);
        connect(_pAlgorithm, SIGNAL(animationFinished()), this, SLOT(on_animationFinished()));
    }
}

void MainWindow::on_algorithmType_currentIndexChanged(int index)
{
    int currentAlgorithm = ui->algorithmType->itemData(index).toInt();

    if(currentAlgorithm != EMPTY_PRACTICE && currentAlgorithm != EMPTY_PROJECTS)
    {
        ui->importDataFromFile->setVisible(true);
        ui->generateRandomData->setVisible(true);
        ui->gb3_params->setVisible(true);
        addAditionalParams(currentAlgorithm);
    }
    else
    {
        ui->importDataFromFile->setVisible(false);
        ui->generateRandomData->setVisible(false);
        ui->gb3_params->setVisible(false);
    }
    ui->gb2_animation->setVisible(false);
}

void MainWindow::removeAdditionalParams()
{
        auto tmp = ui->gb3_params->layout();
        QLayoutItem *item;
        while ( ( item = tmp->takeAt(0) ) ){
            delete item->widget();
            delete item;
        }
        delete tmp->layout();
}

void MainWindow::addAditionalParams(int algorithmType)
{
    if ( ui->gb3_params->layout() ){
        removeAdditionalParams();
    }

    QGridLayout* additionalOptionsLayout = new QGridLayout();
    if(algorithmType == LINE_SEGMENT_INTERSECTION)
    {
            QLabel* lsi_label = new QLabel("Dimenzija ulaza: ");
            QLineEdit* lsi_text = new QLineEdit("20");
            lsi_text->setObjectName("gui_inputDim");
            additionalOptionsLayout->addWidget(lsi_label, 0, 0, 1, 1);
            additionalOptionsLayout->addWidget(lsi_text, 0, 1, 1, 1);
            ui->gb3_params->setLayout(additionalOptionsLayout);
    }
    else if(algorithmType == NEAREST_POINTS)
    {
        QLabel *label = new QLabel("Dimenzija ulaza: ");
        QLineEdit *text = new QLineEdit("20");
        text->setObjectName("gui_inputDim");
        additionalOptionsLayout->addWidget(label, 0, 0, 1, 1);
        additionalOptionsLayout->addWidget(text, 0, 1, 1, 1);
        ui->gb3_params->setLayout(additionalOptionsLayout);
    }
    else if (algorithmType == QUICK_HULL)
    {
        QLabel *label = new QLabel("Dimenzija ulaza: ");
        QLineEdit *text = new QLineEdit("20");
        text->setObjectName("gui_inputDim");
        additionalOptionsLayout->addWidget(label, 0, 0, 1, 1);
        additionalOptionsLayout->addWidget(text, 0, 1, 1, 1);
        ui->gb3_params->setLayout(additionalOptionsLayout);
    }
    else if (algorithmType == INCREMENTAL_INSERTION)
    {
        QLabel *label = new QLabel("Dimenzija ulaza: ");
        QLineEdit *text = new QLineEdit("20");
        text->setObjectName("gui_inputDim");
        additionalOptionsLayout->addWidget(label, 0, 0, 1, 1);
        additionalOptionsLayout->addWidget(text, 0, 1, 1, 1);
        ui->gb3_params->setLayout(additionalOptionsLayout);
    }
    else if (algorithmType == SENTRY_PLACEMENT)
    {
        QLabel* label = new QLabel("Broj ćoškova: ");
        QLineEdit* text = new QLineEdit("10");
        text->setObjectName("gui_numCorners");
        additionalOptionsLayout->addWidget(label, 0, 0, 1, 1);
        additionalOptionsLayout->addWidget(text, 0, 1, 1, 1);
        ui->gb3_params->setLayout(additionalOptionsLayout);

        QLabel * algTip = new QLabel("Tip algoritma:");
        QRadioButton* optimalniAlgoritam = new QRadioButton("Optimalni");
        QRadioButton* naivniAlgoritam = new QRadioButton("Naivni");

        optimalniAlgoritam->setChecked(true);
        naivniAlgoritam->setObjectName("gui_divertToNaiveAlg");
        additionalOptionsLayout->addWidget(algTip, 1, 0, 1, 1);
        additionalOptionsLayout->addWidget(optimalniAlgoritam, 2, 0, 1, 1);
        additionalOptionsLayout->addWidget(naivniAlgoritam, 2, 1, 1, 1);
        ui->gb3_params->setLayout(additionalOptionsLayout);
    }
    else if (algorithmType == FIXEDRADIUSCIRCLE)
    {
        QLabel* label = new QLabel("Precnik kruga: ");
        QLineEdit* text = new QLineEdit("70");
        text->setObjectName("gui_radius");
        additionalOptionsLayout->addWidget(label, 0, 0, 1, 1);
        additionalOptionsLayout->addWidget(text, 0, 1, 1, 1);
        ui->gb3_params->setLayout(additionalOptionsLayout);

        label = new QLabel("Broj tačaka: ");
        text = new QLineEdit("20");
        text->setObjectName("gui_inputSize");
        additionalOptionsLayout->addWidget(label, 1, 0, 1, 1);
        additionalOptionsLayout->addWidget(text, 1, 1, 1, 1);
        ui->gb3_params->setLayout(additionalOptionsLayout);
    }
    else if(algorithmType == CHANS_ALGORITHM)
    {
            QLabel* label = new QLabel("Broj tačaka: ");
            QLineEdit* text = new QLineEdit("20");
            text->setObjectName("gui_inputDim");
            additionalOptionsLayout->addWidget(label, 0, 0, 1, 1);
            additionalOptionsLayout->addWidget(text, 0, 1, 1, 1);
            ui->gb3_params->setLayout(additionalOptionsLayout);
    }
    else if(algorithmType == CIRCLE_INTERSECTION)
    {
        QLabel *label = new QLabel("Broj krugova: ");
        QLineEdit *text = new QLineEdit("20");
        text->setObjectName("gui_inputDim");
        additionalOptionsLayout->addWidget(label, 0, 0, 1, 1);
        additionalOptionsLayout->addWidget(text, 0, 1, 1, 1);
        ui->gb3_params->setLayout(additionalOptionsLayout);
    }
    else if (algorithmType == POINT_ROBOT_SHORTEST_PATH)
    {
        QLabel *label1 = new QLabel("Tip pretrage: ");
        QLabel *label2 = new QLabel("Pocetna i zavrsna tacka: ");
        QRadioButton *radio1 = new QRadioButton("Dijkstra");
        QRadioButton *radio2 = new QRadioButton("A*");
        QLineEdit *line1 = new QLineEdit("x1");
        QLineEdit *line2 = new QLineEdit("y1");
        QLineEdit *line3 = new QLineEdit("x2");
        QLineEdit *line4 = new QLineEdit("y2");

        radio1->setObjectName("gui_searchType1");
        radio2->setObjectName("gui_searchType2");
        radio1->setChecked(true);
        line1->setObjectName("gui_line1");
        line2->setObjectName("gui_line2");
        line3->setObjectName("gui_line3");
        line4->setObjectName("gui_line4");
        ui->generateRandomData->setEnabled(false);

        additionalOptionsLayout->addWidget(label1, 0, 0, 1, 1);
        additionalOptionsLayout->addWidget(radio1, 1, 0, 1, 1);
        additionalOptionsLayout->addWidget(radio2, 1, 1, 1, 1);

        additionalOptionsLayout->addWidget(label2, 2, 0, 1, 2);
        additionalOptionsLayout->addWidget(line1, 3, 0, 1, 1);
        additionalOptionsLayout->addWidget(line2, 3, 1, 1, 1);
        additionalOptionsLayout->addWidget(line3, 4, 0, 1, 1);
        additionalOptionsLayout->addWidget(line4, 4, 1, 1, 1);

        ui->gb3_params->setLayout(additionalOptionsLayout);
    }
    else if (algorithmType == QUADTREE)
    {
        QLabel* label = new QLabel("Stranica kvadrata: ");
        int def = _renderArea->height() / 30;
        def = def ? def : 30;
        QLineEdit* text = new QLineEdit(QString::number(def));
        text->setObjectName("gui_quadtree_square_size");
        additionalOptionsLayout->addWidget(label, 0, 0, 1, 1);
        additionalOptionsLayout->addWidget(text, 0, 1, 1, 1);

        label = new QLabel("Broj kvadrata: ");
        text = new QLineEdit("300");
        text->setObjectName("gui_quadtree_input_size");
        additionalOptionsLayout->addWidget(label, 1, 0, 1, 1);
        additionalOptionsLayout->addWidget(text, 1, 1, 1, 1);
        ui->gb3_params->setLayout(additionalOptionsLayout);
    }
    else if (algorithmType == INTERVAL_SEARCH_TREE)
    {
        QLabel* label = new QLabel("Koordinata duzi za koju se trazi \npresek u formatu  x,y: ");
        QLineEdit* text = new QLineEdit("");
        text->setObjectName("gui_interval");
        additionalOptionsLayout->addWidget(label, 0, 0, 1, 2);
        additionalOptionsLayout->addWidget(text, 1, 0, 1, 1);
        ui->gb3_params->setLayout(additionalOptionsLayout);
    }
    else if(algorithmType == SUTHERLAND_HODGMAN)
    {
        QLabel* label = new QLabel("Broj stranica poligona");
        QLineEdit* text = new QLineEdit("10");
        text->setObjectName("gui_nn");
        additionalOptionsLayout->addWidget(label, 0, 0, 1, 1);
        additionalOptionsLayout->addWidget(text, 0, 1, 1, 1);
        ui->gb3_params->setLayout(additionalOptionsLayout);

        label = new QLabel("Broj stranica klip poligona");
        text = new QLineEdit("3");
        text->setObjectName("gui_mm");
        additionalOptionsLayout->addWidget(label, 1, 0, 1, 1);
        additionalOptionsLayout->addWidget(text, 1, 1, 1, 1);
        ui->gb3_params->setLayout(additionalOptionsLayout);
    }
    else if(algorithmType == MINKOWSKISUMS)
    {
        QLabel* label = new QLabel("Unesite n ");
        QLineEdit* text = new QLineEdit("3");
        text->setObjectName("gui_n");
        additionalOptionsLayout->addWidget(label, 0, 0, 1, 1);
        additionalOptionsLayout->addWidget(text, 0, 1, 1, 1);
        ui->gb3_params->setLayout(additionalOptionsLayout);

        label = new QLabel("Unesite m ");
        text = new QLineEdit("3");
        text->setObjectName("gui_m");
        additionalOptionsLayout->addWidget(label, 1, 0, 1, 1);
        additionalOptionsLayout->addWidget(text, 1, 1, 1, 1);
        ui->gb3_params->setLayout(additionalOptionsLayout);
    }
    else if(algorithmType == INTERSECTION_RECTANGLE)
       {
               QLabel* lsi_label = new QLabel("Unesite broj pravougaonika: ");
               QLineEdit* lsi_text = new QLineEdit("20");
               lsi_text->setObjectName("gui_inputDim");
               additionalOptionsLayout->addWidget(lsi_label, 0, 0, 1, 1);
               additionalOptionsLayout->addWidget(lsi_text, 0, 1, 1, 1);
               ui->gb3_params->setLayout(additionalOptionsLayout);
       }
    else if (algorithmType == SMALLEST_ENCLOSING_CIRCLE)
    {
        QLabel* label = new QLabel("Broj tačaka: ");
        QLineEdit* text = new QLineEdit("20");
        text->setObjectName("gui_inputSize");
        additionalOptionsLayout->addWidget(label, 0, 0, 1, 1);
        additionalOptionsLayout->addWidget(text, 0, 1, 1, 1);
        ui->gb3_params->setLayout(additionalOptionsLayout);
    }
    else if(algorithmType == LARGEST_EMPTY_RECTANGLE)
    {
        QLabel *label = new QLabel("Dimenzija ulaza: ");
        QLineEdit *text = new QLineEdit("20");
        text->setObjectName("gui_inputDim");
        additionalOptionsLayout->addWidget(label, 0, 0, 1, 1);
        additionalOptionsLayout->addWidget(text, 0, 1, 1, 1);
        ui->gb3_params->setLayout(additionalOptionsLayout);

        label = new QLabel("Sirina ekrana: ");
        text = new QLineEdit("1200");
        text->setObjectName("gui_widthSize");
        additionalOptionsLayout->addWidget(label, 1, 0, 1, 1);
        additionalOptionsLayout->addWidget(text, 1, 1, 1, 1);
        ui->gb3_params->setLayout(additionalOptionsLayout);

        label = new QLabel("Visina ekrana: ");
        text = new QLineEdit("500");
        text->setObjectName("gui_heightSize");
        additionalOptionsLayout->addWidget(label, 2, 0, 1, 1);
        additionalOptionsLayout->addWidget(text, 2, 1, 1, 1);
        ui->gb3_params->setLayout(additionalOptionsLayout);
    }
    else if(algorithmType == POINT_LOCATION)
    {
            QLabel* label = new QLabel("Broj tačaka: ");
            QLineEdit* text = new QLineEdit("10");
            text->setObjectName("gui_inputDim");
            additionalOptionsLayout->addWidget(label, 0, 0, 1, 1);
            additionalOptionsLayout->addWidget(text, 0, 1, 1, 1);
            ui->gb3_params->setLayout(additionalOptionsLayout);

            label = new QLabel("Tražena tačka x: ");
            text = new QLineEdit("100");
            text->setObjectName("gui_x");
            additionalOptionsLayout->addWidget(label, 1, 0, 1, 1);
            additionalOptionsLayout->addWidget(text, 1, 1, 1, 1);
            ui->gb3_params->setLayout(additionalOptionsLayout);

            label = new QLabel("Tražena tačka y: ");
            text = new QLineEdit("100");
            text->setObjectName("gui_y");
            additionalOptionsLayout->addWidget(label, 2, 0, 1, 1);
            additionalOptionsLayout->addWidget(text, 2, 1, 1, 1);
            ui->gb3_params->setLayout(additionalOptionsLayout);


    }

    else if (algorithmType == GIFTWRAP)
    {
            QLabel* label = new QLabel("Broj tačaka: ");
            QLineEdit* text = new QLineEdit("20");
            text->setObjectName("gui_inputDim");
            additionalOptionsLayout->addWidget(label, 0, 0, 1, 1);
            additionalOptionsLayout->addWidget(text, 0, 1, 1, 1);
            ui->gb3_params->setLayout(additionalOptionsLayout);
    }

    else if (algorithmType == MONOTONECHAIN)
    {
            QLabel* label = new QLabel("Broj tačaka: ");
            QLineEdit* text = new QLineEdit("20");
            text->setObjectName("gui_inputDim");
            additionalOptionsLayout->addWidget(label, 0, 0, 1, 1);
            additionalOptionsLayout->addWidget(text, 0, 1, 1, 1);
            ui->gb3_params->setLayout(additionalOptionsLayout);
    } else if (algorithmType == NEAREST_POINTS_VORONOI) {
        QLabel *label = new QLabel("Dimenzija ulaza: ");
        QLineEdit *text = new QLineEdit("20");
        text->setObjectName("gui_inputDim");
        additionalOptionsLayout->addWidget(label, 0, 0, 1, 1);
        additionalOptionsLayout->addWidget(text, 0, 1, 1, 1);
        ui->gb3_params->setLayout(additionalOptionsLayout);
    } else if(algorithmType == UNION_OF_RECTANGLES) {
        QLabel *label = new QLabel("Dimenzija ulaza: ");
        QLineEdit *text = new QLineEdit("20");
        text->setObjectName("gui_inputDim");
        additionalOptionsLayout->addWidget(label, 0, 0, 1, 1);
        additionalOptionsLayout->addWidget(text, 0, 1, 1, 1);
        ui->gb3_params->setLayout(additionalOptionsLayout);
    } else if (algorithmType == ROTATING_CALIPERS) {
        QLabel *label = new QLabel("Dimenzija ulaza: ");
        QLineEdit *text = new QLineEdit("20");
        text->setObjectName("gui_inputDim");
        additionalOptionsLayout->addWidget(label, 0, 0, 1, 1);
        additionalOptionsLayout->addWidget(text, 0, 1, 1, 1);
        ui->gb3_params->setLayout(additionalOptionsLayout);
    }
}

void MainWindow::on_importDataFromFile_clicked()
{
    QString workingDirectory = "../algorithms/input_files";
    int currentIndex = ui->algorithmType->currentIndex();
    int currentAlgorithm = ui->algorithmType->itemData(currentIndex).toInt();
    if (currentAlgorithm == SMALLEST_ENCLOSING_CIRCLE)
    {
        workingDirectory += "/ga18_minEnclosingCircle";
    }
    //QString fileName = QFileDialog::getOpenFileName(this, tr("Fajl koji sadrzi koordinate tacaka"), ".", "*.*");
    QString fileName = QFileDialog::getOpenFileName(this, tr("Fajl koji sadrzi koordinate tacaka"), workingDirectory, "*.*");
    if(fileName.isEmpty())
        return;

    makeNewAlgotirhm(fileName.toStdString());
    ui->gb2_animation->setVisible(true);
}

void MainWindow::on_generateRandomData_clicked()
{
    makeNewAlgotirhm("");
    ui->gb2_animation->setVisible(true);
}

void MainWindow::on_clean_clicked()
{
    ui->algorithmType->setCurrentIndex(0);
    ui->gb2_animation->setVisible(false);
    ui->start->setVisible(true);
    ui->gb3_params->setVisible(false);
    ui->gb1_algorithm->setVisible(true);
    if(_pAlgorithm)
    {
        _pAlgorithm->resetAnimation();
        _pAlgorithm = nullptr;
        _renderArea->setPAlgorithmBase(nullptr);
        _renderArea->update();
    }
}

void MainWindow::on_start_clicked()
{
    ui->start->setVisible(false);
    ui->gb1_algorithm->setVisible(false);

    if(_pAlgorithm)
        _pAlgorithm->startAnimation();
}

void MainWindow::on_pause_clicked()
{
    _pAlgorithm->pauseOrContinueAnimation();
}

void MainWindow::on_next_clicked()
{
    if(_pAlgorithm)
        _pAlgorithm->nextStep();
}

void MainWindow::on_restart_clicked()
{
    ui->start->setVisible(false);
    ui->pause->setVisible(true);
    ui->next->setVisible(true);

    if (_pAlgorithm)
    {
        _pAlgorithm->resetAnimation();
        makeNewAlgotirhm(_filename);
        _pAlgorithm->startAnimation();
    }
}

void MainWindow::on_animationFinished()
{
    ui->gb1_algorithm->setVisible(true);
    ui->gb2_animation->setVisible(false);
    ui->start->setVisible(true);

}

void MainWindow::on_lineSeriesChange(double dim, double optimal, double naive)
{
    _optimalSeries->append(dim, optimal);

    if(naive >= 0)
        _naiveSeries->append(dim, naive);
    else
        _naiveSeriesTimedOut->append(dim, -naive);
}

void MainWindow::on_startMeasurement_clicked()
{
    int currentIndex = ui->algorithmType->currentIndex();
    int currentAlgorithm = ui->algorithmType->itemData(currentIndex).toInt();

    int minDim;
    int step;
    int maxDim;

    switch(currentAlgorithm)
    {
    case SENTRY_PLACEMENT:
        minDim = SentryPlacement::MIN_DIMENSION;
        step = SentryPlacement::STEP_SIZE;
        maxDim = SentryPlacement::MAX_DIMENSION;
        if(_chart->axisX())
            _chart->axisX()->setRange(0, SentryPlacement::X_MAX_CHART_VAL);
        _optimalSeries ->clear();
        _naiveSeries ->clear();
        _naiveSeriesTimedOut -> clear();
        _optimalSeries->append(0,0);
        _naiveSeries->append(0,0);
        break;
    case POINT_ROBOT_SHORTEST_PATH:
        minDim = 1;
        step = 1;
        maxDim = 20;
        if(_chart->axisX())
            _chart->axisX()->setRange(0, maxDim);
        if(_chart->axisY())
            _chart->axisY()->setRange(0, 200);
        break;
    case NEAREST_POINTS_VORONOI:
        minDim = 1;
        step = 1;
        maxDim = 5000;
        if (_chart->axisX()) {
            _chart->axisX()->setRange(0, maxDim);
        }
        break;
    case VORONOI_DIAGRAM:
        minDim = 1;
        step = 1;
        maxDim = 250;
        if (_chart->axisX()) {
            _chart->axisX()->setRange(0, maxDim);
        }
        break;
    default:
        minDim = MIN_DIM;
        step = STEP;
        maxDim = MAX_DIM;
        if(_chart->axisX())
            _chart->axisX()->setRange(0, X_MAX_VAL);
        break;
    }

    _mThread = new TimeMeasurementThread(currentAlgorithm, minDim, step, maxDim);
    connect(_mThread, &TimeMeasurementThread::updateChart, this, &MainWindow::on_lineSeriesChange);
    _mThread->start();
}

void MainWindow::on_animationSpeed_valueChanged(int value)
{
    if(_pAlgorithm)
    {
        _pAlgorithm->changeDelay((12-value)*100);
    }
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    if(index == 0)
    {
        ui->startMeasurement->setVisible(false);
    }
    else{
        ui->startMeasurement->setVisible(true);
    }
}
