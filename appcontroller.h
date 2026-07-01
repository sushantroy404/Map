#pragma once

#include <QObject>
#include <QVariantList>
#include <QString>
#include "Graph.h"
#include "Algorithms.h"
#include "AnimationController.h"

class AppController : public QObject {
    Q_OBJECT

    Q_PROPERTY(QVariantList nodes READ nodes NOTIFY graphChanged)
    Q_PROPERTY(QVariantList edges READ edges NOTIFY graphChanged)
    Q_PROPERTY(QVariantList startNodes READ startNodes NOTIFY graphChanged)
    Q_PROPERTY(QVariantList endNodes READ endNodes NOTIFY graphChanged)

    Q_PROPERTY(QString startNodeId READ startNodeId WRITE setStartNodeId NOTIFY selectionChanged)
    Q_PROPERTY(QString endNodeId READ endNodeId WRITE setEndNodeId NOTIFY selectionChanged)

    Q_PROPERTY(int speed READ speed WRITE setSpeed NOTIFY speedChanged)

    Q_PROPERTY(QString activeAlgorithm READ activeAlgorithm NOTIFY algorithmStateChanged)
    Q_PROPERTY(QString currentNode READ currentNode NOTIFY algorithmStateChanged)
    Q_PROPERTY(int visitedCount READ visitedCount NOTIFY algorithmStateChanged)
    Q_PROPERTY(QString totalDistance READ totalDistance NOTIFY algorithmStateChanged)
    Q_PROPERTY(int executionTime READ executionTime NOTIFY algorithmStateChanged)
    Q_PROPERTY(int currentStep READ currentStep NOTIFY algorithmStateChanged)

    Q_PROPERTY(QVariantList frontier READ frontier NOTIFY algorithmStateChanged)
    Q_PROPERTY(QVariantList distanceTable READ distanceTable NOTIFY algorithmStateChanged)
    Q_PROPERTY(QVariantList logs READ logs NOTIFY algorithmStateChanged)

public:
    explicit AppController(QObject *parent = nullptr);

    QVariantList nodes() const;
    QVariantList edges() const;
    QVariantList startNodes() const;
    QVariantList endNodes() const;

    QString startNodeId() const;
    void setStartNodeId(const QString &id);

    QString endNodeId() const;
    void setEndNodeId(const QString &id);

    int speed() const;
    void setSpeed(int speed);

    QString activeAlgorithm() const;
    QString currentNode() const;
    int visitedCount() const;
    QString totalDistance() const;
    int executionTime() const;
    int currentStep() const;

    QVariantList frontier() const;
    QVariantList distanceTable() const;
    QVariantList logs() const;

    Q_INVOKABLE void importOsm();
    Q_INVOKABLE void runAlgorithm(const QString &algorithmName);
    Q_INVOKABLE void pauseAlgorithm();
    Q_INVOKABLE void resumeAlgorithm();
    Q_INVOKABLE void reset();
    Q_INVOKABLE void clearPath();

    Q_INVOKABLE void openProject();
    Q_INVOKABLE void saveProject();
    Q_INVOKABLE void exportScreenshot();

signals:
    void graphChanged();
    void selectionChanged();
    void speedChanged();
    void algorithmStateChanged();

private slots:
    void updateForStep(const PathfindingStep &step);

private:
    void rebuildVisualData();

    Graph m_graph;
    AnimationController *m_animationController;

    QVariantList m_nodes;
    QVariantList m_edges;
    QVariantList m_frontier;
    QVariantList m_distanceTable;
    QVariantList m_logs;

    QString m_startNodeId;
    QString m_endNodeId;
    QString m_activeAlgorithm = "None";
    QString m_currentNode = "--";
    QString m_totalDistance = "--";

    int m_speed = 100;
    int m_visitedCount = 0;
    int m_executionTime = 0;
    int m_currentStep = 0;
};