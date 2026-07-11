#include "Algorithms.h"
#include <queue>
#include <cmath>

// Helper to resolve final shortest-path edges for visual tracking
static QSet<QString> reconstructPath(const QMap<QString, QString> &parentMap, const QString &startId, const QString &endId, const QVector<Edge> &edges) {
    QSet<QString> pathEdges;
    QString current = endId;
    while (current != startId) {
        QString parent = parentMap.value(current);
        if (parent.isEmpty()) break;

        for (const Edge &e : edges) {
            if ((e.sourceId == current && e.targetId == parent) || (e.sourceId == parent && e.targetId == current)) {
                pathEdges.insert(e.id);
                break;
            }
        }
        current = parent;
    }
    return pathEdges;
}

// Struct comparator for min-priority queue
struct PQLess {
    bool operator()(const QueueEntry &a, const QueueEntry &b) const {
        return a.priority > b.priority;
    }
};

std::vector<PathfindingStep> DijkstraAlgorithm::solve(const Graph &graph, const QString &startId, const QString &endId) {
    std::vector<PathfindingStep> steps;
    int stepIdx = 0;

    QMap<QString, double> dist;
    QMap<QString, QString> parent;
    QSet<QString> visited;
    
    // Custom PQ matching UI struct representation
    std::priority_queue<QueueEntry, std::vector<QueueEntry>, PQLess> pq;

    for (const auto &id : graph.getNodes().keys()) {
        dist[id] = std::numeric_limits<double>::infinity();
    }
    dist[startId] = 0.0;
    pq.push({startId, 0.0});

    while (!pq.empty()) {
        QueueEntry curr = pq.top();
        pq.pop();

        QString u = curr.nodeId;
        if (visited.contains(u)) continue;
        visited.insert(u);

        // Package Priority Queue entries for logging
        std::vector<QueueEntry> frontier;
        auto tempPq = pq;
        while(!tempPq.empty()){
            frontier.push_back(tempPq.top());
            tempPq.pop();
        }

        steps.push_back({
            PathfindingStep::Type::Visit,
            ++stepIdx,
            u,
            "",
            visited,
            dist,
            frontier,
            parent,
            QString("Visiting intersection %1. Distance: %2m").arg(u).arg(dist[u], 0, 'f', 1),
            0.0
        });

        if (u == endId) {
            steps.push_back({
                PathfindingStep::Type::PathFound,
                ++stepIdx,
                u,
                "",
                visited,
                dist,
                frontier,
                parent,
                "Goal achieved!",
                dist[endId]
            });
            // Tack on the final reconstructed path
            steps.back().totalDistance = dist[endId];
            return steps;
        }

        for (const auto &adj : graph.getNeighbors(u)) {
            if (visited.contains(adj.targetId)) continue;

            double alt = dist[u] + adj.weight;
            if (alt < dist[adj.targetId]) {
                dist[adj.targetId] = alt;
                parent[adj.targetId] = u;
                pq.push({adj.targetId, alt});

                steps.push_back({
                    PathfindingStep::Type::Relaxation,
                    ++stepIdx,
                    u,
                    adj.edgeId,
                    visited,
                    dist,
                    frontier,
                    parent,
                    QString("Relaxing: %1 -> %2. Total: %3m").arg(u).arg(adj.targetId).arg(alt, 0, 'f', 1),
                    0.0
                });
            }
        }
    }

    steps.push_back({
        PathfindingStep::Type::NoPath,
        ++stepIdx,
        startId,
        "",
        visited,
        dist,
        {},
        parent,
        "No continuous path exists between start and destination.",
        -1.0
    });
    return steps;
}

// Coordinate Distance Haversine
static double haversine(const Node &n1, const Node &n2) {
    const double R = 6371000.0;
    double dLat = (n2.lat - n1.lat) * M_PI / 180.0;
    double dLon = (n2.lon - n1.lon) * M_PI / 180.0;
    double a = std::sin(dLat/2) * std::sin(dLat/2) +
               std::cos(n1.lat * M_PI/180.0) * std::cos(n2.lat * M_PI/180.0) *
               std::sin(dLon/2) * std::sin(dLon/2);
    return R * 2 * std::atan2(std::sqrt(a), std::sqrt(1-a));
}

