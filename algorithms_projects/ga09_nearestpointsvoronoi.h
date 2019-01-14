/* ------------------------------------------------------
    Autor: Sreten Kovacevic
    Godina: 2018.
    Opis problema: Odredjivanje najblizeg para tacaka
                   primenom Voronoj dijagrama.
------------------------------------------------------ */

#ifndef GA09_NEARESTPOINTSVORONOI_H
#define GA09_NEARESTPOINTSVORONOI_H
/*
#include <vector>
#include <QPoint>
#include <QPair>
#include <boost/polygon/voronoi.hpp>
#include "algorithmbase.h"

using boost::polygon::voronoi_builder;
using boost::polygon::voronoi_diagram;

// Link QPoint with boost library, so it can be used for
// construction of Voronoi diagram.
namespace boost {
    namespace polygon {

        template <>
        struct geometry_concept<QPoint> { typedef point_concept type; };

        template <>
        struct point_traits<QPoint> {
            typedef int coordinate_type;

            static inline coordinate_type get(const QPoint& point,
                                              orientation_2d orient) {
                return (orient == HORIZONTAL) ? point.x() : point.y();
            }
        };
    }; // namespace polygon
}; // namespace boost

class NearestPointsVoronoi : public AlgorithmBase
{
public:
    enum AlgorithmStatus {CORRECT_INPUT,
                          CORNER_INPUT,
                          INVALID_INPUT,
                          DIAGRAM_CONSTRUCTED,
                          CHECKING,
                          DONE};

    NearestPointsVoronoi(QWidget *pRenderer,
                         int delayMs,
                         std::string filename = "",
                         int inputSize = DEFAULT_POINTS_NUM)
        : AlgorithmBase{pRenderer, delayMs} {
        if (filename == "") {
            _points = generateRandomPoints(inputSize);
        } else {
            _points = readPointsFromFile(filename);
        }

        if (_points.size() < 2) {
            _status = AlgorithmStatus::INVALID_INPUT;
        } else if (_points.size() == 2) {
            _status = AlgorithmStatus::CORNER_INPUT;
        } else {
            _status = AlgorithmStatus::CORRECT_INPUT;
        }
    }

    ///
    /// \brief Implementacija naprednog algoritma.
    ///
    void runAlgorithm();
    ///
    /// \brief Implementacija vizuelizacije naprednog algoritma.
    /// \param painter
    ///
    void drawAlgorithm(QPainter &painter) const;
    ///
    /// \brief Implementacija naivnog algoritma.
    ///
    void runNaiveAlgorithm();
    ///
    /// \brief Implementacija iscrtavanja ivica Voronoj dijagrama pri vizelizaciji.
    /// \param painter
    ///
    void drawEdges(QPainter &painter) const;
    ///
    /// \brief Ogranicavanje beskonacnih ivica (ivica kojima odgovara samo jedan cvor dijagrama)
    /// \param edge Beskonacna ivica koju treba obraditi.
    /// \param clipped_edge Niz koji sadrzi par tacaka koje ogranicavaju novu ivicu.
    ///
    void clipInfiniteEdge(const voronoi_diagram<double>::edge_type& edge, std::vector<QPoint>* clipped_edge) const;

    // Write getters and setters for testing.
    QPair<QPoint, QPoint> nearestPair() const;
    AlgorithmStatus status() const;
    std::vector<QPoint> points() const;
    void setPoints(const std::vector<QPoint>& points);

private:
    AlgorithmStatus _status;
    voronoi_diagram<double> _diagram;
    std::vector<QPoint> _points;
    QPair<QPoint, QPoint> _nearestPair;
    QPair<QPoint, QPoint> _pairToCheck;
};
*/
#endif // GA09_NEARESTPOINTSVORONOI_H
