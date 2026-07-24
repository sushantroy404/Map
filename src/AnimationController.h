#pragma once

#include <QObject>
#include <QTimer>
#include <QSet>
#include <vector>
#include "Algorithms.h"

class AnimationController : public QObject {
    Q_OBJECT

public:
    explicit AnimationController(QObject *parent = nullptr);
    ~AnimationController() = default;

    void setSteps(const std::vector<PathfindingStep> &steps, int msSpeed);
    void setComparisonSteps(const std::vector<PathfindingStep> &dijkstra,
                            const std::vector<PathfindingStep> &astar,
                            const std::vector<PathfindingStep> &bfs,
                            int msSpeed);
    void start();
    void pause();
    void resume();
    void stop();
    void setSpeed(int msSpeed);

signals:
    void stepRendered(const PathfindingStep &step);
    void comparisonStepsRendered(const PathfindingStep &dijkstraStep,
                                 const PathfindingStep &astarStep,
                                 const PathfindingStep &bfsStep);

private slots:
    void processNextStep();

private:
    QTimer *m_timer;
    std::vector<PathfindingStep> m_steps;
    size_t m_currentIdx;

    // Comparison mode state
    bool m_isComparisonMode;
    std::vector<PathfindingStep> m_stepsDijkstra;
    std::vector<PathfindingStep> m_stepsAStar;
    std::vector<PathfindingStep> m_stepsBFS;
    size_t m_idxDijkstra;
    size_t m_idxAStar;
    size_t m_idxBFS;

    int m_speed;
    bool m_isRunning;
    QSet<QString> m_shortestPathEdges;
    QSet<QString> m_shortestPathEdgesDijkstra;
    QSet<QString> m_shortestPathEdgesAStar;
    QSet<QString> m_shortestPathEdgesBFS;
};
