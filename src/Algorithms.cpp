#include "Dijkstra.h"
#include "AStar.h"
#include "BFS.h"
#include <memory>
#include <string>

/**
 * Single Gateway Interface function connecting the frontend API layer
 * directly to your modular polymorphic classes.
 */
PathResult executeAlgorithm(const std::string& algoName, 
                            const Graph& graph, 
                            int start, int end, 
                            const AlgorithmBase::Callbacks* cb = nullptr) {
    
    std::unique_ptr<AlgorithmBase> strategy = nullptr;

    if (algoName == "Dijkstra") {
        strategy = std::make_unique<Dijkstra>();
    } 
    else if (algoName == "AStar") {
        strategy = std::make_unique<AStar>();
    } 
    else if (algoName == "BFS") {
        strategy = std::make_unique<BFS>();
    }

    if (strategy) {
        return strategy->solve(graph, start, end, cb);
    }

    // Return empty fallback structurally if string misaligned
    PathResult failureResult;
    failureResult.algorithmName = "Unknown";
    failureResult.found = false;
    return failureResult;
}