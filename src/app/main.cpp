// SPDX-License-Identifier: GPL-3.0-or-later
#include "app/Editor.h"
#include "platform/AppInfo.h"
#include "platform/WindowStartup.h"
#include <QCommandLineParser>
#include <QFileInfo>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QSurfaceFormat>
#include <QTimer>

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("OpenPaint"));
    QCoreApplication::setApplicationVersion(AppInfo::version());
    QGuiApplication::setDesktopFileName(QStringLiteral("io.github.AfshinKI.OpenPaint"));
    QGuiApplication::setWindowIcon(QIcon(QStringLiteral(":/qt/qml/OpenPaint/icons/app.svg")));
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QCoreApplication::translate("main", "OpenPaint development preview. Pencil, eraser and "
                                            "undo, image import and safe PNG save."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("image"),
                                 QCoreApplication::translate("main", "One local image to open."),
                                 QStringLiteral("[image]"));
    parser.process(app);
    if (parser.positionalArguments().size() > 1)
        parser.showHelp(2);

    // Allow translucent UI backgrounds; artwork still composites over its checkerboard.
    QQuickWindow::setDefaultAlphaBuffer(true);
    auto surfaceFormat = QSurfaceFormat::defaultFormat();
    surfaceFormat.setAlphaBufferSize(8);
    QSurfaceFormat::setDefaultFormat(surfaceFormat);

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        [] { QCoreApplication::exit(1); }, Qt::QueuedConnection);
    engine.loadFromModule("OpenPaint", "Main");
    if (!engine.rootObjects().isEmpty()) {
        if (auto* window = qobject_cast<QQuickWindow*>(engine.rootObjects().first()))
            revealAfterFirstFrame(window);
    }
    if (!parser.positionalArguments().isEmpty() && !engine.rootObjects().isEmpty()) {
        auto* editor = engine.rootObjects().first()->findChild<Editor*>(QStringLiteral("editor"));
        const auto url =
            QUrl::fromLocalFile(QFileInfo(parser.positionalArguments().first()).absoluteFilePath());
        if (editor)
            QTimer::singleShot(0, editor, [editor, url] { editor->openFile(url); });
    }
    return app.exec();
}
