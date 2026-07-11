#pragma once

#include <QGraphicsEllipseItem>
#include <QPen>
#include <QBrush>
#include "Graph.h"

class QGraphicsSceneMouseEvent;

class NodeItem : public QGraphicsEllipseItem {
public:
    enum class State {
        Default,
        Start,
        End,
        Visited,
        Active
    };

    explicit NodeItem(const Node &node, QGraphicsItem *parent = nullptr);
    ~NodeItem() = default;

    QString getNodeId() const { return m_node.id; }
    void setVisualState(State state);

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    Node m_node;
    State m_state;
};
