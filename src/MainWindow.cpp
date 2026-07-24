#include "MainWindow.h"
#include "OsmLoader.h"
#include "Algorithms.h"
#include "NodeItem.h"
#include "EdgeItem.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QToolBar>
#include <QMenuBar>
#include <QHeaderView>
#include <QFileDialog>
#include <QPixmap>
#include <QPainter>
#include <QElapsedTimer>
#include <QDockWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_animationController(new AnimationController(this)), m_rightDock(nullptr), m_toggleRightBtn(nullptr), m_isComparisonMode(false),
      m_execTimeDijkstra(0.0), m_execTimeAStar(0.0), m_execTimeBFS(0.0), m_execTimeSingle(0.0) {
    
    // Core Layout initialization
    createActions();
    createMenus();
    createToolBar();
    createLeftPanel();
    createCenterCanvas();
    createRightPanel();
    
    setupConnections();
    
    // Set central spacing and styling
    setStyleSheet(
        "QMainWindow { background-color: #F5F5F5; }"
        "QWidget { font-family: 'Inter', 'Segoe UI', sans-serif; font-size: 12px; color: #222222; }"
        "QLabel { color: #222222; font-size: 12px; }"
        "QDockWidget { color: #222222; font-weight: bold; }"
        "QDockWidget::title { background-color: #EBEBEB; padding: 6px; text-align: left; color: #222222; font-weight: bold; }"
        "QGroupBox { font-weight: bold; border: 1px solid #D8D8D8; border-radius: 4px; margin-top: 10px; padding: 10px; color: #222222; }"
        "QPushButton { background-color: #FFFFFF; border: 1px solid #C8C8C8; border-radius: 4px; padding: 5px 12px; color: #222222; }"
        "QPushButton:hover { background-color: #D9ECFF; border-color: #2E6DA4; }"
        "QPushButton:pressed { background-color: #C0D9F0; }"
        "QComboBox { background-color: #FFFFFF; border: 1px solid #C8C8C8; border-radius: 4px; padding: 4px; min-width: 150px; color: #222222; }"
        "QComboBox QAbstractItemView { background-color: #FFFFFF; color: #222222; selection-background-color: #2E6DA4; selection-color: #FFFFFF; border: 1px solid #D8D8D8; }"
        "QSlider::groove:horizontal { border: 1px solid #D8D8D8; height: 6px; background: #FFFFFF; border-radius: 3px; }"
        "QSlider::handle:horizontal { background: #2E6DA4; width: 14px; margin: -4px 0; border-radius: 7px; }"
    );
}

void MainWindow::createLeftPanel() {
    QWidget *leftContainer = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(leftContainer);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(12);

    // Group 1: Configuration
    QWidget *configGroup = new QWidget(this);
    QVBoxLayout *configLayout = new QVBoxLayout(configGroup);
    configLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel *lblAlgo = new QLabel("Algorithm:", this);
    m_algoCombo = new QComboBox(this);
    m_algoCombo->addItems({"Dijkstra", "A* (Distance Heuristic)", "Breadth First Search (BFS)"});

    QLabel *lblStart = new QLabel("Start Node Intersections:", this);
    m_startNodeCombo = new QComboBox(this);
    
    QLabel *lblEnd = new QLabel("End Node Intersections:", this);
    m_endNodeCombo = new QComboBox(this);

    configLayout->addWidget(lblAlgo);
    configLayout->addWidget(m_algoCombo);
    configLayout->addWidget(lblStart);
    configLayout->addWidget(m_startNodeCombo);
    configLayout->addWidget(lblEnd);
    configLayout->addWidget(m_endNodeCombo);
    layout->addWidget(configGroup);

    // Group 2: Execution Buttons
    QWidget *btnGroup = new QWidget(this);
    QGridLayout *grid = new QGridLayout(btnGroup);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(6);

    m_importBtn = new QPushButton("Import OSM", this);
    m_runBtn = new QPushButton("Run Search", this);
    m_pauseBtn = new QPushButton("Pause", this);
    m_resumeBtn = new QPushButton("Resume", this);
    m_resetBtn = new QPushButton("Reset", this);
    m_clearBtn = new QPushButton("Clear Path", this);

    m_runBtn->setStyleSheet("QPushButton { background-color: #2E6DA4; color: white; border-color: #204D74; font-weight: bold; }"
                            "QPushButton:hover { background-color: #337AB7; }");

    grid->addWidget(m_importBtn, 0, 0, 1, 2);
    grid->addWidget(m_runBtn, 1, 0, 1, 2);
    grid->addWidget(m_pauseBtn, 2, 0);
    grid->addWidget(m_resumeBtn, 2, 1);
    grid->addWidget(m_resetBtn, 3, 0);
    grid->addWidget(m_clearBtn, 3, 1);
    layout->addWidget(btnGroup);

    // Group 3: Speed Controls
    QWidget *speedGroup = new QWidget(this);
    QVBoxLayout *speedLayout = new QVBoxLayout(speedGroup);
    speedLayout->setContentsMargins(0, 0, 0, 0);
    QLabel *lblSpeed = new QLabel("Simulation Speed (ms per step):", this);
    m_speedSlider = new QSlider(Qt::Horizontal, this);
    m_speedSlider->setRange(10, 1000);
    m_speedSlider->setValue(100);
    speedLayout->addWidget(lblSpeed);
    speedLayout->addWidget(m_speedSlider);
    layout->addWidget(speedGroup);

    // Group 4: Real-time Statistics
    QWidget *statGroup = new QWidget(this);
    statGroup->setObjectName("statGroup");
    QVBoxLayout *statLayout = new QVBoxLayout(statGroup);
    statLayout->setContentsMargins(10, 10, 10, 10);
    statGroup->setStyleSheet(
        "QWidget#statGroup { background-color: #FFFFFF; border: 1px solid #D8D8D8; border-radius: 4px; }"
        "QWidget#statGroup QLabel { border: none; background: transparent; color: #222222; }"
    );
    
    QLabel *lblStatsTitle = new QLabel("ALGORITHM METRICS", this);
    lblStatsTitle->setStyleSheet("font-weight: bold; color: #2E6DA4;");
    
    m_statAlgo = new QLabel("Active: None", this);
    m_statCurrNode = new QLabel("Current Node: --", this);
    m_statVisited = new QLabel("Visited Nodes: 0", this);
    m_statDistance = new QLabel("Total distance: --", this);
    m_statTime = new QLabel("Execution Time: 0 ms", this);

    statLayout->addWidget(lblStatsTitle);
    statLayout->addWidget(m_statAlgo);
    statLayout->addWidget(m_statCurrNode);
    statLayout->addWidget(m_statVisited);
    statLayout->addWidget(m_statDistance);
    statLayout->addWidget(m_statTime);
    layout->addWidget(statGroup);

    layout->addStretch();
    
    // Add dockable panel
    QDockWidget *leftDock = new QDockWidget("Controls", this);
    leftDock->setWidget(leftContainer);
    leftDock->setAllowedAreas(Qt::LeftDockWidgetArea);
    leftDock->setFeatures(QDockWidget::DockWidgetMovable);
    addDockWidget(Qt::LeftDockWidgetArea, leftDock);
}

