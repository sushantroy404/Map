#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "appcontroller.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;

    AppController controller;
    engine.rootContext()->setContextProperty("appController", &controller);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection
        );

    engine.loadFromModule("PathfindingVisualizer", "MainWindow");

    return app.exec();
}