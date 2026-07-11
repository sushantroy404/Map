#pragma once

#include <vector>
#include <QString>
#include <QMap>
#include <QSet>
#include "Graph.h"

struct QueueEntry {
    QString nodeId;
    double priority;
};

struct PathfindingStep {
    enum class Type {
        Initialization,
        Visit,
        Relaxation,
        PathFound,
        NoPath
    };

    Type type;
    int stepIndex;
    QString currentNodeId;
    QString activeEdgeId;
    QSet<QString> visitedNodes;
    QMap<QString, double> distanceTable;
    std::vector<QueueEntry> frontier; // Priority Queue contents
    QMap<QString, QString> parentMap;
    QString logMessage;
    double totalDistance;
    QSet<QString> finalPathEdges = {};
};

class PathfindingAlgorithm {
public:
    virtual ~PathfindingAlgorithm() = default;
    virtual std::vector<PathfindingStep> solve(const Graph &graph, const QString &startId, const QString &endId) = 0;
};

class DijkstraAlgorithm : public PathfindingAlgorithm {
public:
    std::vector<PathfindingStep> solve(const Graph &graph, const QString &startId, const QString &endId) override;
};

class AStarAlgorithm : public PathfindingAlgorithm {
public:
    std::vector<PathfindingStep> solve(const Graph &graph, const QString &startId, const QString &endId) override;
};

class BFSAlgorithm : public PathfindingAlgorithm {
public:
    std::vector<PathfindingStep> solve(const Graph &graph, const QString &startId, const QString &endId) override;
};
