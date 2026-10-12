// SPDX-License-Identifier: GPL-3.0-or-later
#include "render/CanvasItem.h"
#include <QDir>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlError>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <algorithm>
#include <memory>

class ShellTest : public QObject {
    Q_OBJECT

  private slots:
    void init() {
        warnings.clear();
        engine = std::make_unique<QQmlApplicationEngine>();
        connect(engine.get(), &QQmlApplicationEngine::warnings, this,
                [this](const QList<QQmlError>& errors) {
                    for (const auto& error : errors)
                        warnings.append(error.toString());
                });
        engine->loadFromModule("OpenPaint", "Main");
        QCOMPARE(engine->rootObjects().size(), 1);
        window = qobject_cast<QQuickWindow*>(engine->rootObjects().first());
        QVERIFY(window);
        QVERIFY(QTest::qWaitForWindowExposed(window));
        window->requestActivate();
        QVERIFY(QTest::qWaitForWindowActive(window));
        QSignalSpy readyFrame(window, &QQuickWindow::frameSwapped);
        window->requestUpdate();
        QVERIFY(readyFrame.wait(1000));
    }

    void cleanup() {
        engine.reset();
        window = nullptr;
        QVERIFY2(warnings.isEmpty(), qPrintable(warnings.join('\n')));
    }

    void launchAndResize() {
        QVERIFY(window->isVisible());
        QVERIFY(window->title().contains(QStringLiteral("OpenPaint")));
        QVERIFY(window->findChild<QObject*>(QStringLiteral("canvas")));
        window->resize(1024, 640);
        QTRY_COMPARE(window->size(), QSize(1024, 640));
        QVERIFY(!window->grabWindow().isNull());
        for (auto* swatch : window->findChildren<QQuickItem*>(QStringLiteral("colorSwatch"))) {
            QVERIFY(swatch->width() >= 32);
            QVERIFY(swatch->mapToScene({swatch->width(), 0}).x() <= window->width());
        }
    }

    void themeActions() {
        auto* light = window->findChild<QObject*>(QStringLiteral("lightThemeAction"));
        auto* dark = window->findChild<QObject*>(QStringLiteral("darkThemeAction"));
        auto* system = window->findChild<QObject*>(QStringLiteral("systemThemeAction"));
        QVERIFY(light && dark && system);
        QVERIFY(QMetaObject::invokeMethod(dark, "trigger", Q_ARG(QObject*, nullptr)));
        QTRY_COMPARE(window->color(), QColor("#111418"));
        QVERIFY(dark->property("checked").toBool());
        QVERIFY(!light->property("checked").toBool());
        QVERIFY(capture(QStringLiteral("shell-dark-1280.png")));
        auto* menu = window->findChild<QObject*>(QStringLiteral("fileMenu"));
        QVERIFY(menu);
        QVERIFY(QMetaObject::invokeMethod(menu, "open"));
        QTRY_VERIFY(menu->property("visible").toBool());
        QVERIFY(menu->property("width").toReal() >= 240);
        QVERIFY(capture(QStringLiteral("menu-dark.png")));
        QVERIFY(QMetaObject::invokeMethod(menu, "close"));
        window->resize(1024, 640);
        QVERIFY(capture(QStringLiteral("shell-dark-1024.png")));
        QVERIFY(QMetaObject::invokeMethod(light, "trigger", Q_ARG(QObject*, nullptr)));
        QTRY_COMPARE(window->color(), QColor("#E2E5EA"));
        QVERIFY(light->property("checked").toBool());
        QVERIFY(!dark->property("checked").toBool());
        QVERIFY(capture(QStringLiteral("shell-light-1024.png")));
        window->resize(1280, 800);
        QVERIFY(capture(QStringLiteral("shell-light-1280.png")));
        QVERIFY(QMetaObject::invokeMethod(menu, "open"));
        QTRY_VERIFY(menu->property("visible").toBool());
        QVERIFY(menu->property("width").toReal() >= 240);
        QVERIFY(capture(QStringLiteral("menu-light.png")));
        QVERIFY(QMetaObject::invokeMethod(menu, "close"));
        QVERIFY(QMetaObject::invokeMethod(system, "trigger", Q_ARG(QObject*, nullptr)));
        QVERIFY(system->property("checked").toBool());
        QVERIFY(!light->property("checked").toBool());
    }

