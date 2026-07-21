#include "AnimationController.h"

AnimationController::AnimationController(QObject *parent)
    : QObject(parent), m_timer(new QTimer(this)), m_currentIdx(0), m_speed(100), m_isRunning(false) {
    connect(m_timer, &QTimer::timeout, this, &AnimationController::processNextStep);
}

void AnimationController::setSteps(const std::vector<PathfindingStep> &steps, int msSpeed) {
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

    // Map shortest path lines into the rendering step
    QSet<QString> stepPathEdges;
    QString node = step.currentNodeId;
    
    // If we've finished, overlay full optimal lines
    if (step.type == PathfindingStep::Type::PathFound || m_currentIdx == m_steps.size() - 1) {
        stepPathEdges = m_shortestPathEdges;
    }

    step.finalPathEdges = stepPathEdges;

    emit stepRendered(step);
    m_currentIdx++;
}
