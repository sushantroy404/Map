#pragma once

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QComboBox>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QTableWidget>
#include <QListWidget>
#include <QTreeWidget>
#include <QDockWidget>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QGroupBox>
#include "Graph.h"
#include "AnimationController.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;
    void handleNodeSelected(const QString &nodeId, bool isStart);

private slots:
    void handleImportOsm();
    void handleRunAlgorithm();
    void handlePauseAlgorithm();
    void handleResumeAlgorithm();
    void handleReset();
    void handleClearPath();
    void handleSpeedChanged(int value);
    void handleZoomIn();
    void handleZoomOut();
    void handleFitView();
    void handleExportScreenshot();
    void handleSaveProject();
    void handleOpenProject();
    void handleToggleRightPanel();
    void handleComparisonModeToggled(bool enabled);
    
    // Animation callbacks
    void updateUiForStep(const PathfindingStep &step);
    void updateUiForComparisonSteps(const PathfindingStep &dijkstra, const PathfindingStep &astar, const PathfindingStep &bfs);

private:
    void createActions();
    void createMenus();
    void createToolBar();
    void createLeftPanel();
    void createCenterCanvas();
    void createRightPanel();
    void setupConnections();
    void loadGraphIntoScene();

    // Model data
    Graph m_graph;
    std::shared_ptr<PathfindingAlgorithm> m_activeAlgorithm;
    AnimationController *m_animationController;

    // UI elements - Left Controls
    QComboBox *m_algoCombo;
    QComboBox *m_startNodeCombo;
    QComboBox *m_endNodeCombo;
    QPushButton *m_importBtn;
    QPushButton *m_runBtn;
    QPushButton *m_pauseBtn;
    QPushButton *m_resumeBtn;
    QPushButton *m_resetBtn;
    QPushButton *m_clearBtn;
    QSlider *m_speedSlider;

    // UI elements - Left Statistics
    QLabel *m_statAlgo;
    QLabel *m_statCurrNode;
    QLabel *m_statVisited;
    QLabel *m_statDistance;
    QLabel *m_statTime;

    // UI elements - Center Viewport
    QWidget *m_centralContainer;
    QHBoxLayout *m_centralLayout;
    QGraphicsView *m_view;
    QGraphicsScene *m_scene;

    // Comparison Mode Multi-Views & Multi-Containers
    QWidget *m_containerDijkstra;
    QWidget *m_containerAStar;
    QWidget *m_containerBFS;

    QGraphicsView *m_viewDijkstra;
    QGraphicsView *m_viewAStar;
    QGraphicsView *m_viewBFS;
    QGraphicsScene *m_sceneDijkstra;
    QGraphicsScene *m_sceneAStar;
    QGraphicsScene *m_sceneBFS;

    // Per-map metrics labels below each map in comparison mode
    QLabel *m_cardDijkstraTime;
    QLabel *m_cardDijkstraVisited;
    QLabel *m_cardDijkstraDist;

    QLabel *m_cardAStarTime;
    QLabel *m_cardAStarVisited;
    QLabel *m_cardAStarDist;

    QLabel *m_cardBFSTime;
    QLabel *m_cardBFSVisited;
    QLabel *m_cardBFSDist;

    // UI elements - Right Information Panels (Docked)
    QDockWidget *m_rightDock;
    QPushButton *m_toggleRightBtn; // Reference to sync toggle button text
    QListWidget *m_priorityQueueList;
    QTableWidget *m_distanceTable;
    QListWidget *m_logWidget;
    QLabel *m_currentStepLabel;

    // ID lookup maps
    QString m_selectedStartNodeId;
    QString m_selectedEndNodeId;
    bool m_isComparisonMode;

    // Execution performance metrics
    double m_execTimeDijkstra;
    double m_execTimeAStar;
    double m_execTimeBFS;
    double m_execTimeSingle;

    void updateStartEndNodeHighlights();
};
