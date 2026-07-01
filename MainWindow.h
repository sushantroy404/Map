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
#include "Graph.h"
#include "AnimationController.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
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
    void handleNodeSelected(const QString &nodeId, bool isStart);
    
    // Animation callbacks
    void updateUiForStep(const PathfindingStep &step);

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
    QGraphicsView *m_view;
    QGraphicsScene *m_scene;

    // UI elements - Right Information Panels
    QListWidget *m_priorityQueueList;
    QTableWidget *m_distanceTable;
    QListWidget *m_logWidget;
    QLabel *m_currentStepLabel;

    // ID lookup maps
    QString m_selectedStartNodeId;
    QString m_selectedEndNodeId;
};
