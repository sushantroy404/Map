#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Setup professional clean styling (Fusion layout)
    app.setStyle("Fusion");
    
    MainWindow window;
    window.setWindowTitle("Pathfinding Visualizer (OSM) - C++ Qt 6");
    window.resize(1280, 800);
    window.show();
    
    return app.exec();
}
