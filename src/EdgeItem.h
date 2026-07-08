#pragma once

#include <QGraphicsLineItem>
#include <QPen>
#include "Graph.h"

class EdgeItem : public QGraphicsLineItem {
public:
    enum class State {
        Default,
        Relaxed,
        ShortestPath
    };

    EdgeItem(const Edge &edge, const QPointF &p1, const QPointF &p2, QGraphicsItem *parent = nullptr);
    ~EdgeItem() = default;

    QString getEdgeId() const { return m_edge.id; }
    void setVisualState(State state);

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    Edge m_edge;
    State m_state;
};