std::vector<PathfindingStep> AStarAlgorithm::solve(const Graph &graph, const QString &startId, const QString &endId) {
    std::vector<PathfindingStep> steps;
    int stepIdx = 0;

    QMap<QString, double> gScore;
    QMap<QString, double> fScore;
    QMap<QString, QString> parent;
    QSet<QString> visited;

    std::priority_queue<QueueEntry, std::vector<QueueEntry>, PQLess> pq;

    for (const auto &id : graph.getNodes().keys()) {
        gScore[id] = std::numeric_limits<double>::infinity();
        fScore[id] = std::numeric_limits<double>::infinity();
    }

    gScore[startId] = 0.0;
    double initialH = haversine(graph.getNode(startId), graph.getNode(endId));
    fScore[startId] = initialH;
    pq.push({startId, initialH});

    while (!pq.empty()) {
        QueueEntry curr = pq.top();
        pq.pop();

        QString u = curr.nodeId;
        if (visited.contains(u)) continue;
        visited.insert(u);

        std::vector<QueueEntry> frontier;
        auto tempPq = pq;
        while(!tempPq.empty()){
            frontier.push_back(tempPq.top());
            tempPq.pop();
        }

        steps.push_back({
            PathfindingStep::Type::Visit,
            ++stepIdx,
            u,
            "",
            visited,
            gScore,
            frontier,
            parent,
            QString("A* Visit: %1. g=%2m, f=%3m").arg(u).arg(gScore[u], 0, 'f', 1).arg(fScore[u], 0, 'f', 1),
            0.0
        });

        if (u == endId) {
            steps.push_back({
                PathfindingStep::Type::PathFound,
                ++stepIdx,
                u,
                "",
                visited,
                gScore,
                frontier,
                parent,
                "A* search finished successfully.",
                gScore[endId]
            });
            return steps;
        }

        for (const auto &adj : graph.getNeighbors(u)) {
            if (visited.contains(adj.targetId)) continue;

            double tentativeG = gScore[u] + adj.weight;
            if (tentativeG < gScore[adj.targetId]) {
                gScore[adj.targetId] = tentativeG;
                parent[adj.targetId] = u;

                double h = haversine(graph.getNode(adj.targetId), graph.getNode(endId));
                double f = tentativeG + h;
                fScore[adj.targetId] = f;
                
                pq.push({adj.targetId, f});

                steps.push_back({
                    PathfindingStep::Type::Relaxation,
                    ++stepIdx,
                    u,
                    adj.edgeId,
                    visited,
                    gScore,
                    frontier,
                    parent,
                    QString("Relaxing: g=%1m, h=%2m, f=%3m").arg(tentativeG, 0, 'f', 1).arg(h, 0, 'f', 1).arg(f, 0, 'f', 1),
                    0.0
                });
            }
        }
    }

    steps.push_back({
        PathfindingStep::Type::NoPath,
        ++stepIdx,
        startId,
        "",
        visited,
        gScore,
        {},
        parent,
        "No pathway connects the chosen points.",
        -1.0
    });
    return steps;
}

std::vector<PathfindingStep> BFSAlgorithm::solve(const Graph &graph, const QString &startId, const QString &endId) {
    std::vector<PathfindingStep> steps;
    int stepIdx = 0;

    QMap<QString, double> hops;
    QMap<QString, QString> parent;
    QSet<QString> visited;
    std::queue<QString> q;

    for (const auto &id : graph.getNodes().keys()) {
        hops[id] = std::numeric_limits<double>::infinity();
    }

    visited.insert(startId);
    hops[startId] = 0.0;
    q.push(startId);

    while (!q.empty()) {
        QString u = q.front();
        q.pop();

        std::vector<QueueEntry> frontier;
        auto tempQ = q;
        while(!tempQ.empty()) {
            frontier.push_back({tempQ.front(), 0.0});
            tempQ.pop();
        }

        steps.push_back({
            PathfindingStep::Type::Visit,
            ++stepIdx,
            u,
            "",
            visited,
            hops,
            frontier,
            parent,
            QString("BFS visiting: Node %1. Hops: %2").arg(u).arg(hops[u]),
            0.0
        });

        if (u == endId) {
            // Traverse real metric distance to log path
            double realDist = 0.0;
            QString current = endId;
            while (current != startId) {
                QString p = parent.value(current);
                if (p.isEmpty()) break;
                for (const auto &adj : graph.getNeighbors(current)) {
                    if (adj.targetId == p) {
                        realDist += adj.weight;
                        break;
                    }
                }
                current = p;
            }

            steps.push_back({
                PathfindingStep::Type::PathFound,
                ++stepIdx,
                u,
                "",
                visited,
                hops,
                frontier,
                parent,
                "BFS goal hit!",
                realDist
            });
            return steps;
        }

        for (const auto &adj : graph.getNeighbors(u)) {
            if (!visited.contains(adj.targetId)) {
                visited.insert(adj.targetId);
                hops[adj.targetId] = hops[u] + 1;
                parent[adj.targetId] = u;
                q.push(adj.targetId);

                steps.push_back({
                    PathfindingStep::Type::Relaxation,
                    ++stepIdx,
                    u,
                    adj.edgeId,
                    visited,
                    hops,
                    frontier,
                    parent,
                    QString("Queued Neighbor Node %1").arg(adj.targetId),
                    0.0
                });
            }
        }
    }

    steps.push_back({
        PathfindingStep::Type::NoPath,
        ++stepIdx,
        startId,
        "",
        visited,
        hops,
        {},
        parent,
        "Destination could not be reached via BFS.",
        -1.0
    });
    return steps;
}
