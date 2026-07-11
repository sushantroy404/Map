#pragma once

#include <QObject>
#include <QTimer>
#include <vector>
#include "Algorithms.h"

class AnimationController : public QObject {
    Q_OBJECT

public:
    explicit AnimationController(QObject *parent = nullptr);
    ~AnimationController() = default;

    void setSteps(const std::vector<PathfindingStep> &steps, int msSpeed);
    void start();
    void pause();
    void resume();
    void stop();
    void setSpeed(int msSpeed);

signals:
    void stepRendered(const PathfindingStep &step);

private slots:
    void processNextStep();

private:
    QTimer *m_timer;
    std::vector<PathfindingStep> m_steps;
    size_t m_currentIdx;
    int m_speed;
    bool m_isRunning;
    QSet<QString> m_shortestPathEdges;
};
