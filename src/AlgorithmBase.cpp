#include "AlgorithmBase.h"
#include <algorithm>

std::vector<int> AlgorithmBase::reconstructPath(
    const std::unordered_map<int, int>& parent, 
    int start, int end) const {
    
    std::vector<int> path;
    if (parent.find(end) == parent.end() && start != end) {
        return path; // Return empty if no path exists
    }

    // Walk backwards from target to source
    for (int at = end; at != start; at = parent.at(at)) {
        path.push_back(at);
    }
    path.push_back(start);

    // Reverse to get start -> end order
    std::reverse(path.begin(), path.end());
    return path;
}