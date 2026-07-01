#include "appcontroller.h"
#include "OsmLoader.h"
#include "Algorithms.h"

#include <QFileDialog>
#include <QElapsedTimer>
#include <QVariantMap>
#include <limits>
#include <memory>

AppController::AppController(QObject *parent)
    : QObject(parent),
    m_animationController(new AnimationController(this)) {
    connect(
        m_animationController,
        &AnimationController::stepRendered,
        this,
        &AppController::updateForStep
        );
}

QVariantList AppController::nodes() const {
    return m_nodes;
}

QVariantList AppController::edges() const {
    return m_edges;
}

QVariantList AppController::startNodes() const {
    return m_nodes;
}

QVariantList AppController::endNodes() const {
    return m_nodes;
}

QString AppController::startNodeId() const {
    return m_startNodeId;
}

void AppController::setStartNodeId(const QString &id) {
    if (m_startNodeId == id) return;
    m_startNodeId = id;
    emit selectionChanged();
}

QString AppController::endNodeId() const {
    return m_endNodeId;
}

void AppController::setEndNodeId(const QString &id) {
    if (m_endNodeId == id) return;
    m_endNodeId = id;
    emit selectionChanged();
}

int AppController::speed() const {
    return m_speed;
}

void AppController::setSpeed(int speed) {
    if (m_speed == speed) return;
    m_speed = speed;
    m_animationController->setSpeed(speed);
    emit speedChanged();
}

QString AppController::activeAlgorithm() const {
    return m_activeAlgorithm;
}

QString AppController::currentNode() const {
    return m_currentNode;
}

int AppController::visitedCount() const {
    return m_visitedCount;
}

QString AppController::totalDistance() const {
    return m_totalDistance;
}

int AppController::executionTime() const {
    return m_executionTime;
}

int AppController::currentStep() const {
    return m_currentStep;
}

QVariantList AppController::frontier() const {
    return m_frontier;
}

QVariantList AppController::distanceTable() const {
    return m_distanceTable;
}

QVariantList AppController::logs() const {
    return m_logs;
}

void AppController::importOsm() {
    QString fileName = QFileDialog::getOpenFileName(
        nullptr,
        "Select OpenStreetMap File",
        "",
        "OSM Files (*.osm);;XML Files (*.xml);;All Files (*)"
        );

    if (fileName.isEmpty()) return;

    OsmLoader loader;
    QString error;

    if (!loader.load(fileName, m_graph, error)) {
        return;
    }

    rebuildVisualData();

    if (!m_nodes.isEmpty()) {
        m_startNodeId = m_nodes.first().toMap().value("id").toString();
        m_endNodeId = m_nodes.last().toMap().value("id").toString();
        emit selectionChanged();
    }

    emit graphChanged();
}

void AppController::rebuildVisualData() {
    m_nodes.clear();
    m_edges.clear();

    for (const Node &node : m_graph.getNodes()) {
        QVariantMap item;
        item["id"] = node.id;
        item["label"] = QString("Intersection %1 (Lat: %2, Lon: %3)")
                            .arg(node.id)
                            .arg(node.lat, 0, 'f', 4)
                            .arg(node.lon, 0, 'f', 4);
        item["lat"] = node.lat;
        item["lon"] = node.lon;
        item["x"] = node.screenPos.x();
        item["y"] = node.screenPos.y();
        item["state"] = "default";
        m_nodes.append(item);
    }

    for (const Edge &edge : m_graph.getEdges()) {
        const Node &source = m_graph.getNode(edge.sourceId);
        const Node &target = m_graph.getNode(edge.targetId);

        QVariantMap item;
        item["id"] = edge.id;
        item["sourceId"] = edge.sourceId;
        item["targetId"] = edge.targetId;
        item["name"] = edge.name;
        item["type"] = edge.type;
        item["length"] = edge.length;
        item["x1"] = source.screenPos.x();
        item["y1"] = source.screenPos.y();
        item["x2"] = target.screenPos.x();
        item["y2"] = target.screenPos.y();
        item["state"] = "default";
        m_edges.append(item);
    }
}