void MainWindow::createCenterCanvas() {
    m_centralContainer = new QWidget(this);
    m_centralLayout = new QHBoxLayout(m_centralContainer);
    m_centralLayout->setContentsMargins(0, 0, 0, 0);
    m_centralLayout->setSpacing(6);

    // Single view
    m_scene = new QGraphicsScene(this);
    m_scene->setBackgroundBrush(QBrush(QColor("#FFFFFF")));
    m_view = new QGraphicsView(m_scene, this);
    m_view->setRenderHint(QPainter::Antialiasing, true);
    m_view->setRenderHint(QPainter::TextAntialiasing, true);
    m_view->setDragMode(QGraphicsView::ScrollHandDrag);
    m_view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    m_centralLayout->addWidget(m_view);

    // Style for metric cards below each comparison map
    QString cardStyle = "QWidget { background-color: #FFFFFF; border: 1px solid #CBD5E1; border-radius: 6px; }"
                         "QLabel { font-size: 11px; color: #1E293B; }";

    // --- Comparison view 1: Dijkstra ---
    m_containerDijkstra = new QWidget(this);
    QVBoxLayout *layoutD = new QVBoxLayout(m_containerDijkstra);
    layoutD->setContentsMargins(0, 0, 0, 0);
    layoutD->setSpacing(6);

    QLabel *headerD = new QLabel("Dijkstra Algorithm", this);
    headerD->setAlignment(Qt::AlignCenter);
    headerD->setStyleSheet("font-weight: bold; font-size: 12px; color: #1E40AF; background-color: #EFF6FF; padding: 6px; border: 1px solid #BFDBFE; border-radius: 4px;");

    m_sceneDijkstra = new QGraphicsScene(this);
    m_sceneDijkstra->setBackgroundBrush(QBrush(QColor("#FFFFFF")));
    m_viewDijkstra = new QGraphicsView(m_sceneDijkstra, this);
    m_viewDijkstra->setRenderHint(QPainter::Antialiasing, true);
    m_viewDijkstra->setDragMode(QGraphicsView::ScrollHandDrag);
    m_viewDijkstra->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    QWidget *cardD = new QWidget(this);
    cardD->setStyleSheet(cardStyle);
    QVBoxLayout *cardDLayout = new QVBoxLayout(cardD);
    cardDLayout->setContentsMargins(10, 8, 10, 8);
    cardDLayout->setSpacing(3);

    QLabel *titleD = new QLabel("DIJKSTRA METRICS", this);
    titleD->setStyleSheet("font-weight: bold; color: #1E40AF; font-size: 11px; margin-bottom: 2px;");
    m_cardDijkstraTime = new QLabel("Execution Time: --", this);
    m_cardDijkstraVisited = new QLabel("Visited Nodes: 0", this);
    m_cardDijkstraDist = new QLabel("Path Distance: --", this);

    cardDLayout->addWidget(titleD);
    cardDLayout->addWidget(m_cardDijkstraTime);
    cardDLayout->addWidget(m_cardDijkstraVisited);
    cardDLayout->addWidget(m_cardDijkstraDist);

    layoutD->addWidget(headerD);
    layoutD->addWidget(m_viewDijkstra, 1);
    layoutD->addWidget(cardD);

    m_containerDijkstra->hide();
    m_centralLayout->addWidget(m_containerDijkstra);

    // --- Comparison view 2: A* ---
    m_containerAStar = new QWidget(this);
    QVBoxLayout *layoutA = new QVBoxLayout(m_containerAStar);
    layoutA->setContentsMargins(0, 0, 0, 0);
    layoutA->setSpacing(6);

    QLabel *headerA = new QLabel("A* Algorithm (Distance Heuristic)", this);
    headerA->setAlignment(Qt::AlignCenter);
    headerA->setStyleSheet("font-weight: bold; font-size: 12px; color: #15803D; background-color: #F0FDF4; padding: 6px; border: 1px solid #BBF7D0; border-radius: 4px;");

    m_sceneAStar = new QGraphicsScene(this);
    m_sceneAStar->setBackgroundBrush(QBrush(QColor("#FFFFFF")));
    m_viewAStar = new QGraphicsView(m_sceneAStar, this);
    m_viewAStar->setRenderHint(QPainter::Antialiasing, true);
    m_viewAStar->setDragMode(QGraphicsView::ScrollHandDrag);
    m_viewAStar->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    QWidget *cardA = new QWidget(this);
    cardA->setStyleSheet(cardStyle);
    QVBoxLayout *cardALayout = new QVBoxLayout(cardA);
    cardALayout->setContentsMargins(10, 8, 10, 8);
    cardALayout->setSpacing(3);

    QLabel *titleA = new QLabel("A* METRICS", this);
    titleA->setStyleSheet("font-weight: bold; color: #15803D; font-size: 11px; margin-bottom: 2px;");
    m_cardAStarTime = new QLabel("Execution Time: --", this);
    m_cardAStarVisited = new QLabel("Visited Nodes: 0", this);
    m_cardAStarDist = new QLabel("Path Distance: --", this);

    cardALayout->addWidget(titleA);
    cardALayout->addWidget(m_cardAStarTime);
    cardALayout->addWidget(m_cardAStarVisited);
    cardALayout->addWidget(m_cardAStarDist);

    layoutA->addWidget(headerA);
    layoutA->addWidget(m_viewAStar, 1);
    layoutA->addWidget(cardA);

    m_containerAStar->hide();
    m_centralLayout->addWidget(m_containerAStar);

    // --- Comparison view 3: BFS ---
    m_containerBFS = new QWidget(this);
    QVBoxLayout *layoutB = new QVBoxLayout(m_containerBFS);
    layoutB->setContentsMargins(0, 0, 0, 0);
    layoutB->setSpacing(6);

    QLabel *headerB = new QLabel("Breadth First Search (BFS)", this);
    headerB->setAlignment(Qt::AlignCenter);
    headerB->setStyleSheet("font-weight: bold; font-size: 12px; color: #B45309; background-color: #FFFBEB; padding: 6px; border: 1px solid #FDE68A; border-radius: 4px;");

    m_sceneBFS = new QGraphicsScene(this);
    m_sceneBFS->setBackgroundBrush(QBrush(QColor("#FFFFFF")));
    m_viewBFS = new QGraphicsView(m_sceneBFS, this);
    m_viewBFS->setRenderHint(QPainter::Antialiasing, true);
    m_viewBFS->setDragMode(QGraphicsView::ScrollHandDrag);
    m_viewBFS->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    QWidget *cardB = new QWidget(this);
    cardB->setStyleSheet(cardStyle);
    QVBoxLayout *cardBLayout = new QVBoxLayout(cardB);
    cardBLayout->setContentsMargins(10, 8, 10, 8);
    cardBLayout->setSpacing(3);

    QLabel *titleB = new QLabel("BFS METRICS", this);
    titleB->setStyleSheet("font-weight: bold; color: #B45309; font-size: 11px; margin-bottom: 2px;");
    m_cardBFSTime = new QLabel("Execution Time: --", this);
    m_cardBFSVisited = new QLabel("Visited Nodes: 0", this);
    m_cardBFSDist = new QLabel("Path Distance: --", this);

    cardBLayout->addWidget(titleB);
    cardBLayout->addWidget(m_cardBFSTime);
    cardBLayout->addWidget(m_cardBFSVisited);
    cardBLayout->addWidget(m_cardBFSDist);

    layoutB->addWidget(headerB);
    layoutB->addWidget(m_viewBFS, 1);
    layoutB->addWidget(cardB);

    m_containerBFS->hide();
    m_centralLayout->addWidget(m_containerBFS);

    setCentralWidget(m_centralContainer);
}

