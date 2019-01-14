#-------------------------------------------------
#
# Project created by QtCreator 2017-10-22T17:29:09
#
#-------------------------------------------------

QT       += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GeometryAlgorithms
TEMPLATE = app

# Navedena define naredba ukljucuje upozorenja kompilatora,
# u slucajevima kada se koriste bilo kakvi feature-i Qt-a,
# cija upotreba nije preporucena u buducnosti (tacan tekst upozorenja zavisi od konkretnog kompilatora)
# U slucaju pojave ovakvih upozorenja, upoznajte se sa dokumentacijom zastarelih feature-a,
# i pronadjite uputstva u vezi sa izbegavanjem njihove upotrebe.
DEFINES += QT_DEPRECATED_WARNINGS

# Ova opcija omogucava da se qDebug ne ispisuje u Release modu.
# Nikada ne zelimo da imamo debug poruke u kodu na kojem se mere performanse,
# narocito imajuci u vidu da je kompajler optimizovao dosta ponasanja koda,
# sto nas efektivno onemogucuje da debagujemo program.
# Greske se traze i ispravljaju u debug modu, ne u release modu.
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

# Da bi se ovaj program kompajlirao na Windows sistemima, neophodno je rucno instalirati boost,
# a zatim ga ukljuciti u ovaj projekt fajl.
# Da bi se izbegle situacije tipa: "It works on my machine", sada je moguce definisati environment promenljivu
# na nivou racunara, u kojoj ce se nalaziti putanja do foldera koji sadrzi boost.
win32 {
    INCLUDEPATH += $$(BOOST_FOLDER_LOCATION)
    LIBS += "-L$$(BOOST_FOLDER_LOCATION)/stage/lib/"
}

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    	renderarea.cpp \
    	algorithmbase.cpp \
    	animationthread.cpp \
        utils.cpp \
        algorithms_practice/ga00_drawpolygon.cpp \
    	algorithms_practice/ga01_sweepline.cpp \
        algorithms_projects/ga00_emptyproject.cpp \
        algorithms_practice/ga03_linesegmentintersection.cpp \
        algorithms_projects/ga03_nearestpoints.cpp \
        timemeasurementthread.cpp \
        algorithms_practice/ga04_dcel.cpp \
        algorithms_practice/ga04_dceldemo.cpp \
        algorithms_practice/ga05_triangulation.cpp \
        algorithms_projects/ga05_incrementalinsertion.cpp \
        algorithms_practice/ga02_grahamscan.cpp \
        algorithms_practice/convexhull.cpp \
        algorithms_projects/ga05_quickhull.cpp \
        algorithms_projects/ga16_quadtree.cpp \
        algorithms_projects/ga15_pointrobotshortestpath.cpp \
        algorithms_projects/ga21_fixedradiuscircle.cpp \
        algorithms_projects/ga18_smallestenclosingdisk.cpp \
        algorithms_projects/ga19_convexhullfordisks.cpp \
        algorithms_projects/ga11_intervalsearchtree.cpp \
        algorithms_projects/ga10_circleintersection.cpp \
        algorithms_projects/ga17_minkowskisums.cpp \
        algorithms_projects/ga12_pointlocation.cpp \
        algorithms_projects/ga06_intersectionrectangle.cpp \
        algorithms_projects/ga04_sentryplacement.cpp \
        algorithms_projects/ga07_chansalgorithm.cpp \
        algorithms_projects/ga20_sutherlandhodgman.cpp \
        algorithms_projects/ga14_voronoidiagram.cpp \
        algorithms_projects/ga23_largestemptyrectangle.cpp \
        algorithms_projects/ga08_giftwrap.cpp \
        algorithms_projects/ga08_monotonechain.cpp \
    algorithms_projects/ga12_datastructures.cpp \
    algorithms_projects/ga09_nearestpointsvoronoi.cpp \
    algorithms_projects/ga24_unionofrectangles.cpp
	
HEADERS += \
        mainwindow.h \
    	renderarea.h \
    	algorithmbase.h \
    	animationthread.h \
        utils.h \
        algorithms_practice/ga00_drawpolygon.h \
    	algorithms_practice/ga01_sweepline.h \
        algorithms_projects/ga00_emptyproject.h \
        algorithms_practice/ga03_linesegmentintersection.h \
        algorithms_practice/ga03_datastructures.h \
        algorithms_projects/ga03_nearestpoints.h \
        timemeasurementthread.h \
        config.h \
        algorithms_practice/ga04_dcel.h \
        algorithms_practice/ga04_dceldemo.h \
        algorithms_practice/ga05_triangulation.h \
        algorithms_projects/ga05_incrementalinsertion.h \
        algorithms_practice/ga02_grahamscan.h \
        algorithms_practice/convexhull.h \
        algorithms_projects/ga05_quickhull.h \
        algorithms_projects/ga16_quadtree.h \
        algorithms_projects/ga15_pointrobotshortestpath.h \
        algorithms_projects/ga21_fixedradiuscircle.h \
        algorithms_projects/ga18_smallestenclosingdisk.h \
        algorithms_projects/ga19_convexhullfordisks.h \
        algorithms_projects/ga11_intervalsearchtree.h \
        algorithms_projects/ga10_circleintersection.h \
        algorithms_projects/ga17_minkowskisums.h \
        algorithms_projects/ga12_pointlocation.h \
        algorithms_projects/ga06_intersectionrectangle.h \
        algorithms_projects/ga15_datastructures.h \
        algorithms_projects/ga04_sentryplacement.h \
        algorithms_projects/ga07_chansalgorithm.h \
        algorithms_projects/ga20_sutherlandhodgman.h \
        algorithms_projects/ga14_voronoidiagram.h \
        algorithms_projects/ga23_largestemptyrectangle.h \
        algorithms_projects/ga08_giftwrap.h \
        algorithms_projects/ga08_monotonechain.h \
    algorithms_projects/ga12_datastructures.h \
    algorithms_projects/ga09_nearestpointsvoronoi.h \
    algorithms_projects/ga24_unionofrectangles.h

FORMS += \
        mainwindow.ui

RESOURCES +=    img/play.png \
                img/pause.png \
                img/next.png \
                img/reset.png \
                img/stop.png

QMAKE_CXXFLAGS += -Wno-sign-compare
