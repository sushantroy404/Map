#include "EdgeItem.h"
#include <QGraphicsSceneHoverEvent>
#include <QToolTip>

EdgeItem::EdgeItem(const Edge &edge, const QPointF &p1, const QPointF &p2, QGraphicsItem *parent)
    : QGraphicsLineItem(p1.x(), p1.y(), p2.x(), p2.y(), parent), m_edge(edge), m_state(State::Default) {
    setAcceptHoverEvents(true);
    setVisualState(State::Default);
}

void EdgeItem::setVisualState(State state) {
    m_state = state;
    QPen pen;
    double baseWidth = 1.0;

    // Line weight based on highway type
    if (m_edge.type == "primary") {
        baseWidth = 2.0;
        pen.setColor(QColor("#777777"));
    } else if (m_edge.type == "secondary") {
        baseWidth = 1.5;
        pen.setColor(QColor("#999999"));
    } else if (m_edge.type == "footway") {
        baseWidth = 0.8;
        pen.setColor(QColor("#B0BEC5"));
        pen.setStyle(Qt::DashLine);
    } else {
        baseWidth = 1.0;
        pen.setColor(QColor("#CCCCCC"));
    }

    switch (state) {
        case State::Default:
            // Standard roadway coloring
            break;
        case State::Relaxed:
            pen.setColor(QColor("#E6A700"));
            baseWidth += 1.0;
            break;
        case State::ShortestPath:
            pen.setColor(QColor("#4CAF50"));
            pen.setStyle(Qt::SolidLine);
            baseWidth += 2.0;
            break;
    }

    pen.setWidthF(baseWidth);
    setPen(pen);
}

void EdgeItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    QPen hoverPen = pen();
    hoverPen.setColor(QColor("#2E6DA4"));
    setPen(hoverPen);
    
    QToolTip::showText(event->screenPos(), QString("%1\nType: %2\nLength: %3m").arg(m_edge.name).arg(m_edge.type).arg(m_edge.length, 0, 'f', 1));
    QGraphicsLineItem::hoverEnterEvent(event);
}

void EdgeItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    setVisualState(m_state);
    QToolTip::hideText();
    QGraphicsLineItem::hoverLeaveEvent(event);
}