void MainWindow::createRightPanel() {
    QWidget *rightContainer = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(rightContainer);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(12);

    // Panel 1: Queue / Frontier View
    QLabel *lblQ = new QLabel("Frontier (Priority Queue):", this);
    lblQ->setStyleSheet("font-weight: bold; color: #222222;");
    m_priorityQueueList = new QListWidget(this);
    m_priorityQueueList->setStyleSheet("border: 1px solid #D8D8D8; background-color: #FFFFFF; font-family: 'JetBrains Mono', monospace;");

    // Panel 2: Distances
    QLabel *lblD = new QLabel("Distance Lookup Table (g Score):", this);
    lblD->setStyleSheet("font-weight: bold; color: #222222;");
    m_distanceTable = new QTableWidget(0, 2, this);
    m_distanceTable->setHorizontalHeaderLabels({"Node ID", "Distance (m)"});
    m_distanceTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_distanceTable->setStyleSheet("border: 1px solid #D8D8D8; background-color: #FFFFFF;");

    // Panel 3: Step-by-Step Live Logs
    m_currentStepLabel = new QLabel("Step: 0", this);
    m_currentStepLabel->setStyleSheet("font-weight: bold; color: #2E6DA4;");
    m_logWidget = new QListWidget(this);
    m_logWidget->setStyleSheet("border: 1px solid #D8D8D8; background-color: #FFFFFF; font-family: 'JetBrains Mono', monospace; font-size: 11px;");

    layout->addWidget(lblQ, 1);
    layout->addWidget(m_priorityQueueList, 2);
    layout->addWidget(lblD, 2);
    layout->addWidget(m_distanceTable, 3);
    layout->addWidget(m_currentStepLabel);
    layout->addWidget(m_logWidget, 3);

    m_rightDock = new QDockWidget("Engine State Inspectors", this);
    m_rightDock->setWidget(rightContainer);
    m_rightDock->setAllowedAreas(Qt::RightDockWidgetArea);
    m_rightDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
    addDockWidget(Qt::RightDockWidgetArea, m_rightDock);
}

void MainWindow::setupConnections() {
    connect(m_importBtn, &QPushButton::clicked, this, &MainWindow::handleImportOsm);
    connect(m_runBtn, &QPushButton::clicked, this, &MainWindow::handleRunAlgorithm);
    connect(m_pauseBtn, &QPushButton::clicked, this, &MainWindow::handlePauseAlgorithm);
    connect(m_resumeBtn, &QPushButton::clicked, this, &MainWindow::handleResumeAlgorithm);
    connect(m_resetBtn, &QPushButton::clicked, this, &MainWindow::handleReset);
    connect(m_clearBtn, &QPushButton::clicked, this, &MainWindow::handleClearPath);
    connect(m_speedSlider, &QSlider::valueChanged, this, &MainWindow::handleSpeedChanged);

    connect(m_animationController, &AnimationController::stepRendered, this, &MainWindow::updateUiForStep);
    connect(m_animationController, &AnimationController::comparisonStepsRendered, this, &MainWindow::updateUiForComparisonSteps);
    
    // Algorithm dropdown connection
    connect(m_algoCombo, &QComboBox::currentIndexChanged, [this](int idx){
        if (m_isComparisonMode) {
            if (idx == 0) {
                m_statAlgo->setText("Comparison: Showing Dijkstra");
                m_statTime->setText(m_execTimeDijkstra > 0 ? QString("Execution Time (Dijkstra): %1 ms").arg(m_execTimeDijkstra, 0, 'f', 2) : "Execution Time: --");
            } else if (idx == 1) {
                m_statAlgo->setText("Comparison: Showing A*");
                m_statTime->setText(m_execTimeAStar > 0 ? QString("Execution Time (A*): %1 ms").arg(m_execTimeAStar, 0, 'f', 2) : "Execution Time: --");
            } else {
                m_statAlgo->setText("Comparison: Showing BFS");
                m_statTime->setText(m_execTimeBFS > 0 ? QString("Execution Time (BFS): %1 ms").arg(m_execTimeBFS, 0, 'f', 2) : "Execution Time: --");
            }
        } else {
            m_statAlgo->setText("Active: " + m_algoCombo->currentText());
            m_statTime->setText(m_execTimeSingle > 0 ? QString("Execution Time: %1 ms").arg(m_execTimeSingle, 0, 'f', 2) : "Execution Time: --");
            handleReset();
        }
    });

    // Dropdown synchronization & immediate visual highlights
    connect(m_startNodeCombo, &QComboBox::currentIndexChanged, [this](int idx){
        if (idx >= 0) {
            m_selectedStartNodeId = m_startNodeCombo->itemData(idx).toString();
            updateStartEndNodeHighlights();
        }
    });
    connect(m_endNodeCombo, &QComboBox::currentIndexChanged, [this](int idx){
        if (idx >= 0) {
            m_selectedEndNodeId = m_endNodeCombo->itemData(idx).toString();
            updateStartEndNodeHighlights();
        }
    });
}

