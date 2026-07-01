import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Shapes

ApplicationWindow {
    id: window

    width: 1280
    height: 800
    visible: true
    title: "Pathfinding Visualizer (OSM) - Qt Quick"

    property real mapZoom: 1.0
    property real mapOffsetX: 0
    property real mapOffsetY: 0

    function zoomIn() {
        mapZoom *= 1.2
    }

    function zoomOut() {
        mapZoom /= 1.2
    }

    Shortcut {
        sequence: StandardKey.ZoomIn
        onActivated: zoomIn()
    }

    Shortcut {
        sequence: StandardKey.ZoomOut
        onActivated: zoomOut()
    }

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            spacing: 8

            Button {
                text: "Import OSM"
                onClicked: appController.importOsm()
            }

            Button {
                text: "Open Project"
                onClicked: appController.openProject()
            }

            Button {
                text: "Save Project"
                onClicked: appController.saveProject()
            }

            ToolSeparator {}

            Button {
                text: "Zoom In"
                onClicked: zoomIn()
            }

            Button {
                text: "Zoom Out"
                onClicked: zoomOut()
            }

            Button {
                text: "Fit View"
                onClicked: {
                    mapZoom = 1.0
                    mapOffsetX = 0
                    mapOffsetY = 0
                }
            }

            ToolSeparator {}

            Button {
                text: "Export Screenshot"
                onClicked: appController.exportScreenshot()
            }

            Item {
                Layout.fillWidth: true
            }
        }
    }

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal

        // Left controls panel
        Rectangle {
            SplitView.preferredWidth: 260
            color: "#F5F5F5"
            border.color: "#D8D8D8"

            ScrollView {
                anchors.fill: parent

                ColumnLayout {
                    width: parent.width
                    spacing: 12
                    anchors.margins: 10

                    Label {
                        text: "Algorithm:"
                    }

                    ComboBox {
                        id: algorithmCombo
                        Layout.fillWidth: true
                        model: [
                            "Dijkstra",
                            "A* (Distance Heuristic)",
                            "Breadth First Search (BFS)"
                        ]
                    }

                    Label {
                        text: "Start Node Intersections:"
                    }

                    ComboBox {
                        id: startNodeCombo
                        Layout.fillWidth: true
                        textRole: "label"
                        valueRole: "id"
                        model: appController.startNodes
                        onCurrentValueChanged: appController.startNodeId = currentValue
                    }

                    Label {
                        text: "End Node Intersections:"
                    }

                    ComboBox {
                        id: endNodeCombo
                        Layout.fillWidth: true
                        textRole: "label"
                        valueRole: "id"
                        model: appController.endNodes
                        onCurrentValueChanged: appController.endNodeId = currentValue
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: 2
                        rowSpacing: 6
                        columnSpacing: 6

                        Button {
                            text: "Import OSM"
                            Layout.columnSpan: 2
                            Layout.fillWidth: true
                            onClicked: appController.importOsm()
                        }

                        Button {
                            text: "Run Search"
                            Layout.columnSpan: 2
                            Layout.fillWidth: true
                            highlighted: true
                            onClicked: appController.runAlgorithm(algorithmCombo.currentText)
                        }

                        Button {
                            text: "Pause"
                            Layout.fillWidth: true
                            onClicked: appController.pauseAlgorithm()
                        }

                        Button {
                            text: "Resume"
                            Layout.fillWidth: true
                            onClicked: appController.resumeAlgorithm()
                        }

                        Button {
                            text: "Reset"
                            Layout.fillWidth: true
                            onClicked: appController.reset()
                        }

                        Button {
                            text: "Clear Path"
                            Layout.fillWidth: true
                            onClicked: appController.clearPath()
                        }
                    }

                    Label {
                        text: "Simulation Speed (ms per step):"
                    }

                    Slider {
                        id: speedSlider
                        Layout.fillWidth: true
                        from: 10
                        to: 1000
                        value: 100
                        onValueChanged: appController.speed = value
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        radius: 4
                        border.color: "#D8D8D8"
                        color: "#FFFFFF"
                        height: metricsColumn.implicitHeight + 20

                        ColumnLayout {
                            id: metricsColumn
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 6

                            Label {
                                text: "ALGORITHM METRICS"
                                font.bold: true
                                color: "#2E6DA4"
                            }

                            Label {
                                text: "Active: " + appController.activeAlgorithm
                            }

                            Label {
                                text: "Current Node: " + appController.currentNode
                            }

                            Label {
                                text: "Visited Nodes: " + appController.visitedCount
                            }

                            Label {
                                text: "Total distance: " + appController.totalDistance
                            }

                            Label {
                                text: "Execution Time: " + appController.executionTime + " ms"
                            }
                        }
                    }

                    Item {
                        Layout.fillHeight: true
                    }
                }
            }
        }

        // Center map canvas
        Rectangle {
            id: mapViewport
            SplitView.fillWidth: true
            color: "#FFFFFF"
            clip: true

            Item {
                id: mapLayer

                x: mapViewport.width / 2 + mapOffsetX
                y: mapViewport.height / 2 + mapOffsetY
                scale: mapZoom

                // Roads / edges
                Repeater {
                    model: appController.edges

                    Shape {
                        ShapePath {
                            strokeWidth: modelData.state === "shortestPath" ? 4 :
                                         modelData.state === "relaxed" ? 2.5 : 1

                            strokeColor: modelData.state === "shortestPath" ? "#4CAF50" :
                                         modelData.state === "relaxed" ? "#E6A700" :
                                         modelData.type === "primary" ? "#777777" :
                                         modelData.type === "secondary" ? "#999999" :
                                         modelData.type === "footway" ? "#B0BEC5" :
                                         "#CCCCCC"

                            fillColor: "transparent"

                            startX: modelData.x1
                            startY: modelData.y1

                            PathLine {
                                x: modelData.x2
                                y: modelData.y2
                            }
                        }
                    }
                }

                // Nodes / intersections
                Repeater {
                    model: appController.nodes

                    Rectangle {
                        width: 8
                        height: 8
                        radius: 4

                        x: modelData.x - width / 2
                        y: modelData.y - height / 2

                        border.width: 1.2

                        border.color: modelData.state === "start" ? "#4CAF50" :
                                      modelData.state === "end" ? "#D9534F" :
                                      modelData.state === "visited" ? "#2E6DA4" :
                                      modelData.state === "active" ? "#E6A700" :
                                      "#D8D8D8"

                        color: modelData.state === "start" ? "#E8F5E9" :
                               modelData.state === "end" ? "#FFEBEE" :
                               modelData.state === "visited" ? "#D9ECFF" :
                               modelData.state === "active" ? "#FFF8E1" :
                               "#FFFFFF"

                        ToolTip.visible: hoverHandler.hovered
                        ToolTip.text: "Intersection " + modelData.id +
                                      "\nLat: " + modelData.lat +
                                      "\nLon: " + modelData.lon

                        HoverHandler {
                            id: hoverHandler
                        }
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton
                hoverEnabled: true
                cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor

                property real lastMouseX: 0
                property real lastMouseY: 0

                onPressed: function(mouse) {
                    lastMouseX = mouse.x
                    lastMouseY = mouse.y
                    mouse.accepted = true
                }

                onPositionChanged: function(mouse) {
                    if (pressed) {
                        mapOffsetX += mouse.x - lastMouseX
                        mapOffsetY += mouse.y - lastMouseY

                        lastMouseX = mouse.x
                        lastMouseY = mouse.y
                    }
                }

                onWheel: function(wheel) {
                    if (wheel.angleDelta.y > 0) {
                        zoomIn()
                    } else if (wheel.angleDelta.y < 0) {
                        zoomOut()
                    }

                    wheel.accepted = true
                }
            }
        }

        // Right inspector panel
        Rectangle {
            SplitView.preferredWidth: 320
            color: "#F5F5F5"
            border.color: "#D8D8D8"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                Label {
                    text: "Frontier (Priority Queue):"
                    font.bold: true
                }

                ListView {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 150
                    clip: true
                    model: appController.frontier

                    delegate: Label {
                        width: ListView.view.width
                        text: "Node " + modelData.nodeId + ": f=" + modelData.priority
                        font.family: "Consolas"
                    }
                }

                Label {
                    text: "Distance Lookup Table (g Score):"
                    font.bold: true
                }

                ListView {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 220
                    clip: true
                    model: appController.distanceTable

                    delegate: RowLayout {
                        width: ListView.view.width

                        Label {
                            text: modelData.nodeId
                            Layout.fillWidth: true
                            font.family: "Consolas"
                        }

                        Label {
                            text: modelData.distance
                            font.family: "Consolas"
                        }
                    }
                }

                Label {
                    text: "Step: " + appController.currentStep
                    font.bold: true
                    color: "#2E6DA4"
                }

                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: appController.logs

                    delegate: Label {
                        width: ListView.view.width
                        text: modelData
                        wrapMode: Text.Wrap
                        font.family: "Consolas"
                        font.pixelSize: 11
                    }
                }
            }
        }
    }
}
