#include "AnimationController.h"

AnimationController::AnimationController(QObject *parent)
    : QObject(parent), m_timer(new QTimer(this)), m_currentIdx(0), m_isComparisonMode(false),
      m_idxDijkstra(0), m_idxAStar(0), m_idxBFS(0), m_speed(100), m_isRunning(false) {
    connect(m_timer, &QTimer::timeout, this, &AnimationController::processNextStep);
}

void AnimationController::setSteps(const std::vector<PathfindingStep> &steps, int msSpeed) {
    m_isComparisonMode = false;
    m_steps = steps;
    m_speed = msSpeed;
    m_currentIdx = 0;
    
    // Resolve final path structure prior to starting animation
    m_shortestPathEdges.clear();
    if (!m_steps.empty() && m_steps.back().type == PathfindingStep::Type::PathFound) {
        const auto &lastStep = m_steps.back();
        QString current = lastStep.currentNodeId;
        
        // Loop back up parents
        while (!current.isEmpty()) {
            QString parent = lastStep.parentMap.value(current);
            if (parent.isEmpty()) break;
            
            // Generate a simple key for shortest edge tracing
            m_shortestPathEdges.insert(current + "_" + parent);
            m_shortestPathEdges.insert(parent + "_" + current);
            
            current = parent;
        }
    }
}

void AnimationController::setComparisonSteps(const std::vector<PathfindingStep> &dijkstra,
                                             const std::vector<PathfindingStep> &astar,
                                             const std::vector<PathfindingStep> &bfs,
                                             int msSpeed) {
    m_isComparisonMode = true;
    m_stepsDijkstra = dijkstra;
    m_stepsAStar = astar;
    m_stepsBFS = bfs;
    m_speed = msSpeed;
    m_idxDijkstra = 0;
    m_idxAStar = 0;
    m_idxBFS = 0;

    auto resolveShortestPath = [](const std::vector<PathfindingStep> &steps, QSet<QString> &shortest) {
        shortest.clear();
        if (!steps.empty() && steps.back().type == PathfindingStep::Type::PathFound) {
            const auto &lastStep = steps.back();
            QString current = lastStep.currentNodeId;
            while (!current.isEmpty()) {
                QString parent = lastStep.parentMap.value(current);
                if (parent.isEmpty()) break;
                shortest.insert(current + "_" + parent);
                shortest.insert(parent + "_" + current);
                current = parent;
            }
        }
    };

    resolveShortestPath(m_stepsDijkstra, m_shortestPathEdgesDijkstra);
    resolveShortestPath(m_stepsAStar, m_shortestPathEdgesAStar);
    resolveShortestPath(m_stepsBFS, m_shortestPathEdgesBFS);
}

void AnimationController::start() {
    if (!m_isComparisonMode) {
        if (m_steps.empty()) return;
        m_currentIdx = 0;
    } else {
        if (m_stepsDijkstra.empty() && m_stepsAStar.empty() && m_stepsBFS.empty()) return;
        m_idxDijkstra = 0;
        m_idxAStar = 0;
        m_idxBFS = 0;
    }
    m_isRunning = true;
    m_timer->start(m_speed);
}

void AnimationController::pause() {
    m_isRunning = false;
    m_timer->stop();
}

void AnimationController::resume() {
    if (!m_isComparisonMode) {
        if (m_steps.empty() || m_currentIdx >= m_steps.size()) return;
    } else {
        if (m_idxDijkstra >= m_stepsDijkstra.size() && m_idxAStar >= m_stepsAStar.size() && m_idxBFS >= m_stepsBFS.size()) return;
    }
    m_isRunning = true;
    m_timer->start(m_speed);
}

void AnimationController::stop() {
    m_isRunning = false;
    m_timer->stop();
    m_currentIdx = 0;
    m_idxDijkstra = 0;
    m_idxAStar = 0;
    m_idxBFS = 0;
}

void AnimationController::setSpeed(int msSpeed) {
    m_speed = msSpeed;
    if (m_isRunning) {
        m_timer->start(m_speed);
    }
}

void AnimationController::processNextStep() {
    if (!m_isComparisonMode) {
        if (m_currentIdx >= m_steps.size()) {
            m_timer->stop();
            m_isRunning = false;
            return;
        }

        PathfindingStep step = m_steps[m_currentIdx];
        step.stepIndex = static_cast<int>(m_currentIdx);

        QSet<QString> stepPathEdges;
        if (step.type == PathfindingStep::Type::PathFound || m_currentIdx == m_steps.size() - 1) {
            stepPathEdges = m_shortestPathEdges;
        }
        step.finalPathEdges = stepPathEdges;

        emit stepRendered(step);
        m_currentIdx++;
    } else {
        bool anyRunning = false;
        PathfindingStep stepDijkstra;
        PathfindingStep stepAStar;
        PathfindingStep stepBFS;

        // Dijkstra step
        if (!m_stepsDijkstra.empty()) {
            size_t idx = std::min(m_idxDijkstra, m_stepsDijkstra.size() - 1);
            stepDijkstra = m_stepsDijkstra[idx];
            stepDijkstra.stepIndex = static_cast<int>(idx);

            QSet<QString> stepPathEdges;
            if (stepDijkstra.type == PathfindingStep::Type::PathFound || idx == m_stepsDijkstra.size() - 1) {
                stepPathEdges = m_shortestPathEdgesDijkstra;
            }
            stepDijkstra.finalPathEdges = stepPathEdges;

            if (m_idxDijkstra < m_stepsDijkstra.size() - 1) {
                m_idxDijkstra++;
                anyRunning = true;
            }
        }

        // A* step
        if (!m_stepsAStar.empty()) {
            size_t idx = std::min(m_idxAStar, m_stepsAStar.size() - 1);
            stepAStar = m_stepsAStar[idx];
            stepAStar.stepIndex = static_cast<int>(idx);

            QSet<QString> stepPathEdges;
            if (stepAStar.type == PathfindingStep::Type::PathFound || idx == m_stepsAStar.size() - 1) {
                stepPathEdges = m_shortestPathEdgesAStar;
            }
            stepAStar.finalPathEdges = stepPathEdges;

            if (m_idxAStar < m_stepsAStar.size() - 1) {
                m_idxAStar++;
                anyRunning = true;
            }
        }

        // BFS step
        if (!m_stepsBFS.empty()) {
            size_t idx = std::min(m_idxBFS, m_stepsBFS.size() - 1);
            stepBFS = m_stepsBFS[idx];
            stepBFS.stepIndex = static_cast<int>(idx);

            QSet<QString> stepPathEdges;
            if (stepBFS.type == PathfindingStep::Type::PathFound || idx == m_stepsBFS.size() - 1) {
                stepPathEdges = m_shortestPathEdgesBFS;
            }
            stepBFS.finalPathEdges = stepPathEdges;

            if (m_idxBFS < m_stepsBFS.size() - 1) {
                m_idxBFS++;
                anyRunning = true;
            }
        }

        emit comparisonStepsRendered(stepDijkstra, stepAStar, stepBFS);

        if (!anyRunning) {
            m_timer->stop();
            m_isRunning = false;
        }
    }
}