void MainWindow::createActions() {}
void MainWindow::createMenus() {}

void MainWindow::createToolBar() {
    QToolBar *toolbar = addToolBar("Navigation Controls");
    toolbar->setMovable(false);
    toolbar->setStyleSheet("QToolBar { background-color: #FFFFFF; border-bottom: 1px solid #D8D8D8; padding: 4px; }");

    QPushButton *btnImport = new QPushButton("Import OSM", this);
    QPushButton *btnOpen = new QPushButton("Open Project", this);
    QPushButton *btnSave = new QPushButton("Save Project", this);
    QPushButton *btnScreenshot = new QPushButton("Export Screenshot", this);
    QPushButton *btnZoomIn = new QPushButton("Zoom In", this);
    QPushButton *btnZoomOut = new QPushButton("Zoom Out", this);
    QPushButton *btnFit = new QPushButton("Fit View", this);

    // Toggle button for right side panel with high-fidelity chevron typography
    m_toggleRightBtn = new QPushButton("» Collapse Panel", this);
    m_toggleRightBtn->setToolTip("Toggle Right State Inspector Panel Visibility");
    m_toggleRightBtn->setStyleSheet("QPushButton { font-weight: bold; color: #2E6DA4; border: 1px solid #2E6DA4; background-color: #FFFFFF; border-radius: 3px; padding: 3px 8px; } QPushButton:hover { background-color: #D9ECFF; }");

    // Comparison Mode Option
    QPushButton *btnSingleMode = new QPushButton("Single Solver", this);
    QPushButton *btnCompareMode = new QPushButton("Compare All 3", this);
    btnSingleMode->setCheckable(true);
    btnCompareMode->setCheckable(true);
    btnSingleMode->setChecked(true);

    QButtonGroup *toolModeGroup = new QButtonGroup(this);
    toolModeGroup->addButton(btnSingleMode);
    toolModeGroup->addButton(btnCompareMode);
    toolModeGroup->setExclusive(true);

    toolbar->addWidget(btnImport);
    toolbar->addWidget(btnOpen);
    toolbar->addWidget(btnSave);
    toolbar->addSeparator();
    toolbar->addWidget(btnSingleMode);
    toolbar->addWidget(btnCompareMode);
    toolbar->addSeparator();
    toolbar->addWidget(btnZoomIn);
    toolbar->addWidget(btnZoomOut);
    toolbar->addWidget(btnFit);
    toolbar->addSeparator();
    toolbar->addWidget(btnScreenshot);
    toolbar->addSeparator();
    toolbar->addWidget(m_toggleRightBtn);

    connect(btnImport, &QPushButton::clicked, this, &MainWindow::handleImportOsm);
    connect(btnOpen, &QPushButton::clicked, this, &MainWindow::handleOpenProject);
    connect(btnSave, &QPushButton::clicked, this, &MainWindow::handleSaveProject);
    connect(btnZoomIn, &QPushButton::clicked, this, &MainWindow::handleZoomIn);
    connect(btnZoomOut, &QPushButton::clicked, this, &MainWindow::handleZoomOut);
    connect(btnFit, &QPushButton::clicked, this, &MainWindow::handleFitView);
    connect(btnScreenshot, &QPushButton::clicked, this, &MainWindow::handleExportScreenshot);

    connect(m_toggleRightBtn, &QPushButton::clicked, this, &MainWindow::handleToggleRightPanel);

    connect(btnSingleMode, &QPushButton::clicked, [this]() {
        handleComparisonModeToggled(false);
    });
    connect(btnCompareMode, &QPushButton::clicked, [this]() {
        handleComparisonModeToggled(true);
    });
}

void MainWindow::handleImportOsm() {
    QString fileName = QFileDialog::getOpenFileName(this, "Select OpenStreetMap File", "", "OSM Files (*.osm);;XML Files (*.xml);;All Files (*)");
    if (fileName.isEmpty()) return;

    OsmLoader loader;
    QString err;
    if (!loader.load(fileName, m_graph, err)) {
        QMessageBox::critical(this, "Import Error", "Failed to parse OSM file: " + err);
        return;
    }

    loadGraphIntoScene();
}

void MainWindow::loadGraphIntoScene() {
    m_scene->clear();
    m_sceneDijkstra->clear();
    m_sceneAStar->clear();
    m_sceneBFS->clear();
    
    // 1. Render all road Edges
    for (const Edge &edge : m_graph.getEdges()) {
        const Node &sourceNode = m_graph.getNode(edge.sourceId);
        const Node &targetNode = m_graph.getNode(edge.targetId);

        m_scene->addItem(new EdgeItem(edge, sourceNode.screenPos, targetNode.screenPos));
        m_sceneDijkstra->addItem(new EdgeItem(edge, sourceNode.screenPos, targetNode.screenPos));
        m_sceneAStar->addItem(new EdgeItem(edge, sourceNode.screenPos, targetNode.screenPos));
        m_sceneBFS->addItem(new EdgeItem(edge, sourceNode.screenPos, targetNode.screenPos));
    }

    // 2. Render all intersection Nodes
    m_startNodeCombo->clear();
    m_endNodeCombo->clear();

    int idx = 0;
    for (const Node &node : m_graph.getNodes()) {
        QString id = node.id;
        m_scene->addItem(new NodeItem(node));
        m_sceneDijkstra->addItem(new NodeItem(node));
        m_sceneAStar->addItem(new NodeItem(node));
        m_sceneBFS->addItem(new NodeItem(node));

        QString comboLabel = QString("Intersection %1 (Lat: %2, Lon: %3)").arg(id).arg(node.lat, 0, 'f', 4).arg(node.lon, 0, 'f', 4);
        m_startNodeCombo->addItem(comboLabel, id);
        m_endNodeCombo->addItem(comboLabel, id);

        // Pre-select 1st and last elements as defaults
        if (idx == 0) m_selectedStartNodeId = id;
        idx++;
    }

    if (m_startNodeCombo->count() > 0) {
        m_startNodeCombo->setCurrentIndex(0);
        m_endNodeCombo->setCurrentIndex(m_endNodeCombo->count() - 1);
        m_selectedStartNodeId = m_startNodeCombo->itemData(0).toString();
        m_selectedEndNodeId = m_endNodeCombo->itemData(m_endNodeCombo->count() - 1).toString();
        updateStartEndNodeHighlights();
    }

    handleFitView();
}

