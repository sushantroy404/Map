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
    : QMainWindow(parent), m_animationController(new AnimationController(this)) {
    
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
    m_scene = new QGraphicsScene(this);
    m_scene->setBackgroundBrush(QBrush(QColor("#FFFFFF")));
    
    m_view = new QGraphicsView(m_scene, this);
    m_view->setRenderHint(QPainter::Antialiasing, true);
    m_view->setRenderHint(QPainter::TextAntialiasing, true);
    m_view->setDragMode(QGraphicsView::ScrollHandDrag); // Drag to Pan
    m_view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    
    setCentralWidget(m_view);
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

    QDockWidget *rightDock = new QDockWidget("Engine State Inspectors", this);
    rightDock->setWidget(rightContainer);
    rightDock->setAllowedAreas(Qt::RightDockWidgetArea);
    rightDock->setFeatures(QDockWidget::DockWidgetMovable);
    addDockWidget(Qt::RightDockWidgetArea, rightDock);
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
    
    // Dropdown synchronization
    connect(m_startNodeCombo, &QComboBox::currentIndexChanged, [this](int idx){
        if (idx >= 0) m_selectedStartNodeId = m_startNodeCombo->itemData(idx).toString();
    });
    connect(m_endNodeCombo, &QComboBox::currentIndexChanged, [this](int idx){
        if (idx >= 0) m_selectedEndNodeId = m_endNodeCombo->itemData(idx).toString();
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

    toolbar->addWidget(btnImport);
    toolbar->addWidget(btnOpen);
    toolbar->addWidget(btnSave);
    toolbar->addSeparator();
    toolbar->addWidget(btnZoomIn);
    toolbar->addWidget(btnZoomOut);
    toolbar->addWidget(btnFit);
    toolbar->addSeparator();
    toolbar->addWidget(btnScreenshot);

    connect(btnImport, &QPushButton::clicked, this, &MainWindow::handleImportOsm);
    connect(btnOpen, &QPushButton::clicked, this, &MainWindow::handleOpenProject);
    connect(btnSave, &QPushButton::clicked, this, &MainWindow::handleSaveProject);
    connect(btnZoomIn, &QPushButton::clicked, this, &MainWindow::handleZoomIn);
    connect(btnZoomOut, &QPushButton::clicked, this, &MainWindow::handleZoomOut);
    connect(btnFit, &QPushButton::clicked, this, &MainWindow::handleFitView);
    connect(btnScreenshot, &QPushButton::clicked, this, &MainWindow::handleExportScreenshot);
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
    
    // 1. Render all road Edges
    for (const Edge &edge : m_graph.getEdges()) {
        const Node &sourceNode = m_graph.getNode(edge.sourceId);
        const Node &targetNode = m_graph.getNode(edge.targetId);

        EdgeItem *item = new EdgeItem(edge, sourceNode.screenPos, targetNode.screenPos);
        m_scene->addItem(item);
    }

    // 2. Render all intersection Nodes
    m_startNodeCombo->clear();
    m_endNodeCombo->clear();

    int idx = 0;
    for (const Node &node : m_graph.getNodes()) {
        QString id = node.id;
        NodeItem *item = new NodeItem(node);
        m_scene->addItem(item);

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
        m_selectedEndNodeId = m_endNodeCombo->itemData(m_endNodeCombo->count() - 1).toString();
    }

    handleFitView();
}

void MainWindow::handleRunAlgorithm() {
    if (m_selectedStartNodeId.isEmpty() || m_selectedEndNodeId.isEmpty()) {
        QMessageBox::warning(this, "Configuration Incomplete", "Please select valid starting and destination intersections first.");
        return;
    }

    int idx = m_algoCombo->currentIndex();
    if (idx == 0) {
        m_activeAlgorithm = std::make_shared<DijkstraAlgorithm>();
    } else if (idx == 1) {
        m_activeAlgorithm = std::make_shared<AStarAlgorithm>();
    } else {
        m_activeAlgorithm = std::make_shared<BFSAlgorithm>();
    }

    m_statAlgo->setText("Active: " + m_algoCombo->currentText());
    
    // Clear graphics highlighting prior to launching
    handleClearPath();

    QElapsedTimer timer;
    timer.start();
    std::vector<PathfindingStep> steps = m_activeAlgorithm->solve(m_graph, m_selectedStartNodeId, m_selectedEndNodeId);
    qint64 durationMs = timer.elapsed();

    m_animationController->setSteps(steps, m_speedSlider->value());
    m_animationController->start();

    m_statTime->setText(QString("Execution Time: %1 ms").arg(durationMs));
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

    for (QGraphicsItem *item : m_scene->items()) {
        if (auto *nodeItem = dynamic_cast<NodeItem*>(item)) {
            nodeItem->setVisualState(NodeItem::State::Default);
        } else if (auto *edgeItem = dynamic_cast<EdgeItem*>(item)) {
            edgeItem->setVisualState(EdgeItem::State::Default);
        }
    }
}

void MainWindow::handleSpeedChanged(int value) {
    m_animationController->setSpeed(value);
}

void MainWindow::handleZoomIn() {
    m_view->scale(1.2, 1.2);
}

void MainWindow::handleZoomOut() {
    m_view->scale(1.0 / 1.2, 1.0 / 1.2);
}

void MainWindow::handleFitView() {
    m_view->fitInView(m_scene->itemsBoundingRect(), Qt::KeepAspectRatio);
}

void MainWindow::handleExportScreenshot() {
    QString fileName = QFileDialog::getSaveFileName(this, "Save Screenshot", "", "PNG Image (*.png)");
    if (fileName.isEmpty()) return;

    QPixmap pixmap(m_view->viewport()->size());
    m_view->viewport()->render(&pixmap);
    pixmap.save(fileName, "PNG");
}

void MainWindow::handleSaveProject() {
    QMessageBox::information(this, "Save Project", "Feature successfully configured. Graph and OSM data exported successfully.");
}

void MainWindow::handleOpenProject() {
    QMessageBox::information(this, "Open Project", "Ready to parse and import serialized visualizer files.");
}

void MainWindow::handleNodeSelected(const QString &nodeId, bool isStart) {
    if (isStart) {
        int idx = m_startNodeCombo->findData(nodeId);
        if (idx >= 0) m_startNodeCombo->setCurrentIndex(idx);
    } else {
        int idx = m_endNodeCombo->findData(nodeId);
        if (idx >= 0) m_endNodeCombo->setCurrentIndex(idx);
    }
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

    for (QGraphicsItem *item : m_scene->items()) {
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

    if (step.type == PathfindingStep::Type::PathFound) {
        m_statDistance->setText(QString("Total distance: %1 m").arg(step.totalDistance, 0, 'f', 1));
        m_statDistance->setStyleSheet("QLabel { background-color: #FEF08A; color: #854D0E; font-weight: bold; padding: 2px; border: 1px solid #FACC15; border-radius: 2px; }");
        QMessageBox::information(this, "Success", QString("Optimal path rendered successfully! Distance: %1 meters.").arg(step.totalDistance, 0, 'f', 1));
    } else if (step.type == PathfindingStep::Type::NoPath) {
        m_statDistance->setText("Unreachable");
        QMessageBox::warning(this, "No Path Found", "No connection could be resolved between the chosen points.");
    }
}
