#include "NodeItem.h"
#include <QGraphicsSceneHoverEvent>
#include <QToolTip>

NodeItem::NodeItem(const Node &node, QGraphicsItem *parent)
    : QGraphicsEllipseItem(-4, -4, 8, 8, parent), m_node(node), m_state(State::Default) {
    setPos(node.screenPos);
    setAcceptHoverEvents(true);
    setVisualState(State::Default);
}

void NodeItem::setVisualState(State state) {
    m_state = state;
    QPen pen;
    QBrush brush;

    pen.setWidthF(1.2);
    
    switch (state) {
        case State::Default:
            pen.setColor(QColor("#D8D8D8"));
            brush.setColor(QColor("#FFFFFF"));
            break;
        case State::Start:
            pen.setColor(QColor("#4CAF50"));
            brush.setColor(QColor("#E8F5E9"));
            break;
        case State::End:
            pen.setColor(QColor("#D9534F"));
            brush.setColor(QColor("#FFEBEE"));
            break;
        case State::Visited:
            pen.setColor(QColor("#2E6DA4"));
            brush.setColor(QColor("#D9ECFF"));
            break;
        case State::Active:
            pen.setColor(QColor("#E6A700"));
            brush.setColor(QColor("#FFF8E1"));
            break;
    }

    setPen(pen);
    setBrush(brush);
}

void NodeItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    setPen(QPen(QColor("#2E6DA4"), 2.0));
    QToolTip::showText(event->screenPos(), QString("Intersection %1\nLat: %2\nLon: %3").arg(m_node.id).arg(m_node.lat, 0, 'f', 5).arg(m_node.lon, 0, 'f', 5));
    QGraphicsEllipseItem::hoverEnterEvent(event);
}

void NodeItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    setVisualState(m_state);
    QToolTip::hideText();
    QGraphicsEllipseItem::hoverLeaveEvent(event);
}