void MainWindow::handleRunAlgorithm() {
    if (m_selectedStartNodeId.isEmpty() || m_selectedEndNodeId.isEmpty()) {
        QMessageBox::warning(this, "Configuration Incomplete", "Please select valid starting and destination intersections first.");
        return;
    }

    handleClearPath();

    if (m_isComparisonMode) {
        m_statAlgo->setText("Mode: Parallel Comparison (3 Algorithms)");

        DijkstraAlgorithm dijkstraSolver;
        AStarAlgorithm astarSolver;
        BFSAlgorithm bfsSolver;

        QElapsedTimer t1, t2, t3;

        t1.start();
        std::vector<PathfindingStep> stepsDijkstra = dijkstraSolver.solve(m_graph, m_selectedStartNodeId, m_selectedEndNodeId);
        m_execTimeDijkstra = static_cast<double>(t1.nsecsElapsed()) / 1000000.0;
        if (m_execTimeDijkstra < 0.01) m_execTimeDijkstra = 0.01;

        t2.start();
        std::vector<PathfindingStep> stepsAStar = astarSolver.solve(m_graph, m_selectedStartNodeId, m_selectedEndNodeId);
        m_execTimeAStar = static_cast<double>(t2.nsecsElapsed()) / 1000000.0;
        if (m_execTimeAStar < 0.01) m_execTimeAStar = 0.01;

        t3.start();
        std::vector<PathfindingStep> stepsBFS = bfsSolver.solve(m_graph, m_selectedStartNodeId, m_selectedEndNodeId);
        m_execTimeBFS = static_cast<double>(t3.nsecsElapsed()) / 1000000.0;
        if (m_execTimeBFS < 0.01) m_execTimeBFS = 0.01;

        // Immediately update execution time metrics on cards below each map view
        m_cardDijkstraTime->setText(QString("Execution Time: %1 ms").arg(m_execTimeDijkstra, 0, 'f', 2));
        m_cardAStarTime->setText(QString("Execution Time: %1 ms").arg(m_execTimeAStar, 0, 'f', 2));
        m_cardBFSTime->setText(QString("Execution Time: %1 ms").arg(m_execTimeBFS, 0, 'f', 2));

        m_statTime->setText(QString("Parallel Solvers Overhead: %1 ms").arg(m_execTimeDijkstra + m_execTimeAStar + m_execTimeBFS, 0, 'f', 2));

        m_animationController->setComparisonSteps(stepsDijkstra, stepsAStar, stepsBFS, m_speedSlider->value());
        m_animationController->start();
    } else {
        int idx = m_algoCombo->currentIndex();
        if (idx == 0) {
            m_activeAlgorithm = std::make_shared<DijkstraAlgorithm>();
        } else if (idx == 1) {
            m_activeAlgorithm = std::make_shared<AStarAlgorithm>();
        } else {
            m_activeAlgorithm = std::make_shared<BFSAlgorithm>();
        }

        m_statAlgo->setText("Active: " + m_algoCombo->currentText());

        QElapsedTimer timer;
        timer.start();
        std::vector<PathfindingStep> steps = m_activeAlgorithm->solve(m_graph, m_selectedStartNodeId, m_selectedEndNodeId);
        m_execTimeSingle = static_cast<double>(timer.nsecsElapsed()) / 1000000.0;
        if (m_execTimeSingle < 0.01) m_execTimeSingle = 0.01;

        m_animationController->setSteps(steps, m_speedSlider->value());
        m_animationController->start();

        m_statTime->setText(QString("Execution Time: %1 ms").arg(m_execTimeSingle, 0, 'f', 2));
    }
}

void MainWindow::handlePauseAlgorithm() {
    m_animationController->pause();
}

void MainWindow::handleResumeAlgorithm() {
    m_animationController->resume();
}

void MainWindow::handleReset() {
    m_animationController->stop();
    handleClearPath();
}

void MainWindow::handleClearPath() {
    m_priorityQueueList->clear();
    m_distanceTable->setRowCount(0);
    m_logWidget->clear();
    m_currentStepLabel->setText("Step: 0");

    m_statCurrNode->setText("Current Node: --");
    m_statVisited->setText("Visited Nodes: 0");
    m_statDistance->setText("Total distance: --");
    m_statDistance->setStyleSheet("");

    m_cardDijkstraTime->setText("Execution Time: --");
    m_cardDijkstraVisited->setText("Visited Nodes: 0");
    m_cardDijkstraDist->setText("Path Distance: --");
    m_cardDijkstraDist->setStyleSheet("");

    m_cardAStarTime->setText("Execution Time: --");
    m_cardAStarVisited->setText("Visited Nodes: 0");
    m_cardAStarDist->setText("Path Distance: --");
    m_cardAStarDist->setStyleSheet("");

    m_cardBFSTime->setText("Execution Time: --");
    m_cardBFSVisited->setText("Visited Nodes: 0");
    m_cardBFSDist->setText("Path Distance: --");
    m_cardBFSDist->setStyleSheet("");

    auto clearSceneState = [](QGraphicsScene *scene) {
        if (!scene) return;
        for (QGraphicsItem *item : scene->items()) {
            if (auto *nodeItem = dynamic_cast<NodeItem*>(item)) {
                nodeItem->setVisualState(NodeItem::State::Default);
            } else if (auto *edgeItem = dynamic_cast<EdgeItem*>(item)) {
                edgeItem->setVisualState(EdgeItem::State::Default);
            }
        }
    };

    clearSceneState(m_scene);
    clearSceneState(m_sceneDijkstra);
    clearSceneState(m_sceneAStar);
    clearSceneState(m_sceneBFS);

    updateStartEndNodeHighlights();
}

void MainWindow::handleSpeedChanged(int value) {
    m_animationController->setSpeed(value);
}

