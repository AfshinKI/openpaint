// SPDX-License-Identifier: GPL-3.0-or-later
#include <QCommandLineParser>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("OpenPaint"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0-dev"));
    QGuiApplication::setDesktopFileName(QStringLiteral("io.github.AfshinKI.OpenPaint"));
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QCoreApplication::translate("main", "OpenPaint development preview. Pencil, eraser and "
                                            "undo, and safe PNG open/save."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        [] { QCoreApplication::exit(1); }, Qt::QueuedConnection);
    engine.loadFromModule("OpenPaint", "Main");
    return app.exec();
}
