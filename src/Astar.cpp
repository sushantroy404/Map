#include "AStar.h"
#include <queue>
#include <chrono>
#include <cmath>

// ─────────────────────────────────────────────────────────────────────────────
//  Heuristic Calculation Helper
// ─────────────────────────────────────────────────────────────────────────────
double AStar::h(const Node& a, const Node& goal) const {
    if (heuristic_ == Heuristic::ZERO) {
        return 0.0;
    }

    double dx = a.x - goal.x;
    double dy = a.y - goal.y;

    if (heuristic_ == Heuristic::MANHATTAN) {
        return std::abs(dx) + std::abs(dy);
    }
    
    // Default: Heuristic::EUCLIDEAN
    return std::sqrt(dx * dx + dy * dy);
}

// ─────────────────────────────────────────────────────────────────────────────
//  A* Search Algorithm Solver Implementation
// ─────────────────────────────────────────────────────────────────────────────
PathResult AStar::solve(const Graph& graph, int start, int end, const Callbacks* cb) {
    PathResult result;
    result.algorithmName = name_;

    // If start or end nodes do not exist in the graph, exit early
    if (!graph.hasNode(start) || !graph.hasNode(end)) {
        result.found = false;
        return result;
    }

    // Start wall-clock timer
    auto startTime = std::chrono::high_resolution_clock::now();

    const Node& goalNode = graph.getNode(end);

    // Core Data Structures
    std::unordered_map<int, double> gScore; // Exact cost from start to current node
    std::unordered_map<int, int> parent;

    // Min-heap tracking: pair<fScore, nodeId>
    using MinHeapPair = std::pair<double, int>;
    std::priority_queue<MinHeapPair, std::vector<MinHeapPair>, std::greater<MinHeapPair>> pq;

    // Initialize source node
    gScore[start] = 0.0;
    double initialH = h(graph.getNode(start), goalNode);
    pq.push({initialH, start}); // fScore = gScore + hScore

    while (!pq.empty()) {
        auto topPair = pq.top();
        pq.pop();
        double currentF = topPair.first;
        int u = topPair.second;

        // UI Hook: Node popped from priority queue (Visited/Explored)
        result.nodesExplored++;
        if (cb && cb->onNodeVisited) {
            // Pass the accurate actual cost (gScore) up to the UI binding layer
            cb->onNodeVisited(u, gScore[u]);
        }

        // Target short-circuit check
        if (u == end) {
            result.found = true;
            break;
        }

        // Loop through all outgoing connections of node 'u'
        for (const auto& edge : graph.neighbors(u)) {
            int v = edge.to;
            double weight = edge.weight;
            double tentativeG = gScore[u] + weight;

            // If neighbor 'v' hasn't been discovered, or a shorter actual path is found
            if (gScore.find(v) == gScore.end() || tentativeG < gScore[v]) {
                gScore[v] = tentativeG;
                parent[v] = u;
                
                // fScore = g(v) + h(v)
                double fScore = tentativeG + h(graph.getNode(v), goalNode);
                pq.push({fScore, v});

                // UI Hook: Edge relaxed (Frontend tracking visualization)
                if (cb && cb->onEdgeRelaxed) {
                    cb->onEdgeRelaxed(u, v, tentativeG);
                }
            }
        }
    }

    // Stop wall-clock timer
    auto endTime = std::chrono::high_resolution_clock::now();
    result.timeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    // Construct final tracking metrics if a valid route was found
    if (result.found) {
        result.path = reconstructPath(parent, start, end);
        result.totalCost = gScore[end];
    }

    // UI Hook: Execution complete
    if (cb && cb->onComplete) {
        cb->onComplete(result);
    }

    return result;
}