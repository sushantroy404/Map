#include "Dijkstra.h"
#include <queue>
#include <chrono>

PathResult Dijkstra::solve(const Graph& graph, int start, int end, const Callbacks* cb) {
    PathResult result;
    result.algorithmName = name_;
    
    // 1. Start Wall-Clock Timer
    auto startTime = std::chrono::high_resolution_clock::now();

    // 2. Core Data Structures
    std::unordered_map<int, double> distances;
    std::unordered_map<int, int> parent;
    
    // Min-heap tracking: pair<distance, nodeId>
    using MinHeapPair = std::pair<double, int>;
    std::priority_queue<MinHeapPair, std::vector<MinHeapPair>, std::greater<MinHeapPair>> pq;

    // Initialize source node
    distances[start] = 0.0;
    pq.push({0.0, start});

    while (!pq.empty()) {
        // FIX 1: Replaced C++17 structured binding with C++11 standard pair unpacking
        auto topPair = pq.top();
        pq.pop();
        double currentDist = topPair.first;  // The distance accumulated
        int u = topPair.second;              // The current node ID

        // UI Hook: Node popped from priority queue (Visited/Explored)
        result.nodesExplored++;
        if (cb && cb->onNodeVisited) {
            cb->onNodeVisited(u, currentDist);
        }

        // Short-circuit check if destination is reached
        if (u == end) {
            result.found = true;
            break;
        }

        // If we found a worse path to this node already, skip it
        if (currentDist > distances[u]) continue;

        // FIX 2: Using graph.neighbors(u) matching Graph.h exactly
        for (const auto& edge : graph.neighbors(u)) { 
            int v = edge.to; 
            double weight = edge.weight;
            double newDist = currentDist + weight;

            // If neighbor 'v' hasn't been discovered, or a shorter path is found
            if (distances.find(v) == distances.end() || newDist < distances[v]) {
                distances[v] = newDist;
                parent[v] = u;
                pq.push({newDist, v});

                // UI Hook: Edge relaxed (Frontend animates the path highlight)
                if (cb && cb->onEdgeRelaxed) {
                    cb->onEdgeRelaxed(u, v, newDist);
                }
            }
        }
    }

    // 3. Stop Wall-Clock Timer
    auto endTime = std::chrono::high_resolution_clock::now();
    result.timeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    // 4. Construct Final Path Metrics
    if (result.found) {
        result.path = reconstructPath(parent, start, end);
        result.totalCost = distances[end];
    }

    // UI Hook: Search completely finished
    if (cb && cb->onComplete) {
        cb->onComplete(result);
    }

    return result;
}