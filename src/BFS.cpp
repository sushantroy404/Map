#include "BFS.h"
#include <queue>
#include <chrono>
#include <unordered_set>

// ─────────────────────────────────────────────────────────────────────────────
//  BFS Search Algorithm Solver Implementation
// ─────────────────────────────────────────────────────────────────────────────
PathResult BFS::solve(const Graph& graph, int start, int end, const Callbacks* cb) {
    PathResult result;
    result.algorithmName = name_;

    // If start or end nodes do not exist in the graph, exit early
    if (!graph.hasNode(start) || !graph.hasNode(end)) {
        result.found = false;
        return result;
    }

    // Start wall-clock timer
    auto startTime = std::chrono::high_resolution_clock::now();

    // Core Data Structures
    std::unordered_map<int, double> distances; // Tracks accumulated path weights
    std::unordered_map<int, int> parent;
    std::unordered_set<int> visited;          // Keeps track of processed nodes
    
    // Standard FIFO Queue for tracking node IDs
    std::queue<int> q;

    // Initialize source node
    distances[start] = 0.0;
    visited.insert(start);
    q.push(start);

    while (!q.empty()) {
        int u = q.front();
        q.pop();

        // UI Hook: Node popped from queue (Visited/Explored)
        result.nodesExplored++;
        if (cb && cb->onNodeVisited) {
            cb->onNodeVisited(u, distances[u]);
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

            // BFS explores tier by tier; we skip already visited nodes
            if (visited.find(v) == visited.end()) {
                visited.insert(v);
                parent[v] = u;
                distances[v] = distances[u] + weight;
                
                q.push(v);

                // UI Hook: Edge discovered/relaxed (Frontend tracking visualization)
                if (cb && cb->onEdgeRelaxed) {
                    cb->onEdgeRelaxed(u, v, distances[v]);
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
        result.totalCost = distances[end];
    }

    // UI Hook: Execution complete
    if (cb && cb->onComplete) {
        cb->onComplete(result);
    }

    return result;
}