#pragma once

#include <QString>
#include "Graph.h"

class OsmLoader {
public:
    OsmLoader() = default;
    ~OsmLoader() = default;

    // Returns true on success, populates Graph with nodes and highway networks
    bool load(const QString &filePath, Graph &graph, QString &errorMessage);
};