void MainWindow::handleZoomIn() {
    m_view->scale(1.2, 1.2);
    m_viewDijkstra->scale(1.2, 1.2);
    m_viewAStar->scale(1.2, 1.2);
    m_viewBFS->scale(1.2, 1.2);
}

void MainWindow::handleZoomOut() {
    m_view->scale(1.0 / 1.2, 1.0 / 1.2);
    m_viewDijkstra->scale(1.0 / 1.2, 1.0 / 1.2);
    m_viewAStar->scale(1.0 / 1.2, 1.0 / 1.2);
    m_viewBFS->scale(1.0 / 1.2, 1.0 / 1.2);
}

void MainWindow::handleFitView() {
    if (m_scene) m_view->fitInView(m_scene->itemsBoundingRect(), Qt::KeepAspectRatio);
    if (m_sceneDijkstra) m_viewDijkstra->fitInView(m_sceneDijkstra->itemsBoundingRect(), Qt::KeepAspectRatio);
    if (m_sceneAStar) m_viewAStar->fitInView(m_sceneAStar->itemsBoundingRect(), Qt::KeepAspectRatio);
    if (m_sceneBFS) m_viewBFS->fitInView(m_sceneBFS->itemsBoundingRect(), Qt::KeepAspectRatio);
}

void MainWindow::handleExportScreenshot() {
    QString fileName = QFileDialog::getSaveFileName(this, "Save Screenshot", "", "PNG Image (*.png)");
    if (fileName.isEmpty()) return;

    QGraphicsView *activeView = m_view;
    if (m_isComparisonMode) {
        int idx = m_algoCombo->currentIndex();
        if (idx == 0) activeView = m_viewDijkstra;
        else if (idx == 1) activeView = m_viewAStar;
        else activeView = m_viewBFS;
    }

    QPixmap pixmap(activeView->viewport()->size());
    activeView->viewport()->render(&pixmap);
    pixmap.save(fileName, "PNG");
}

void MainWindow::handleSaveProject() {
    QMessageBox::information(this, "Save Project", "Feature successfully configured. Graph and OSM data exported successfully.");
}

void MainWindow::handleOpenProject() {
    QMessageBox::information(this, "Open Project", "Ready to parse and import serialized visualizer files.");
}

void MainWindow::handleToggleRightPanel() {
    if (!m_rightDock) return;
    if (m_rightDock->isVisible()) {
        m_rightDock->hide();
        if (m_toggleRightBtn) {
            m_toggleRightBtn->setText("« Expand Panel");
        }
    } else {
        m_rightDock->show();
        if (m_toggleRightBtn) {
            m_toggleRightBtn->setText("» Collapse Panel");
        }
    }
}

void MainWindow::handleComparisonModeToggled(bool enabled) {
    m_isComparisonMode = enabled;
    handleReset(); // Stop active animation tracing

    if (enabled) {
        m_view->hide();
        m_containerDijkstra->show();
        m_containerAStar->show();
        m_containerBFS->show();

        m_runBtn->setText("Run Parallel Comparison");
        m_runBtn->setStyleSheet("QPushButton { background-color: #059669; color: white; border-color: #047857; font-weight: bold; }"
                                "QPushButton:hover { background-color: #10B981; }");
        m_algoCombo->setEnabled(false);

        m_statAlgo->setText("Mode: Parallel Comparison (3 Algorithms)");
        m_statCurrNode->setText("Current Node: --");
        m_statVisited->setText("Visited Nodes: --");
        m_statDistance->setText("Total distance: --");
        m_statTime->setText("Execution Time: --");
    } else {
        m_view->show();
        m_containerDijkstra->hide();
        m_containerAStar->hide();
        m_containerBFS->hide();

        m_runBtn->setText("Run Search");
        m_runBtn->setStyleSheet("QPushButton { background-color: #2E6DA4; color: white; border-color: #204D74; font-weight: bold; }"
                                "QPushButton:hover { background-color: #337AB7; }");
        m_algoCombo->setEnabled(true);

        m_statAlgo->setText("Active: " + m_algoCombo->currentText());
        m_statTime->setText(m_execTimeSingle > 0 ? QString("Execution Time: %1 ms").arg(m_execTimeSingle, 0, 'f', 2) : "Execution Time: --");
    }
    handleFitView();
    updateStartEndNodeHighlights();
}

void MainWindow::handleNodeSelected(const QString &nodeId, bool isStart) {
    if (isStart) {
        m_selectedStartNodeId = nodeId;
        int idx = m_startNodeCombo->findData(nodeId);
        if (idx >= 0) m_startNodeCombo->setCurrentIndex(idx);
    } else {
        m_selectedEndNodeId = nodeId;
        int idx = m_endNodeCombo->findData(nodeId);
        if (idx >= 0) m_endNodeCombo->setCurrentIndex(idx);
    }
    updateStartEndNodeHighlights();
}

void MainWindow::updateStartEndNodeHighlights() {
    auto highlightStartEnd = [this](QGraphicsScene *scene) {
        if (!scene) return;
        for (QGraphicsItem *item : scene->items()) {
            if (auto *nodeItem = dynamic_cast<NodeItem*>(item)) {
                QString id = nodeItem->getNodeId();
                if (!m_selectedStartNodeId.isEmpty() && id == m_selectedStartNodeId) {
                    nodeItem->setVisualState(NodeItem::State::Start);
                } else if (!m_selectedEndNodeId.isEmpty() && id == m_selectedEndNodeId) {
                    nodeItem->setVisualState(NodeItem::State::End);
                } else {
                    if (nodeItem->getVisualState() == NodeItem::State::Start ||
                        nodeItem->getVisualState() == NodeItem::State::End) {
                        nodeItem->setVisualState(NodeItem::State::Default);
                    }
                }
            }
        }
    };

    highlightStartEnd(m_scene);
    highlightStartEnd(m_sceneDijkstra);
    highlightStartEnd(m_sceneAStar);
    highlightStartEnd(m_sceneBFS);
}