void AppController::runAlgorithm(const QString &algorithmName) {
    if (m_startNodeId.isEmpty() || m_endNodeId.isEmpty()) return;

    std::shared_ptr<PathfindingAlgorithm> algorithm;

    if (algorithmName.startsWith("Dijkstra")) {
        algorithm = std::make_shared<DijkstraAlgorithm>();
    } else if (algorithmName.startsWith("A*")) {
        algorithm = std::make_shared<AStarAlgorithm>();
    } else {
        algorithm = std::make_shared<BFSAlgorithm>();
    }

    m_activeAlgorithm = algorithmName;
    clearPath();

    QElapsedTimer timer;
    timer.start();

    std::vector<PathfindingStep> steps =
        algorithm->solve(m_graph, m_startNodeId, m_endNodeId);

    m_executionTime = static_cast<int>(timer.elapsed());

    m_animationController->setSteps(steps, m_speed);
    m_animationController->start();

    emit algorithmStateChanged();
}

void AppController::pauseAlgorithm() {
    m_animationController->pause();
}

void AppController::resumeAlgorithm() {
    m_animationController->resume();
}

void AppController::reset() {
    m_animationController->stop();
    clearPath();
}

void AppController::clearPath() {
    m_frontier.clear();
    m_distanceTable.clear();
    m_logs.clear();

    m_currentStep = 0;
    m_currentNode = "--";
    m_visitedCount = 0;
    m_totalDistance = "--";

    for (QVariant &nodeVariant : m_nodes) {
        QVariantMap node = nodeVariant.toMap();
        node["state"] = "default";
        nodeVariant = node;
    }

    for (QVariant &edgeVariant : m_edges) {
        QVariantMap edge = edgeVariant.toMap();
        edge["state"] = "default";
        edgeVariant = edge;
    }

    emit graphChanged();
    emit algorithmStateChanged();
}

void AppController::updateForStep(const PathfindingStep &step) {
    m_currentStep = step.stepIndex;
    m_currentNode = step.currentNodeId;
    m_visitedCount = step.visitedNodes.size();

    m_logs.prepend(QString("[%1] %2").arg(step.stepIndex).arg(step.logMessage));

    m_frontier.clear();
    for (const QueueEntry &entry : step.frontier) {
        QVariantMap item;
        item["nodeId"] = entry.nodeId;
        item["priority"] = QString::number(entry.priority, 'f', 1);
        m_frontier.append(item);
    }

    m_distanceTable.clear();
    for (auto it = step.distanceTable.begin(); it != step.distanceTable.end(); ++it) {
        if (it.value() == std::numeric_limits<double>::infinity()) continue;

        QVariantMap item;
        item["nodeId"] = it.key();
        item["distance"] = QString::number(it.value(), 'f', 1);
        m_distanceTable.append(item);
    }

    for (QVariant &nodeVariant : m_nodes) {
        QVariantMap node = nodeVariant.toMap();
        QString id = node["id"].toString();

        if (id == m_startNodeId) {
            node["state"] = "start";
        } else if (id == m_endNodeId) {
            node["state"] = "end";
        } else if (id == step.currentNodeId) {
            node["state"] = "active";
        } else if (step.visitedNodes.contains(id)) {
            node["state"] = "visited";
        } else {
            node["state"] = "default";
        }

        nodeVariant = node;
    }

    for (QVariant &edgeVariant : m_edges) {
        QVariantMap edge = edgeVariant.toMap();
        QString edgeId = edge["id"].toString();

        if (step.finalPathEdges.contains(edgeId)) {
            edge["state"] = "shortestPath";
        } else if (step.activeEdgeId == edgeId) {
            edge["state"] = "relaxed";
        } else {
            edge["state"] = "default";
        }

        edgeVariant = edge;
    }

    if (step.type == PathfindingStep::Type::PathFound) {
        m_totalDistance = QString("%1 m").arg(step.totalDistance, 0, 'f', 1);
    } else if (step.type == PathfindingStep::Type::NoPath) {
        m_totalDistance = "Unreachable";
    }

    emit graphChanged();
    emit algorithmStateChanged();
}
void AppController::openProject() {
}

void AppController::saveProject() {
}

void AppController::exportScreenshot() {
}