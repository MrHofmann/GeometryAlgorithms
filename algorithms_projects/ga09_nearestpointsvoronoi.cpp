#include "ga09_nearestpointsvoronoi.h"
#include "utils.h"
/*
void NearestPointsVoronoi::runAlgorithm() {
    if (_status == AlgorithmStatus::INVALID_INPUT) {
        emit animationFinished();
        return;
    } else if (_status == AlgorithmStatus::CORNER_INPUT) {
        _nearestPair = {_points[0], _points[1]};
        _status = AlgorithmStatus::DONE;
        AlgorithmBase_updateCanvasAndBlock();
        return;
    }

    // Construct Voronoi diagram from set of points.
    construct_voronoi(_points.begin(), _points.end(), &_diagram);
    _status = AlgorithmStatus::DIAGRAM_CONSTRUCTED;

    _nearestPair = {};
    double minDistance = std::numeric_limits<double>::max();

    // Draw diagram.
    AlgorithmBase_updateCanvasAndBlock();

    _status = AlgorithmStatus::CHECKING;
    std::vector<bool> visited(_points.size(), false);
    for (auto it = _diagram.cells().begin();
         it != _diagram.cells().end();
         ++it) {
        QPoint p = _points[it->source_index()];
        const voronoi_diagram<double>::edge_type* edge = it->incident_edge();
        auto start = edge;
        do {
            auto neighbour = edge->twin()->cell();
            unsigned index = neighbour->source_index();

            // If cell is visited, then we checked this pair already.
            if (visited[index]) {
                edge = edge->next();
                continue;
            }

            QPoint q = _points[index];
            _pairToCheck = {p, q};
            AlgorithmBase_updateCanvasAndBlock();

            double tmpDistance = utils::distance(p, q);
            if (tmpDistance < minDistance) {
                _nearestPair = {p, q};
                minDistance = tmpDistance;
                AlgorithmBase_updateCanvasAndBlock();
            }

            edge = edge->next();
        } while (start != edge);
        // Mark cell sa visited.
        visited[it->source_index()] = true;
    }

    _status = AlgorithmStatus::DONE;

    AlgorithmBase_updateCanvasAndBlock();

    emit animationFinished();
}

void NearestPointsVoronoi::drawAlgorithm(QPainter &painter) const {
    // Draw all the points.
    QPen pen = painter.pen();
    pen.setColor(Qt::red);
    pen.setWidth(3);
    painter.setPen(pen);
    for (QPoint p : _points) {
        painter.drawPoint(p);
    }

    if (_status >= AlgorithmStatus::DIAGRAM_CONSTRUCTED) {
        // Draw edges of Voronoi diagram.
        pen = painter.pen();
        pen.setColor(Qt::black);
        pen.setWidth(3);
        painter.setPen(pen);
        drawEdges(painter);

        if (_status == AlgorithmStatus::CHECKING) {
            // Draw currently nearest.
            pen = painter.pen();
            pen.setColor(Qt::blue);
            painter.setPen(pen);
            painter.drawLine(_nearestPair.first, _nearestPair.second);

            // Draw currently checking.
            pen.setColor(Qt::green);
            painter.setPen(pen);
            painter.drawLine(_pairToCheck.first, _pairToCheck.second);
        } else if (_status == AlgorithmStatus::DONE) {
            // Draw nearest.
            pen = painter.pen();
            pen.setColor(Qt::red);
            painter.setPen(pen);
            painter.drawLine(_nearestPair.first, _nearestPair.second);
        }
    }
}

void NearestPointsVoronoi::runNaiveAlgorithm() {
    if (_status == AlgorithmStatus::INVALID_INPUT) {
        return;
    }

    unsigned length = _points.size();
    double minDistance, tmpDistance;

    _nearestPair = {};
    minDistance = std::numeric_limits<double>::max();

    for (unsigned i = 0; i < length; i++) {
        for (unsigned j = i + 1; j < length; j++) {
            // If two points are same, we don't want them to set
            // minimal distance to 0.
            // TODO: Think more about this issue.
            if (_points[i] == _points[j]) {
                continue;
            }
            tmpDistance = utils::distance(_points[i], _points[j]);
            if (tmpDistance < minDistance) {
                _nearestPair = {_points[i], _points[j]};
                minDistance = tmpDistance;
            }
        }
    }

    _status = AlgorithmStatus::DONE;
}

void NearestPointsVoronoi::drawEdges(QPainter &painter) const {
    for (auto it = _diagram.edges().begin(); it != _diagram.edges().end(); ++it) {
        if (it->is_primary()) {
            if (it->is_finite()) {
                painter.drawLine(it->vertex0()->x(), it->vertex0()->y(),
                                 it->vertex1()->x(), it->vertex1()->y());
            } else {
                // Clip infinite edge.
                std::vector<QPoint> samples;
                clipInfiniteEdge(*it, &samples);
                painter.drawLine(samples[0], samples[1]);
            }
        }
    }
}

void NearestPointsVoronoi::clipInfiniteEdge(
        const voronoi_diagram<double>::edge_type& edge,
        std::vector<QPoint>* clipped_edge) const {
    auto cell1 = *edge.cell();
    auto cell2 = *edge.twin()->cell();


    QPoint p1 = _points[cell1.source_index()];
    QPoint p2 = _points[cell2.source_index()];
    QPoint origin((p1.x() + p2.x()) * 0.5, (p1.y() + p2.y()) * 0.5);
    QPoint direction(p1.y() - p2.y(), p2.x() - p1.x());

    // Use value that represents twice a width of the render area.
    double side = 2400 - 2 * getDRAWING_BORDER();
    double koef = side / (std::max)(fabs(direction.x()), fabs(direction.y()));
    if (edge.vertex0() == nullptr) {
        clipped_edge->push_back(QPoint(
          origin.x() - direction.x() * koef,
          origin.y() - direction.y() * koef));
    } else {
        clipped_edge->push_back(
          QPoint(edge.vertex0()->x(), edge.vertex0()->y()));
    }
    if (edge.vertex1() == nullptr) {
        clipped_edge->push_back(QPoint(
          origin.x() + direction.x() * koef,
          origin.y() + direction.y() * koef));
    } else {
        clipped_edge->push_back(
          QPoint(edge.vertex1()->x(), edge.vertex1()->y()));
    }
}

QPair<QPoint, QPoint> NearestPointsVoronoi::nearestPair() const {
    return _nearestPair;
}

NearestPointsVoronoi::AlgorithmStatus NearestPointsVoronoi::status() const {
    return _status;
}

std::vector<QPoint> NearestPointsVoronoi::points() const {
    return _points;
}

void NearestPointsVoronoi::setPoints(const std::vector<QPoint>& points) {
    if (points.size() < 2) {
        _status = AlgorithmStatus::INVALID_INPUT;
    } else if (points.size() == 2) {
        _status = AlgorithmStatus::CORNER_INPUT;
    } else {
        _status = AlgorithmStatus::CORRECT_INPUT;
    }

    _points = points;
}
*/