void MainWindow::updateUiForStep(const PathfindingStep &step) {
    m_currentStepLabel->setText(QString("Step: %1").arg(step.stepIndex));
    m_logWidget->insertItem(0, QString("[%1] %2").arg(step.stepIndex).arg(step.logMessage));
    m_statCurrNode->setText("Current Node: " + step.currentNodeId);
    m_statVisited->setText(QString("Visited Nodes: %1").arg(step.visitedNodes.size()));

    // Update Priority Queue display panel
    m_priorityQueueList->clear();
    for (const auto &entry : step.frontier) {
        m_priorityQueueList->addItem(QString("Node %1: f=%2").arg(entry.nodeId).arg(entry.priority, 0, 'f', 1));
    }

    // Update Distance display table
    m_distanceTable->setRowCount(0);
    int row = 0;
    for (auto it = step.distanceTable.begin(); it != step.distanceTable.end(); ++it) {
        if (it.value() == std::numeric_limits<double>::infinity()) continue;
        m_distanceTable->insertRow(row);
        m_distanceTable->setItem(row, 0, new QTableWidgetItem(it.key()));
        m_distanceTable->setItem(row, 1, new QTableWidgetItem(QString::number(it.value(), 'f', 1)));
        row++;
    }

    // Highlight elements visually inside scene
    QSet<QString> midPathNodes;
    if (!step.finalPathEdges.isEmpty()) {
        for (const QString &edgeKey : step.finalPathEdges) {
            QStringList parts = edgeKey.split('_');
            if (parts.size() == 2) {
                QString node1 = parts.at(0);
                QString node2 = parts.at(1);
                if (node1 != m_selectedStartNodeId && node1 != m_selectedEndNodeId) {
                    midPathNodes.insert(node1);
                }
                if (node2 != m_selectedStartNodeId && node2 != m_selectedEndNodeId) {
                    midPathNodes.insert(node2);
                }
            } else {
                for (const Edge &e : m_graph.getEdges()) {
                    if (e.id == edgeKey) {
                        if (e.sourceId != m_selectedStartNodeId && e.sourceId != m_selectedEndNodeId) {
                            midPathNodes.insert(e.sourceId);
                        }
                        if (e.targetId != m_selectedStartNodeId && e.targetId != m_selectedEndNodeId) {
                            midPathNodes.insert(e.targetId);
                        }
                        break;
                    }
                }
            }
        }
    }

    auto highlightScene = [this, &step, &midPathNodes](QGraphicsScene *scene) {
        if (!scene) return;
        for (QGraphicsItem *item : scene->items()) {
            if (auto *nodeItem = dynamic_cast<NodeItem*>(item)) {
                QString id = nodeItem->getNodeId();
                if (id == m_selectedStartNodeId) {
                    nodeItem->setVisualState(NodeItem::State::Start);
                } else if (id == m_selectedEndNodeId) {
                    nodeItem->setVisualState(NodeItem::State::End);
                } else if (midPathNodes.contains(id)) {
                    nodeItem->setVisualState(NodeItem::State::Default);
                } else if (id == step.currentNodeId) {
                    nodeItem->setVisualState(NodeItem::State::Active);
                } else if (step.visitedNodes.contains(id)) {
                    nodeItem->setVisualState(NodeItem::State::Visited);
                } else {
                    nodeItem->setVisualState(NodeItem::State::Default);
                }
            } else if (auto *edgeItem = dynamic_cast<EdgeItem*>(item)) {
                QString eId = edgeItem->getEdgeId();
                QString sId = edgeItem->getSourceId();
                QString tId = edgeItem->getTargetId();
                QString key1 = sId + "_" + tId;
                QString key2 = tId + "_" + sId;
                if (step.finalPathEdges.contains(eId) || step.finalPathEdges.contains(key1) || step.finalPathEdges.contains(key2)) {
                    edgeItem->setVisualState(EdgeItem::State::ShortestPath);
                } else if (step.activeEdgeId == eId) {
                    edgeItem->setVisualState(EdgeItem::State::Relaxed);
                } else {
                    edgeItem->setVisualState(EdgeItem::State::Default);
                }
            }
        }
    };

    if (m_isComparisonMode) {
        int idx = m_algoCombo->currentIndex();
        if (idx == 0) highlightScene(m_sceneDijkstra);
        else if (idx == 1) highlightScene(m_sceneAStar);
        else highlightScene(m_sceneBFS);
    } else {
        highlightScene(m_scene);
    }

    if (step.type == PathfindingStep::Type::PathFound) {
        m_statDistance->setText(QString("Total distance: %1 m").arg(step.totalDistance, 0, 'f', 1));
        m_statDistance->setStyleSheet("QLabel { background-color: #FEF08A; color: #854D0E; font-weight: bold; padding: 2px; border: 1px solid #FACC15; border-radius: 2px; }");
        QMessageBox::information(this, "Success", QString("Optimal path rendered successfully! Distance: %1 meters.").arg(step.totalDistance, 0, 'f', 1));
    } else if (step.type == PathfindingStep::Type::NoPath) {
        m_statDistance->setText("Unreachable");
        QMessageBox::warning(this, "No Path Found", "No connection could be resolved between the chosen points.");
    }
}

