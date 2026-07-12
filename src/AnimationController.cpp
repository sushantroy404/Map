#include "AnimationController.h"

AnimationController::AnimationController(QObject *parent)
    : QObject(parent), m_timer(new QTimer(this)), m_currentIdx(0), m_speed(100), m_isRunning(false) {
    connect(m_timer, &QTimer::timeout, this, &AnimationController::processNextStep);
}

void AnimationController::setSteps(const std::vector<PathfindingStep> &steps, int msSpeed) {
    m_steps = steps;
    m_speed = msSpeed;
    m_currentIdx = 0;
    
    // The algorithms provide the OSM edge IDs in their final PathFound step.
    m_shortestPathEdges.clear();
    if (!m_steps.empty() && m_steps.back().type == PathfindingStep::Type::PathFound) {
        m_shortestPathEdges = m_steps.back().finalPathEdges;
    }
}

void AnimationController::start() {
    if (m_steps.empty()) return;
    m_currentIdx = 0;
    m_isRunning = true;
    m_timer->start(m_speed);
}

void AnimationController::pause() {
    m_isRunning = false;
    m_timer->stop();
}

void AnimationController::resume() {
    if (m_steps.empty() || m_currentIdx >= m_steps.size()) return;
    m_isRunning = true;
    m_timer->start(m_speed);
}

void AnimationController::stop() {
    m_isRunning = false;
    m_timer->stop();
    m_currentIdx = 0;
}

void AnimationController::setSpeed(int msSpeed) {
    m_speed = msSpeed;
    if (m_isRunning) {
        m_timer->start(m_speed);
    }
}

void AnimationController::processNextStep() {
    if (m_currentIdx >= m_steps.size()) {
        m_timer->stop();
        m_isRunning = false;
        return;
    }

    PathfindingStep step = m_steps[m_currentIdx];
    
    // Supplement C++ step index
    step.stepIndex = static_cast<int>(m_currentIdx);

    // If we've finished, overlay full optimal lines
    if (step.type == PathfindingStep::Type::PathFound || m_currentIdx == m_steps.size() - 1) {
        step.finalPathEdges = m_shortestPathEdges;
    } else {
        step.finalPathEdges.clear();
    }

    emit stepRendered(step);
    m_currentIdx++;
}