    void keyboardAboutAndEscape() {
        auto* dialog = window->findChild<QObject*>(QStringLiteral("aboutDialog"));
        QVERIFY(dialog);
        window->requestActivate();
        QVERIFY(QTest::qWaitForWindowActive(window));
        QTest::keyClick(window, Qt::Key_F1);
        QTRY_VERIFY(dialog->property("visible").toBool());
        QVERIFY(capture(QStringLiteral("dialog-light.png")));
        QTest::keyClick(window, Qt::Key_Escape);
        QTRY_VERIFY(!dialog->property("visible").toBool());
        QVERIFY(window->isVisible());
    }

    void drawingAndTransforms() {
        auto* canvas = window->findChild<CanvasItem*>(QStringLiteral("canvas"));
        auto* editor = window->findChild<Editor*>(QStringLiteral("editor"));
        QVERIFY(canvas && editor);
        const auto initial = editor->image();
        auto start = canvas->mapToScene(canvas->fromDocument({100, 100})).toPoint();
        auto end = canvas->mapToScene(canvas->fromDocument({300, 100})).toPoint();
        QTest::mousePress(window, Qt::LeftButton, {}, start);
        QTest::mouseMove(window, end);
        QTest::mouseRelease(window, Qt::LeftButton, {}, end);
        QVERIFY(editor->dirty());
        QVERIFY(editor->canUndo());
        QCOMPARE(editor->image().pixelColor(200, 100), QColor(Qt::black));
        editor->undo();
        QCOMPARE(editor->image(), initial);
        QVERIFY(!editor->dirty());
        editor->redo();
        const auto painted = editor->image();
        QTest::mousePress(window, Qt::LeftButton, {}, end + QPoint(0, 20));
        QTest::mouseMove(window, end + QPoint(0, 80));
        QTest::keyClick(window, Qt::Key_Escape);
        QTest::mouseRelease(window, Qt::LeftButton, {}, end + QPoint(0, 80));
        QCOMPARE(editor->image(), painted);
        const QPointF anchor(211.5, 173.25);
        const auto before = canvas->toDocument(anchor);
        canvas->zoomAt(1.25, anchor);
        QVERIFY(QLineF(before, canvas->toDocument(anchor)).length() < 0.00001);
        QCOMPARE(editor->image(), painted);
        editor->undo();
    }

    void interruptedAndSecondaryStroke() {
        auto* canvas = window->findChild<CanvasItem*>(QStringLiteral("canvas"));
        auto* editor = canvas->editor();
        editor->secondary = Qt::red;
        canvas->zoomAt(1.5, {200, 200});
        auto at = [&](QPointF p) { return canvas->mapToScene(canvas->fromDocument(p)).toPoint(); };
        QTest::mousePress(window, Qt::RightButton, {}, at({100, 100}));
        QTest::mouseRelease(window, Qt::RightButton, {}, at({200, 100}));
        QCOMPARE(editor->image().pixelColor(150, 100), QColor(Qt::red));
        const auto first = editor->image();
        QTest::mousePress(window, Qt::LeftButton, {}, at({100, 150}));
        QTest::mouseMove(window, at({200, 150}));
        canvas->setFocus(false);
        QVERIFY(editor->canUndo());
        const auto interrupted = editor->image();
        QTest::mouseRelease(window, Qt::LeftButton, {}, at({300, 150}));
        QCOMPARE(editor->image(), interrupted);
        editor->undo();
        QCOMPARE(editor->image(), first);
        editor->undo();
    }

    void unsavedCancelAndSaveClose() {
        auto* editor = window->findChild<Editor*>(QStringLiteral("editor"));
        auto* dialog = window->findChild<QObject*>(QStringLiteral("unsavedDialog"));
        QTemporaryDir dir;
        auto url = QUrl::fromLocalFile(dir.filePath("drawing.png"));
        editor->begin({20, 20}, false);
        editor->finish();
        const auto drawing = editor->image();
        QTest::keyClick(window, Qt::Key_N, Qt::ControlModifier);
        QTRY_VERIFY(dialog->property("visible").toBool());
        QVERIFY(QMetaObject::invokeMethod(dialog, "reject"));
        QCOMPARE(editor->image(), drawing);
        QVERIFY(editor->dirty());
        QVERIFY(window->isVisible());
        editor->saveFile(url);
        QTRY_VERIFY(!editor->busy());
        editor->begin({40, 40}, false);
        editor->finish();
        const auto final = editor->image();
        window->close();
        QTRY_VERIFY(dialog->property("visible").toBool());
        QVERIFY(window->isVisible());
        QVERIFY(QMetaObject::invokeMethod(dialog, "accept"));
        QTRY_VERIFY(!editor->busy());
        QTRY_VERIFY(!window->isVisible());
        QCOMPARE(openpaint::loadPng(url.toLocalFile()).image, final);
    }

