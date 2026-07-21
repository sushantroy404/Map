#include "Graph.h"
#include <stdexcept>

void Graph::addNode(const Node &node) {
    m_nodes[node.id] = node;
}

void Graph::addEdge(const Edge &edge) {
    m_edges.append(edge);
    
    // Add bidirectional mapping for undirected streets
    m_adjacencyList[edge.sourceId].append({edge.targetId, edge.id, edge.weight});
    m_adjacencyList[edge.targetId].append({edge.sourceId, edge.id, edge.weight});
}

const Node& Graph::getNode(const QString &id) const {
    auto it = m_nodes.find(id);
    if (it != m_nodes.end()) {
        return it.value();
    }
    throw std::runtime_error("Node matching specified ID not found in Graph model.");
}

bool Graph::hasNode(const QString &id) const {
    return m_nodes.contains(id);
}

const QMap<QString, Node>& Graph::getNodes() const {
    return m_nodes;
}

const QVector<Edge>& Graph::getEdges() const {
    return m_edges;
}

const QVector<EdgeAdjacency>& Graph::getNeighbors(const QString &nodeId) const {
    static const QVector<EdgeAdjacency> empty;
    auto it = m_adjacencyList.find(nodeId);
    if (it != m_adjacencyList.end()) {
        return it.value();
    }
    return empty;
}

void Graph::clear() {
    m_nodes.clear();
    m_edges.clear();
    m_adjacencyList.clear();
}