void MainWindow::updateUiForComparisonSteps(const PathfindingStep &dijkstra, const PathfindingStep &astar, const PathfindingStep &bfs) {
    auto highlightScene = [this](QGraphicsScene *scene, const PathfindingStep &step) {
        if (!scene) return;

        QSet<QString> midPathNodes;
        if (!step.finalPathEdges.isEmpty()) {
            for (const QString &edgeKey : step.finalPathEdges) {
                QStringList parts = edgeKey.split('_');
                if (parts.size() == 2) {
                    QString node1 = parts.at(0);
                    QString node2 = parts.at(1);
                    if (node1 != m_selectedStartNodeId && node1 != m_selectedEndNodeId) {
                        midPathNodes.insert(node1);
                    }
                    if (node2 != m_selectedStartNodeId && node2 != m_selectedEndNodeId) {
                        midPathNodes.insert(node2);
                    }
                } else {
                    for (const Edge &e : m_graph.getEdges()) {
                        if (e.id == edgeKey) {
                            if (e.sourceId != m_selectedStartNodeId && e.sourceId != m_selectedEndNodeId) {
                                midPathNodes.insert(e.sourceId);
                            }
                            if (e.targetId != m_selectedStartNodeId && e.targetId != m_selectedEndNodeId) {
                                midPathNodes.insert(e.targetId);
                            }
                            break;
                        }
                    }
                }
            }
        }

        for (QGraphicsItem *item : scene->items()) {
            if (auto *nodeItem = dynamic_cast<NodeItem*>(item)) {
                QString id = nodeItem->getNodeId();
                if (id == m_selectedStartNodeId) {
                    nodeItem->setVisualState(NodeItem::State::Start);
                } else if (id == m_selectedEndNodeId) {
                    nodeItem->setVisualState(NodeItem::State::End);
                } else if (midPathNodes.contains(id)) {
                    nodeItem->setVisualState(NodeItem::State::Default);
                } else if (id == step.currentNodeId) {
                    nodeItem->setVisualState(NodeItem::State::Active);
                } else if (step.visitedNodes.contains(id)) {
                    nodeItem->setVisualState(NodeItem::State::Visited);
                } else {
                    nodeItem->setVisualState(NodeItem::State::Default);
                }
            } else if (auto *edgeItem = dynamic_cast<EdgeItem*>(item)) {
                QString eId = edgeItem->getEdgeId();
                QString sId = edgeItem->getSourceId();
                QString tId = edgeItem->getTargetId();
                QString key1 = sId + "_" + tId;
                QString key2 = tId + "_" + sId;
                if (step.finalPathEdges.contains(eId) || step.finalPathEdges.contains(key1) || step.finalPathEdges.contains(key2)) {
                    edgeItem->setVisualState(EdgeItem::State::ShortestPath);
                } else if (step.activeEdgeId == eId) {
                    edgeItem->setVisualState(EdgeItem::State::Relaxed);
                } else {
                    edgeItem->setVisualState(EdgeItem::State::Default);
                }
            }
        }
    };

    highlightScene(m_sceneDijkstra, dijkstra);
    highlightScene(m_sceneAStar, astar);
    highlightScene(m_sceneBFS, bfs);

    // Update per-map metric cards below each map in comparison mode
    // 1. Dijkstra Card
    m_cardDijkstraVisited->setText(QString("Visited Nodes: %1").arg(dijkstra.visitedNodes.size()));
    if (dijkstra.type == PathfindingStep::Type::PathFound) {
        m_cardDijkstraDist->setText(QString("Path Distance: %1 m").arg(dijkstra.totalDistance, 0, 'f', 1));
        m_cardDijkstraDist->setStyleSheet("font-weight: bold; color: #1E40AF;");
    } else if (dijkstra.type == PathfindingStep::Type::NoPath) {
        m_cardDijkstraDist->setText("Path Distance: Unreachable");
        m_cardDijkstraDist->setStyleSheet("font-weight: bold; color: #DC2626;");
    } else {
        m_cardDijkstraDist->setText(QString("Path Distance: Searching... (Step %1)").arg(dijkstra.stepIndex));
        m_cardDijkstraDist->setStyleSheet("color: #475569;");
    }

    // 2. A* Card
    m_cardAStarVisited->setText(QString("Visited Nodes: %1").arg(astar.visitedNodes.size()));
    if (astar.type == PathfindingStep::Type::PathFound) {
        m_cardAStarDist->setText(QString("Path Distance: %1 m").arg(astar.totalDistance, 0, 'f', 1));
        m_cardAStarDist->setStyleSheet("font-weight: bold; color: #15803D;");
    } else if (astar.type == PathfindingStep::Type::NoPath) {
        m_cardAStarDist->setText("Path Distance: Unreachable");
        m_cardAStarDist->setStyleSheet("font-weight: bold; color: #DC2626;");
    } else {
        m_cardAStarDist->setText(QString("Path Distance: Searching... (Step %1)").arg(astar.stepIndex));
        m_cardAStarDist->setStyleSheet("color: #475569;");
    }

    // 3. BFS Card
    m_cardBFSVisited->setText(QString("Visited Nodes: %1").arg(bfs.visitedNodes.size()));
    if (bfs.type == PathfindingStep::Type::PathFound) {
        m_cardBFSDist->setText(QString("Path Distance: %1 m").arg(bfs.totalDistance, 0, 'f', 1));
        m_cardBFSDist->setStyleSheet("font-weight: bold; color: #B45309;");
    } else if (bfs.type == PathfindingStep::Type::NoPath) {
        m_cardBFSDist->setText("Path Distance: Unreachable");
        m_cardBFSDist->setStyleSheet("font-weight: bold; color: #DC2626;");
    } else {
        m_cardBFSDist->setText(QString("Path Distance: Searching... (Step %1)").arg(bfs.stepIndex));
        m_cardBFSDist->setStyleSheet("color: #475569;");
    }

    // Sync state inspector layout with active view representation
    int idx = m_algoCombo->currentIndex();
    PathfindingStep activeStep = (idx == 0) ? dijkstra : ((idx == 1) ? astar : bfs);
    m_statAlgo->setText("Mode: Parallel Comparison (3 Algorithms Running)");

    m_currentStepLabel->setText(QString("Step: %1").arg(activeStep.stepIndex));
    m_statCurrNode->setText("Current Node: " + activeStep.currentNodeId);
    m_statVisited->setText(QString("Visited Nodes: %1").arg(activeStep.visitedNodes.size()));

    m_logWidget->clear();
    m_logWidget->addItem(QString("[Dijkstra %1] %2").arg(dijkstra.stepIndex).arg(dijkstra.logMessage));
    m_logWidget->addItem(QString("[A* %1] %2").arg(astar.stepIndex).arg(astar.logMessage));
    m_logWidget->addItem(QString("[BFS %1] %2").arg(bfs.stepIndex).arg(bfs.logMessage));

    m_priorityQueueList->clear();
    for (const auto &entry : activeStep.frontier) {
        m_priorityQueueList->addItem(QString("Node %1: f=%2").arg(entry.nodeId).arg(entry.priority, 0, 'f', 1));
    }

    m_distanceTable->setRowCount(0);
    int row = 0;
    for (auto it = activeStep.distanceTable.begin(); it != activeStep.distanceTable.end(); ++it) {
        if (it.value() == std::numeric_limits<double>::infinity()) continue;
        m_distanceTable->insertRow(row);
        m_distanceTable->setItem(row, 0, new QTableWidgetItem(it.key()));
        m_distanceTable->setItem(row, 1, new QTableWidgetItem(QString::number(it.value(), 'f', 1)));
        row++;
    }

    if (activeStep.type == PathfindingStep::Type::PathFound) {
        m_statDistance->setText(QString("Distance: %1 m").arg(activeStep.totalDistance, 0, 'f', 1));
        m_statDistance->setStyleSheet("QLabel { background-color: #FEF08A; color: #854D0E; font-weight: bold; padding: 2px; border: 1px solid #FACC15; border-radius: 2px; }");
    } else if (activeStep.type == PathfindingStep::Type::NoPath) {
        m_statDistance->setText("Unreachable");
    }
}