    void failedSavePreventsClose() {
        auto* editor = window->findChild<Editor*>(QStringLiteral("editor"));
        auto* dialog = window->findChild<QObject*>(QStringLiteral("unsavedDialog"));
        auto* error = window->findChild<QObject*>(QStringLiteral("errorDialog"));
        QTemporaryDir dir;
        QVERIFY(QDir(dir.path()).mkdir("gone"));
        const auto path = dir.filePath("gone/drawing.png");
        editor->saveFile(QUrl::fromLocalFile(path));
        QTRY_VERIFY(!editor->busy());
        QVERIFY(QFile::remove(path));
        QVERIFY(QDir(dir.path()).rmdir("gone"));
        editor->begin({30, 30}, false);
        editor->finish();
        const auto drawing = editor->image();
        window->close();
        QTRY_VERIFY(dialog->property("visible").toBool());
        QVERIFY(QMetaObject::invokeMethod(dialog, "accept"));
        QTRY_VERIFY(!editor->busy());
        QTRY_VERIFY(error->property("visible").toBool());
        QVERIFY(window->isVisible());
        QVERIFY(editor->dirty());
        QCOMPARE(editor->image(), drawing);
    }

    void fileDialogCancel() {
        auto* dialog = window->findChild<QObject*>(QStringLiteral("openDialog"));
        QTest::keyClick(window, Qt::Key_O, Qt::ControlModifier);
        QTRY_VERIFY(dialog->property("visible").toBool());
        QVERIFY(QMetaObject::invokeMethod(dialog, "reject"));
        QTRY_VERIFY(!dialog->property("visible").toBool());
        QVERIFY(window->isVisible());
    }

    void renderTiming() {
        if (!qEnvironmentVariableIsSet("OPENPAINT_BENCHMARK"))
            return;
        auto* canvas = window->findChild<CanvasItem*>(QStringLiteral("canvas"));
        struct BenchmarkEditor : Editor {
            BenchmarkEditor() {
                QImage image(2048, 2048, QImage::Format_ARGB32_Premultiplied);
                image.fill(Qt::white);
                document.reset(image);
            }
        } benchmarkEditor;
        auto* previousEditor = canvas->editor();
        canvas->setEditor(&benchmarkEditor);
        QList<double> timings;
        benchmarkEditor.begin({100, 100}, false);
        for (int i = 0; i < 65; ++i) {
            QSignalSpy frame(window, &QQuickWindow::frameSwapped);
            QElapsedTimer timer;
            timer.start();
            benchmarkEditor.extend({100.0 + i * 20, 100.0 + i * 10});
            window->requestUpdate();
            QVERIFY(frame.wait(1000));
            if (i >= 5)
                timings.append(timer.nsecsElapsed() / 1000000.0);
        }
        benchmarkEditor.finish();
        std::sort(timings.begin(), timings.end());
        qInfo("2048px sample-to-frame p50 %.2f ms, p95 %.2f ms (60 samples)", timings[30],
              timings[56]);
        canvas->setEditor(previousEditor);
    }

    void quitClosesWindow() {
        auto* quit = window->findChild<QObject*>(QStringLiteral("quitAction"));
        QVERIFY(quit);
        QVERIFY(QMetaObject::invokeMethod(quit, "trigger", Q_ARG(QObject*, nullptr)));
        QTRY_VERIFY(!window->isVisible());
    }

  private:
    // Optional synthetic evidence: capture only this test window, never the desktop.
    bool capture(const QString& name) {
        const auto directory = qEnvironmentVariable("OPENPAINT_CAPTURE_DIR");
        if (directory.isEmpty())
            return true;
        if (!QDir().mkpath(directory))
            return false;
        QSignalSpy frame(window, &QQuickWindow::frameSwapped);
        window->requestUpdate();
        if (!frame.wait(1000))
            return false;
        return window->grabWindow().save(QDir(directory).filePath(name));
    }

    std::unique_ptr<QQmlApplicationEngine> engine;
    QQuickWindow* window = nullptr;
    QStringList warnings;
};

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle(QStringLiteral("Basic"));
    ShellTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "tst_shell.moc"
