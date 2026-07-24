#pragma once

#include <QString>
#include <QPointF>
#include <QMap>
#include <QVector>

struct Node {
    QString id;
    double lat;
    double lon;
    QPointF screenPos;
};

struct Edge {
    QString id;
    QString sourceId;
    QString targetId;
    double weight;
    double length; // meters
    QString name;
    QString type;  // e.g. primary, secondary, residential, footway
};

struct EdgeAdjacency {
    QString targetId;
    QString edgeId;
    double weight;
};

class Graph {
public:
    Graph() = default;
    ~Graph() = default;

    void addNode(const Node &node);
    void addEdge(const Edge &edge);
    
    const Node& getNode(const QString &id) const;
    bool hasNode(const QString &id) const;
    const QMap<QString, Node>& getNodes() const;
    const QVector<Edge>& getEdges() const;
    const QVector<EdgeAdjacency>& getNeighbors(const QString &nodeId) const;
    
    void clear();

private:
    QMap<QString, Node> m_nodes;
    QVector<Edge> m_edges;
    QMap<QString, QVector<EdgeAdjacency>> m_adjacencyList;
};
