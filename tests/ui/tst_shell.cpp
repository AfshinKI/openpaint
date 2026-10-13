// SPDX-License-Identifier: GPL-3.0-or-later
#include "color/ColorPalette.h"
#include "color/ColorSpectrum.h"
#include "io/ImageIo.h"
#include "render/CanvasItem.h"
#include <QAccessible>
#include <QColorSpace>
#include <QDesktopServices>
#include <QDir>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlError>
#include <QQmlExpression>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QScopeGuard>
#include <QSemaphore>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>
#include <QtConcurrentRun>
#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>
#include <sys/resource.h>

static QElapsedTimer startupTimer;

static QList<QQuickItem*> visualItems(QQuickItem* root, const QString& name) {
    QList<QQuickItem*> result;
    for (auto* child : root->childItems()) {
        if (child->objectName() == name)
            result.append(child);
        result.append(visualItems(child, name));
    }
    return result;
}

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
        QGuiApplication::sync();
        QVERIFY(!window->grabWindow().isNull());
        if (qEnvironmentVariableIsSet("OPENPAINT_BENCHMARK_STARTUP"))
            qInfo("M1 startup_ms %.3f", startupTimer.nsecsElapsed() / 1000000.0);
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
        for (auto* swatch : visualItems(window->contentItem(), QStringLiteral("colorSwatch"))) {
            QVERIFY(swatch->width() >= 24);
            QVERIFY(swatch->mapToScene({swatch->width(), 0}).x() <= window->width());
        }
    }

    void quickActionPlacement() {
        auto* menu = window->findChild<QQuickItem*>(QStringLiteral("primaryMenuBar"));
        auto* save = window->findChild<QQuickItem*>(QStringLiteral("quickSaveButton"));
        auto* undo = window->findChild<QQuickItem*>(QStringLiteral("quickUndoButton"));
        auto* redo = window->findChild<QQuickItem*>(QStringLiteral("quickRedoButton"));
        QVERIFY(menu && save && undo && redo);
        window->resize(1024, 640);
        QGuiApplication::sync();
        const auto right = menu->mapToScene({menu->width(), 0}).x();
        QVERIFY(save->mapToScene({0, 0}).x() >= right);
        QVERIFY(save->mapToScene({0, 0}).x() - right < 40);
        QVERIFY(save->mapToScene({0, 0}).x() < undo->mapToScene({0, 0}).x());
        QVERIFY(undo->mapToScene({0, 0}).x() < redo->mapToScene({0, 0}).x());
        QVERIFY(tabTo("quickSaveButton"));
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

    void captureReleaseUrl(const QUrl& url) {
        QCOMPARE(url, QUrl("https://github.com/AfshinKI/openpaint/releases"));
    }

    void updatesWorkflow() {
        QDesktopServices::setUrlHandler("https", this, "captureReleaseUrl");
        const auto resetHandler = qScopeGuard([] { QDesktopServices::unsetUrlHandler("https"); });
        auto* editor = window->findChild<Editor*>(QStringLiteral("editor"));
        auto* action = window->findChild<QObject*>(QStringLiteral("updatesMenuItem"));
        auto* dialog = window->findChild<QObject*>(QStringLiteral("updatesDialog"));
        QVERIFY(editor && action && dialog);
        editor->begin({30, 30}, false);
        editor->extend({80, 50});
        editor->finish();
        const auto artwork = editor->image();
        keyClick(Qt::Key_H, Qt::AltModifier);
        keyClick(Qt::Key_Down);
        keyClick(Qt::Key_Return);
        QTRY_VERIFY(dialog->property("visible").toBool());
        QVERIFY(tabTo("viewReleasesButton"));
        keyClick(Qt::Key_Space);
        window->resize(1024, 640);
        for (const auto& theme : {QString("light"), QString("dark")}) {
            auto* themeAction = window->findChild<QObject*>(theme + "ThemeAction");
            QVERIFY(QMetaObject::invokeMethod(themeAction, "trigger", Q_ARG(QObject*, nullptr)));
            QVERIFY(capture("updates-" + theme + "-1024.png"));
        }
        QSignalSpy closed(dialog, SIGNAL(closed()));
        keyClick(Qt::Key_Escape);
        QTRY_COMPARE(closed.size(), 1);
        QCOMPARE(editor->image(), artwork);
        QVERIFY(editor->dirty());
    }

    void exportDialogWorkflow() {
        auto* editor = window->findChild<Editor*>(QStringLiteral("editor"));
        auto* action = window->findChild<QObject*>(QStringLiteral("exportAction"));
        auto* options = window->findChild<QObject*>(QStringLiteral("exportOptions"));
        auto* file = window->findChild<QObject*>(QStringLiteral("exportFileDialog"));
        QVERIFY(editor && action && options && file);
        QTemporaryDir dir;
        QQmlExpression fallback(qmlContext(file), file, "FileDialog.DontUseNativeDialog");
        const auto flag = fallback.evaluate();
        QVERIFY(!fallback.hasError());
        QVERIFY(file->setProperty("options", flag));
        QVERIFY(file->setProperty("currentFolder", QUrl::fromLocalFile(dir.path())));
        QSignalSpy closed(options, SIGNAL(closed()));
        editor->begin({30, 30}, false);
        editor->extend({80, 50});
        editor->finish();
        const auto artwork = editor->image();
        QVERIFY(QMetaObject::invokeMethod(action, "trigger", Q_ARG(QObject*, nullptr)));
        QTRY_VERIFY(options->property("visible").toBool());
        QVERIFY(tabTo("exportFormat"));
        keyClick(Qt::Key_Space);
        keyClick(Qt::Key_Down);
        keyClick(Qt::Key_Down);
        keyClick(Qt::Key_Return);
        QTRY_COMPARE(options->property("formatIndex").toInt(), 1);
        QVERIFY(options->property("flatten").toBool());
        QVERIFY(tabTo("exportQuality"));
        keyClick(Qt::Key_A, Qt::ControlModifier);
        keyClick('7');
        keyClick('5');
        keyClick(Qt::Key_Tab);
        QCOMPARE(options->property("quality").toInt(), 75);
        QVERIFY(!editor->hasSelection());
        window->resize(1024, 640);
        auto* light = window->findChild<QObject*>(QStringLiteral("lightThemeAction"));
        QVERIFY(QMetaObject::invokeMethod(light, "trigger", Q_ARG(QObject*, nullptr)));
        auto* formatButton = window->findChild<QQuickItem*>(QStringLiteral("exportFormat"));
        QVERIFY(formatButton);
        QVERIFY(formatButton->width() <= 420);
        QVERIFY(capture(QStringLiteral("export-jpeg-light-1024.png")));
        auto* dark = window->findChild<QObject*>(QStringLiteral("darkThemeAction"));
        QVERIFY(QMetaObject::invokeMethod(dark, "trigger", Q_ARG(QObject*, nullptr)));
        QVERIFY(options->setProperty("formatIndex", 3));
        QVERIFY(options->property("lossless").toBool());
        QVERIFY(!options->property("flatten").toBool());
        QVERIFY(capture(QStringLiteral("export-webp-dark-1024.png")));
        keyClick(Qt::Key_Escape);
        QTRY_COMPARE(closed.size(), 1);
        QCOMPARE(editor->image(), artwork);
        QVERIFY(!file->property("visible").toBool());
        QVERIFY(QMetaObject::invokeMethod(action, "trigger", Q_ARG(QObject*, nullptr)));
        QTRY_VERIFY(options->property("visible").toBool());
        QCOMPARE(options->property("formatIndex").toInt(), 0);
        QVERIFY(tabToButton("OK"));
        keyClick(Qt::Key_Space);
        QTRY_COMPARE(closed.size(), 2);
        QTRY_VERIFY(file->property("visible").toBool());
        QCOMPARE(file->property("defaultSuffix").toString(), QString("png"));
        QVERIFY(tabTo("fileNameTextField"));
        for (char key : std::string("export.png"))
            keyClick(key);
        QVERIFY(tabToButton("Save"));
        keyClick(Qt::Key_Space);
        QTRY_VERIFY(!file->property("visible").toBool());
        QTRY_VERIFY(!editor->busy());
        QVERIFY(QFileInfo::exists(dir.filePath("export.png")));
        QCOMPARE(openpaint::loadPng(dir.filePath("export.png")).image, artwork);
        QVERIFY(editor->dirty());
        QVERIFY(editor->fileUrl().isEmpty());
        QCOMPARE(editor->image(), artwork);
    }

    void resizeDialogWorkflow() {
        auto* editor = window->findChild<Editor*>(QStringLiteral("editor"));
        auto* dialog = window->findChild<QObject*>(QStringLiteral("resizeDialog"));
        auto* widthInput = window->findChild<QObject*>(QStringLiteral("resizeWidth"));
        auto* heightInput = window->findChild<QObject*>(QStringLiteral("resizeHeight"));
        const auto original = editor->image();
        QVERIFY(tabTo("resizeButton"));
        keyClick(Qt::Key_Space);
        QTRY_VERIFY(dialog->property("visible").toBool());
        QSignalSpy closed(dialog, SIGNAL(closed()));
        keyClick(Qt::Key_N, Qt::ControlModifier);
        QVERIFY(dialog->property("visible").toBool());
        QCOMPARE(editor->image(), original);
        QVERIFY(tabTo("resizeWidth"));
        keyClick(Qt::Key_Up);
        QCOMPARE(widthInput->property("value").toInt(), 1153);
        QCOMPARE(heightInput->property("value").toInt(), 649);
        QVERIFY(QMetaObject::invokeMethod(dialog, "setPercent", Q_ARG(bool, true)));
        QVERIFY(dialog->property("percentages").toBool());
        QCOMPARE(widthInput->property("value").toInt(), 100);
        window->resize(1024, 640);
        auto* light = window->findChild<QObject*>(QStringLiteral("lightThemeAction"));
        QVERIFY(QMetaObject::invokeMethod(light, "trigger", Q_ARG(QObject*, nullptr)));
        QVERIFY(capture(QStringLiteral("resize-light-1024.png")));
        auto* dark = window->findChild<QObject*>(QStringLiteral("darkThemeAction"));
        QVERIFY(QMetaObject::invokeMethod(dark, "trigger", Q_ARG(QObject*, nullptr)));
        QVERIFY(capture(QStringLiteral("resize-dark-1024.png")));
        keyClick(Qt::Key_Escape);
        QTRY_COMPARE(closed.size(), 1);
        QCOMPARE(editor->image(), original);
        QVERIFY(!editor->dirty());
        QVERIFY(tabTo("resizeButton"));
        keyClick(Qt::Key_Space);
        QTRY_VERIFY(dialog->property("visible").toBool());
        QVERIFY(QMetaObject::invokeMethod(dialog, "setPercent", Q_ARG(bool, true)));
        QVERIFY(widthInput->setProperty("value", 50));
        QVERIFY(heightInput->setProperty("value", 50));
        QVERIFY(tabToButton("OK"));
        keyClick(Qt::Key_Space);
        QTRY_COMPARE(closed.size(), 2);
        QTRY_VERIFY(!editor->busy());
        QCOMPARE(editor->imageSize(), QSize(576, 324));
        editor->undo();
        QCOMPARE(editor->image(), original);
    }

    void imageTransformWorkflow() {
        auto* canvas = window->findChild<CanvasItem*>(QStringLiteral("canvas"));
        auto* editor = canvas->editor();
        editor->begin({100, 100}, false);
        editor->extend({240, 160});
        editor->finish();
        const auto original = editor->image();
        QVERIFY(tabTo("rotateMenuButton"));
        // Hold the pool until Escape is delivered; the keyboard helper processes events,
        // so an unblocked tiny rotation can legitimately finish before cancellation.
        auto* pool = QThreadPool::globalInstance();
        pool->waitForDone();
        const int threadCount = pool->maxThreadCount();
        pool->setMaxThreadCount(1);
        QSemaphore started, resume;
        auto gate = QtConcurrent::run([&] {
            started.release();
            resume.acquire();
        });
        started.acquire();
        editor->transformImage(0);
        keyClick(Qt::Key_Escape);
        resume.release();
        gate.waitForFinished();
        pool->setMaxThreadCount(threadCount);
        QTRY_VERIFY(!editor->busy());
        QCOMPARE(editor->image(), original);
        keyClick(Qt::Key_Space);
        auto* menu = window->findChild<QObject*>(QStringLiteral("rotateMenu"));
        QTRY_VERIFY(menu->property("visible").toBool());
        QSignalSpy closed(menu, SIGNAL(closed()));
        keyClick(Qt::Key_Down); // Rotate right is the first entry.
        keyClick(Qt::Key_Return);
        QTRY_COMPARE(closed.size(), 1);
        QTRY_VERIFY(!editor->busy());
        QCOMPARE(editor->imageSize(), QSize(648, 1152));
        QCOMPARE(editor->image(), original.transformed(QTransform().rotate(90)));
        canvas->forceActiveFocus();
        keyClick(Qt::Key_Z, Qt::ControlModifier);
        QCOMPARE(editor->image(), original);
        editor->selectRect(60, 60, 240, 140);
        window->resize(1024, 640);
        auto* light = window->findChild<QObject*>(QStringLiteral("lightThemeAction"));
        QVERIFY(QMetaObject::invokeMethod(light, "trigger", Q_ARG(QObject*, nullptr)));
        QVERIFY(capture(QStringLiteral("image-controls-light-1024.png")));
        QVERIFY(QMetaObject::invokeMethod(menu, "open"));
        QTRY_VERIFY(menu->property("visible").toBool());
        auto* dark = window->findChild<QObject*>(QStringLiteral("darkThemeAction"));
        QVERIFY(QMetaObject::invokeMethod(dark, "trigger", Q_ARG(QObject*, nullptr)));
        QVERIFY(capture(QStringLiteral("image-menu-dark.png")));
        QVERIFY(QMetaObject::invokeMethod(menu, "close"));
        QTRY_COMPARE(closed.size(), 2);
        const auto crop = editor->selectedImage();
        QVERIFY(tabTo("cropButton"));
        keyClick(Qt::Key_Space);
        QTRY_VERIFY(!editor->busy());
        QCOMPARE(editor->image(), crop);
        canvas->forceActiveFocus();
        keyClick(Qt::Key_Z, Qt::ControlModifier);
        QCOMPARE(editor->image(), original);
    }

    void freeFormSelectionWorkflow() {
        auto* canvas = window->findChild<CanvasItem*>(QStringLiteral("canvas"));
        auto* editor = canvas->editor();
        QVERIFY(tabTo("selectionMenuButton"));
        keyClick(Qt::Key_Space);
        auto* menu = window->findChild<QObject*>(QStringLiteral("selectionMenu"));
        QTRY_VERIFY(menu->property("visible").toBool());
        keyClick(Qt::Key_Down);
        keyClick(Qt::Key_Down);
        keyClick(Qt::Key_Return);
        QTRY_VERIFY(editor->freeFormSelection);
        QCOMPARE(editor->tool(), Editor::Tool::Select);
        auto at = [&](QPointF p) { return canvas->mapToScene(canvas->fromDocument(p)).toPoint(); };
        QTest::mousePress(window, Qt::LeftButton, {}, at({100, 100}));
        QTest::mouseMove(window, at({320, 120}));
        QTest::mouseMove(window, at({220, 180}));
        QTest::mouseMove(window, at({320, 300}));
        QTest::mouseRelease(window, Qt::LeftButton, {}, at({100, 300}));
        QVERIFY(editor->hasSelection());
        QVERIFY(!editor->selection().contains(QPoint(310, 200)));
        window->resize(1024, 640);
        auto* light = window->findChild<QObject*>(QStringLiteral("lightThemeAction"));
        QVERIFY(QMetaObject::invokeMethod(light, "trigger", Q_ARG(QObject*, nullptr)));
        QVERIFY(capture(QStringLiteral("lasso-light-1024.png")));
        QVERIFY(QMetaObject::invokeMethod(menu, "open"));
        QTRY_VERIFY(menu->property("visible").toBool());
        auto* dark = window->findChild<QObject*>(QStringLiteral("darkThemeAction"));
        QVERIFY(QMetaObject::invokeMethod(dark, "trigger", Q_ARG(QObject*, nullptr)));
        QVERIFY(capture(QStringLiteral("lasso-menu-dark.png")));
        QVERIFY(QMetaObject::invokeMethod(menu, "close"));
    }

    void clipboardMoveWorkflow() {
        auto* canvas = window->findChild<CanvasItem*>(QStringLiteral("canvas"));
        auto* editor = canvas->editor();
        editor->begin({110, 110}, false);
        editor->extend({150, 150});
        editor->finish();
        const auto original = editor->image();
        editor->selectRect(100, 100, 80, 80);
        canvas->forceActiveFocus();
        keyClick(Qt::Key_C, Qt::ControlModifier);
        QTRY_VERIFY(!editor->busy());
        keyClick(Qt::Key_V, Qt::ControlModifier);
        QTRY_VERIFY(!editor->busy());
        QVERIFY(editor->floatingActive());
        keyClick(Qt::Key_Right, Qt::ShiftModifier);
        QCOMPARE(editor->floatingPosition(), QPoint(10, 0));
        keyClick(Qt::Key_Down);
        QCOMPARE(editor->floatingPosition(), QPoint(10, 1));
        editor->moveFloating(240, 120);
        auto at = [&](QPointF p) { return canvas->mapToScene(canvas->fromDocument(p)).toPoint(); };
        QTest::mousePress(window, Qt::LeftButton, {}, at({260, 140}));
        QTest::mouseMove(window, at({300, 170}));
        QTest::mouseRelease(window, Qt::LeftButton, {}, at({300, 170}));
        QVERIFY(editor->floatingActive());
        QVERIFY(std::abs(editor->floatingPosition().x() - 280) <= 1);
        window->resize(1024, 640);
        auto* light = window->findChild<QObject*>(QStringLiteral("lightThemeAction"));
        QVERIFY(QMetaObject::invokeMethod(light, "trigger", Q_ARG(QObject*, nullptr)));
        QVERIFY(capture(QStringLiteral("clipboard-light-1024.png")));
        auto* options = window->findChild<QObject*>(QStringLiteral("selectionOptions"));
        QVERIFY(QMetaObject::invokeMethod(options, "open"));
        QTRY_VERIFY(options->property("visible").toBool());
        auto* dark = window->findChild<QObject*>(QStringLiteral("darkThemeAction"));
        QVERIFY(QMetaObject::invokeMethod(dark, "trigger", Q_ARG(QObject*, nullptr)));
        QVERIFY(capture(QStringLiteral("clipboard-position-dark.png")));
        QSignalSpy closed(options, SIGNAL(closed()));
        QVERIFY(QMetaObject::invokeMethod(options, "close"));
        QTRY_COMPARE(closed.size(), 1);
        canvas->forceActiveFocus();
        const auto pasted = editor->image();
        keyClick(Qt::Key_Return);
        QVERIFY(!editor->floatingActive());
        keyClick(Qt::Key_Z, Qt::ControlModifier);
        QCOMPARE(editor->image(), original);
        keyClick(Qt::Key_Y, Qt::ControlModifier);
        QCOMPARE(editor->image(), pasted);
        keyClick(Qt::Key_X, Qt::ControlModifier);
        QTRY_VERIFY(!editor->busy());
        QVERIFY(editor->image() != pasted);
        keyClick(Qt::Key_Z, Qt::ControlModifier);
        QCOMPARE(editor->image(), pasted);
        keyClick(Qt::Key_V, Qt::ControlModifier);
        QTRY_VERIFY(!editor->busy());
        QVERIFY(editor->floatingActive());
        keyClick(Qt::Key_Escape);
        QCOMPARE(editor->image(), pasted);
    }

    void rectangularSelectionWorkflow() {
        auto* canvas = window->findChild<CanvasItem*>(QStringLiteral("canvas"));
        auto* editor = canvas->editor();
        const auto base = editor->image();
        QVERIFY(tabTo("selectionButton"));
        keyClick(Qt::Key_Space);
        QCOMPARE(editor->tool(), Editor::Tool::Select);
        auto at = [&](QPointF p) { return canvas->mapToScene(canvas->fromDocument(p)).toPoint(); };
        QTest::mousePress(window, Qt::LeftButton, {}, at({100, 100}));
        QTest::mouseMove(window, at({240, 200}));
        QTest::mouseRelease(window, Qt::LeftButton, {}, at({240, 200}));
        QVERIFY(editor->hasSelection());
        QVERIFY(!editor->dirty());
        keyClick(Qt::Key_Delete);
        QCOMPARE(editor->image().pixelColor(150, 150).alpha(), 0);
        QCOMPARE(editor->image().pixelColor(50, 150), QColor(Qt::white));
        keyClick(Qt::Key_Z, Qt::ControlModifier);
        QCOMPARE(editor->image(), base);
        keyClick(Qt::Key_A, Qt::ControlModifier);
        QCOMPARE(editor->selectionRect(), base.rect());
        keyClick(Qt::Key_D, Qt::ControlModifier);
        QVERIFY(!editor->hasSelection());
        editor->selectRect(100, 100, 240, 180);
        window->resize(1024, 640);
        auto* light = window->findChild<QObject*>(QStringLiteral("lightThemeAction"));
        QVERIFY(QMetaObject::invokeMethod(light, "trigger", Q_ARG(QObject*, nullptr)));
        QVERIFY(capture(QStringLiteral("selection-light-1024.png")));
        auto* menu = window->findChild<QObject*>(QStringLiteral("selectionMenu"));
        QVERIFY(QMetaObject::invokeMethod(menu, "open"));
        QTRY_VERIFY(menu->property("visible").toBool());
        QVERIFY(capture(QStringLiteral("selection-menu-light.png")));
        auto* dark = window->findChild<QObject*>(QStringLiteral("darkThemeAction"));
        QVERIFY(QMetaObject::invokeMethod(dark, "trigger", Q_ARG(QObject*, nullptr)));
        QVERIFY(capture(QStringLiteral("selection-menu-dark.png")));
        QSignalSpy closed(menu, SIGNAL(closed()));
        QVERIFY(QMetaObject::invokeMethod(menu, "close"));
        QTRY_COMPARE(closed.size(), 1);
        auto* options = window->findChild<QObject*>(QStringLiteral("selectionOptions"));
        QVERIFY(QMetaObject::invokeMethod(options, "open"));
        QTRY_VERIFY(options->property("visible").toBool());
        QVERIFY(capture(QStringLiteral("selection-bounds-dark.png")));
        QVERIFY(QMetaObject::invokeMethod(options, "close"));
    }

    void inlineTextWorkflow() {
        auto* canvas = window->findChild<CanvasItem*>(QStringLiteral("canvas"));
        auto* editor = canvas->editor();
        auto* input = window->findChild<QQuickItem*>(QStringLiteral("inlineTextInput"));
        auto* transition = window->findChild<QObject*>(QStringLiteral("textTransition"));
        auto* options = window->findChild<QObject*>(QStringLiteral("textOptions"));
        QVERIFY(input && transition && options);
        const auto original = editor->image();
        editor->setTool(Editor::Tool::Text);
        auto at = [&](QPointF p) { return canvas->mapToScene(canvas->fromDocument(p)).toPoint(); };
        QTest::mousePress(window, Qt::LeftButton, {}, at({80, 60}));
        QTest::mouseMove(window, at({400, 180}));
        QTest::mouseRelease(window, Qt::LeftButton, {}, at({400, 180}));
        QTRY_VERIFY(editor->textReady());
        QTRY_VERIFY(input->hasActiveFocus());
        QVERIFY(editor->textRect().width() >= 319);
        for (char c : std::string("OpenPaint"))
            keyClick(c);
        QCOMPARE(editor->textDraft(), QStringLiteral("OpenPaint"));
        keyClick(Qt::Key_Z, Qt::ControlModifier);
        QVERIFY(editor->textActive());
        QVERIFY(editor->textDraft() != QStringLiteral("OpenPaint"));
        keyClick(Qt::Key_Y, Qt::ControlModifier);
        QCOMPARE(editor->textDraft(), QStringLiteral("OpenPaint"));
        keyClick(Qt::Key_A, Qt::ControlModifier);
        keyClick(Qt::Key_C, Qt::ControlModifier);
        keyClick(Qt::Key_End);
        keyClick(Qt::Key_V, Qt::ControlModifier);
        QCOMPARE(editor->textDraft(), QStringLiteral("OpenPaintOpenPaint"));
        keyClick(Qt::Key_Z, Qt::ControlModifier);
        QCOMPARE(editor->textDraft(), QStringLiteral("OpenPaint"));
        const auto draft = editor->image();
        keyClick(Qt::Key_N, Qt::ControlModifier);
        QTRY_VERIFY(transition->property("visible").toBool());
        QSignalSpy closed(transition, SIGNAL(closed()));
        QVERIFY(QMetaObject::invokeMethod(transition, "reject"));
        QTRY_COMPARE(closed.size(), 1);
        QTRY_VERIFY(input->hasActiveFocus());
        QCOMPARE(editor->image(), draft);
        QVERIFY(editor->textActive());
        auto* light = window->findChild<QObject*>(QStringLiteral("lightThemeAction"));
        QVERIFY(QMetaObject::invokeMethod(light, "trigger", Q_ARG(QObject*, nullptr)));
        window->resize(1024, 640);
        QVERIFY(capture(QStringLiteral("text-light-1024.png")));
        QVERIFY(QMetaObject::invokeMethod(options, "open"));
        QTRY_VERIFY(options->property("visible").toBool());
        QVERIFY(capture(QStringLiteral("text-options-light.png")));
        auto* dark = window->findChild<QObject*>(QStringLiteral("darkThemeAction"));
        QVERIFY(QMetaObject::invokeMethod(dark, "trigger", Q_ARG(QObject*, nullptr)));
        QVERIFY(capture(QStringLiteral("text-options-dark.png")));
        QSignalSpy optionsClosed(options, SIGNAL(closed()));
        QVERIFY(QMetaObject::invokeMethod(options, "close"));
        QTRY_COMPARE(optionsClosed.size(), 1);
        QTRY_VERIFY(input->hasActiveFocus());
        keyClick(Qt::Key_Return, Qt::ControlModifier);
        QVERIFY(!editor->textActive());
        QCOMPARE(editor->image(), draft);
        editor->undo();
        QCOMPARE(editor->image(), original);
        editor->redo();
        QCOMPARE(editor->image(), draft);
        editor->begin({30, 30}, false);
        editor->endGesture();
        QTRY_VERIFY(input->hasActiveFocus());
        keyClick('x');
        keyClick(Qt::Key_Escape);
        QVERIFY(!editor->textActive());
        QCOMPARE(editor->image(), draft);
        editor->begin({30, 30}, false);
        editor->endGesture();
        keyClick('x');
        keyClick(Qt::Key_N, Qt::ControlModifier);
        QTRY_VERIFY(transition->property("visible").toBool());
        QQmlExpression discard(qmlContext(transition), transition,
                               "footer.standardButton(Dialog.Discard)");
        auto* discardButton = qvariant_cast<QObject*>(discard.evaluate());
        QVERIFY(discardButton && !discard.hasError());
        QVERIFY(QMetaObject::invokeMethod(discardButton, "click"));
        auto* unsaved = window->findChild<QObject*>(QStringLiteral("unsavedDialog"));
        QTRY_VERIFY(unsaved->property("visible").toBool());
        QVERIFY(!editor->textActive());
        QCOMPARE(editor->image(), draft);
        QSignalSpy unsavedClosed(unsaved, SIGNAL(closed()));
        QVERIFY(QMetaObject::invokeMethod(unsaved, "reject"));
        QTRY_COMPARE(unsavedClosed.size(), 1);
        editor->begin({30, 30}, false);
        editor->endGesture();
        keyClick('y');
        const auto applied = editor->image();
        keyClick(Qt::Key_N, Qt::ControlModifier);
        QTRY_VERIFY(transition->property("visible").toBool());
        QQmlExpression apply(qmlContext(transition), transition,
                             "footer.standardButton(Dialog.Apply)");
        auto* applyButton = qvariant_cast<QObject*>(apply.evaluate());
        QVERIFY(applyButton && !apply.hasError());
        QVERIFY(QMetaObject::invokeMethod(applyButton, "click"));
        QTRY_VERIFY(unsaved->property("visible").toBool());
        QVERIFY(!editor->textActive());
        QCOMPARE(editor->image(), applied);
        QVERIFY(QMetaObject::invokeMethod(unsaved, "reject"));
        QTRY_COMPARE(unsavedClosed.size(), 2);
        editor->undo();
        QCOMPARE(editor->image(), draft);
    }

    void chordAndPanLifecycle() {
        auto* canvas = window->findChild<CanvasItem*>(QStringLiteral("canvas"));
        auto* editor = canvas->editor();
        const auto original = editor->image();
        auto at = [&](QPointF p) { return canvas->mapToScene(canvas->fromDocument(p)).toPoint(); };
        const auto start = at({100, 100});
        const auto middle = at({200, 100});
        const auto end = at({300, 100});
        QTest::mousePress(window, Qt::LeftButton, {}, start);
        QTest::mouseMove(window, middle);
        QTest::mousePress(window, Qt::MiddleButton, {}, middle);
        QTest::mouseRelease(window, Qt::MiddleButton, {}, middle);
        QTest::mouseMove(window, end);
        QTest::mouseRelease(window, Qt::LeftButton, {}, end);
        QCOMPARE(editor->image().pixelColor(250, 100), QColor(Qt::black));
        editor->undo();
        QCOMPARE(editor->image(), original);
        QVERIFY(!editor->canUndo());
        const auto origin = canvas->fromDocument({0, 0});
        QTest::keyPress(window, Qt::Key_Space);
        QTest::mousePress(window, Qt::LeftButton, {}, middle);
        QTest::mouseMove(window, middle + QPoint(20, 10));
        QTest::mouseRelease(window, Qt::LeftButton, {}, middle + QPoint(40, 30));
        QTest::keyRelease(window, Qt::Key_Space);
        QVERIFY(QLineF(canvas->fromDocument({0, 0}), origin + QPointF(40, 30)).length() < 0.01);
        QCOMPARE(editor->image(), original);
        QVERIFY(!editor->dirty());
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

    void nativePortalWorkflow() {
        const auto directory = qEnvironmentVariable("OPENPAINT_PORTAL_DIR");
        if (directory.isEmpty())
            return;
        const auto token = qEnvironmentVariable("OPENPAINT_PORTAL_TOKEN");
        QVERIFY(!token.isEmpty());
        auto* editor = window->findChild<Editor*>(QStringLiteral("editor"));
        auto* save = window->findChild<QObject*>(QStringLiteral("saveDialog"));
        auto* open = window->findChild<QObject*>(QStringLiteral("openDialog"));
        const auto url = QUrl::fromLocalFile(QDir(directory).filePath("portal.png"));
        QVERIFY(save->setProperty("title", "OpenPaint validation save " + token));
        QVERIFY(open->setProperty("title", "OpenPaint validation open " + token));
        QVERIFY(save->setProperty("currentFolder", QUrl::fromLocalFile(directory)));
        QVERIFY(save->setProperty("selectedFile", url));
        QVERIFY(open->setProperty("currentFolder", QUrl::fromLocalFile(directory)));
        editor->begin({50, 50}, false);
        editor->extend({160, 140});
        editor->finish();
        keyClick(Qt::Key_S, Qt::ControlModifier);
        QTRY_VERIFY_WITH_TIMEOUT(!editor->fileUrl().isEmpty(), 30000);
        QTRY_VERIFY(!editor->busy());
        QCOMPARE(editor->fileUrl(), url);
        QVERIFY(!editor->dirty());
        QCOMPARE(openpaint::loadPng(url.toLocalFile()).image, editor->image());
        // Replace the selected destination through the same safe save path.
        editor->begin({220, 220}, false);
        editor->finish();
        const auto artwork = editor->image();
        keyClick(Qt::Key_S, Qt::ControlModifier);
        QTRY_VERIFY(!editor->busy());
        QVERIFY(!editor->dirty());
        QCOMPARE(openpaint::loadPng(url.toLocalFile()).image, artwork);
        keyClick(Qt::Key_N, Qt::ControlModifier);
        keyClick(Qt::Key_O, Qt::ControlModifier);
        QTRY_COMPARE_WITH_TIMEOUT(editor->fileUrl(), url, 30000);
        QTRY_VERIFY(!editor->busy());
        QCOMPARE(editor->image(), artwork);
        QTRY_VERIFY(window->isActive());
        QVERIFY(open->setProperty("title", "OpenPaint validation cancel " + token));
        keyClick(Qt::Key_O, Qt::ControlModifier);
        QTRY_VERIFY(open->property("visible").toBool());
        QTRY_VERIFY_WITH_TIMEOUT(!open->property("visible").toBool(), 30000);
        QCOMPARE(editor->image(), artwork);
        QVERIFY(!editor->dirty());
    }

    void keyboardFileWorkflow() {
        auto* editor = window->findChild<Editor*>(QStringLiteral("editor"));
        auto* save = window->findChild<QObject*>(QStringLiteral("saveDialog"));
        auto* open = window->findChild<QObject*>(QStringLiteral("openDialog"));
        auto* unsaved = window->findChild<QObject*>(QStringLiteral("unsavedDialog"));
        QVERIFY(editor && save && open && unsaved);
        QTemporaryDir dir;
        const auto url = QUrl::fromLocalFile(dir.filePath("keyboard.png"));
        // Select the documented Qt fallback for repeatable keyboard tests.
        QQmlExpression options(qmlContext(save), save, "FileDialog.DontUseNativeDialog");
        const auto flag = options.evaluate();
        QVERIFY(!options.hasError());
        QVERIFY(save->setProperty("options", flag));
        QVERIFY(open->setProperty("options", flag));
        QVERIFY(save->setProperty("currentFolder", QUrl::fromLocalFile(dir.path())));
        QVERIFY(open->setProperty("currentFolder", QUrl::fromLocalFile(dir.path())));
        editor->begin({40, 40}, false);
        editor->extend({180, 120});
        editor->finish();
        const auto artwork = editor->image();
        keyClick(Qt::Key_Z, Qt::ControlModifier);
        QVERIFY(!editor->dirty());
        keyClick(Qt::Key_Z, Qt::ControlModifier | Qt::ShiftModifier);
        QCOMPARE(editor->image(), artwork);
        keyClick(Qt::Key_Z, Qt::ControlModifier);
        keyClick(Qt::Key_Y, Qt::ControlModifier);
        QCOMPARE(editor->image(), artwork);
        keyClick(Qt::Key_S, Qt::ControlModifier);
        QTRY_VERIFY(save->property("visible").toBool());
        QVERIFY(tabTo("fileNameTextField"));
        for (char key : std::string("keyboard.png"))
            keyClick(key);
        QVERIFY(tabToButton("Save"));
        keyClick(Qt::Key_Space);
        QTRY_VERIFY(!save->property("visible").toBool());
        QTRY_VERIFY(!editor->busy());
        QCOMPARE(editor->fileUrl(), url);
        QVERIFY(!editor->dirty());
        QCOMPARE(openpaint::loadPng(url.toLocalFile()).image, artwork);
        keyClick(Qt::Key_N, Qt::ControlModifier);
        QVERIFY(editor->fileUrl().isEmpty());
        keyClick(Qt::Key_O, Qt::ControlModifier);
        QTRY_VERIFY(open->property("visible").toBool());
        QVERIFY(tabTo("fileDialogListView"));
        auto* chooserWindow = qobject_cast<QQuickWindow*>(QGuiApplication::focusWindow());
        QVERIFY(chooserWindow);
        auto* list = chooserWindow->activeFocusItem();
        while (list && list->objectName() != "fileDialogListView")
            list = list->parentItem();
        QVERIFY(list);
        QTRY_COMPARE(list->property("count").toInt(), 1);
        keyClick(Qt::Key_Home);
        keyClick(Qt::Key_Down);
        QVERIFY(tabToButton("Open"));
        keyClick(Qt::Key_Space);
        QTRY_VERIFY(!open->property("visible").toBool());
        QTRY_VERIFY(!editor->busy());
        QCOMPARE(editor->image(), artwork);
        editor->begin({220, 220}, false);
        editor->finish();
        const auto changed = editor->image();
        keyClick(Qt::Key_N, Qt::ControlModifier);
        QTRY_VERIFY(unsaved->property("visible").toBool());
        keyClick(Qt::Key_Escape);
        QTRY_VERIFY(!unsaved->property("visible").toBool());
        QCOMPARE(editor->image(), changed);
        keyClick(Qt::Key_S, Qt::ControlModifier);
        QTRY_VERIFY(!editor->busy());
        QVERIFY(!editor->dirty());
        QCOMPARE(openpaint::loadPng(url.toLocalFile()).image, changed);
        editor->begin({280, 280}, false);
        editor->finish();
        const auto savedFromPrompt = editor->image();
        keyClick(Qt::Key_N, Qt::ControlModifier);
        QTRY_VERIFY(unsaved->property("visible").toBool());
        QVERIFY(tabToButton("Save"));
        keyClick(Qt::Key_Space);
        QTRY_VERIFY(!editor->busy());
        QTRY_VERIFY(editor->fileUrl().isEmpty());
        QCOMPARE(openpaint::loadPng(url.toLocalFile()).image, savedFromPrompt);
        editor->begin({30, 30}, false);
        editor->finish();
        keyClick(Qt::Key_N, Qt::ControlModifier);
        QTRY_VERIFY(unsaved->property("visible").toBool());
        QQmlExpression discard(qmlContext(unsaved), unsaved,
                               "footer.standardButton(Dialog.Discard)");
        auto* discardButton = qvariant_cast<QQuickItem*>(discard.evaluate());
        QVERIFY(!discard.hasError() && discardButton);
        QVERIFY(tabToItem(discardButton));
        QSignalSpy closed(unsaved, SIGNAL(closed()));
        keyClick(Qt::Key_Space);
        QTRY_VERIFY(!unsaved->property("visible").toBool());
        QVERIFY(!editor->dirty());
        QCOMPARE(openpaint::loadPng(url.toLocalFile()).image, savedFromPrompt);
        QTRY_COMPARE(closed.size(), 1);
        keyClick(Qt::Key_O, Qt::ControlModifier);
        QTRY_VERIFY(open->property("visible").toBool());
        keyClick(Qt::Key_Escape);
        QTRY_VERIFY(!open->property("visible").toBool());
        keyClick(Qt::Key_Q, Qt::ControlModifier);
        QTRY_VERIFY(!window->isVisible());
    }

    void ribbonShapeKeyboard() {
        auto* editor = window->findChild<Editor*>(QStringLiteral("editor"));
        QVERIFY(tabTo("ribbonShape0"));
        for (int i = 0; i < 18; ++i)
            keyClick(Qt::Key_Right);
        QCOMPARE(window->activeFocusItem()->objectName(), QString("ribbonShape18"));
        keyClick(Qt::Key_Space);
        QCOMPARE(editor->shapeKind, 18);
        QCOMPARE(editor->tool(), Editor::Tool::Shape);
        const auto* item = window->activeFocusItem();
        const auto bounds = item->mapRectToScene(item->boundingRect());
        QVERIFY(bounds.top() >= 40 && bounds.bottom() <= 152);
        QVERIFY(!editor->dirty());
    }

    void shapePaletteKeyboardAndLayout() {
        QVERIFY(tabTo("shapesButton"));
        keyClick(Qt::Key_Space);
        auto* palette = window->findChild<QObject*>(QStringLiteral("shapesMenu"));
        QTRY_VERIFY(palette->property("visible").toBool());
        window->resize(1024, 640);
        QVERIFY(QMetaObject::invokeMethod(
            window->findChild<QObject*>(QStringLiteral("lightThemeAction")), "trigger",
            Q_ARG(QObject*, nullptr)));
        QVERIFY(capture(QStringLiteral("shape-palette-light.png")));
        QVERIFY(QMetaObject::invokeMethod(
            window->findChild<QObject*>(QStringLiteral("darkThemeAction")), "trigger",
            Q_ARG(QObject*, nullptr)));
        QVERIFY(capture(QStringLiteral("shape-palette-dark.png")));
        keyClick(Qt::Key_Down);
        keyClick(Qt::Key_Down);
        keyClick(Qt::Key_Down);
        QCOMPARE(window->activeFocusItem()->objectName(), QString("shapeChoice15"));
        keyClick(Qt::Key_Space);
        QTRY_VERIFY(!palette->property("visible").toBool());
        auto* editor = window->findChild<Editor*>(QStringLiteral("editor"));
        QCOMPARE(editor->shapeKind, 15);
        QCOMPARE(editor->tool(), Editor::Tool::Shape);
    }
    void shapeOptionsKeyboard() {
        auto* editor = window->findChild<Editor*>(QStringLiteral("editor"));
        const auto original = editor->image();
        QVERIFY(tabTo("shapesButton"));
        keyClick(Qt::Key_Space);
        keyClick(Qt::Key_Right);
        keyClick(Qt::Key_Space);
        QCOMPARE(editor->tool(), Editor::Tool::Shape);
        QCOMPARE(editor->shapeKind, 1);
        editor->begin({80, 80}, false);
        editor->extend({400, 300});
        editor->endGesture();
        QVERIFY(tabTo("shapeOptionsButton"));
        keyClick(Qt::Key_Space);
        auto* options = window->findChild<QObject*>(QStringLiteral("shapeOptions"));
        QTRY_VERIFY(options->property("visible").toBool());
        QVERIFY(tabTo("shapeStartX"));
        keyClick(Qt::Key_Up);
        QCOMPARE(editor->shapeStart().x(), 81.0);
        window->resize(1024, 640);
        QVERIFY(QMetaObject::invokeMethod(
            window->findChild<QObject*>(QStringLiteral("lightThemeAction")), "trigger",
            Q_ARG(QObject*, nullptr)));
        QVERIFY(capture(QStringLiteral("shape-options-light.png")));
        QVERIFY(QMetaObject::invokeMethod(
            window->findChild<QObject*>(QStringLiteral("darkThemeAction")), "trigger",
            Q_ARG(QObject*, nullptr)));
        QVERIFY(capture(QStringLiteral("shape-options-dark.png")));
        QVERIFY(tabToButton(QStringLiteral("Apply")));
        keyClick(Qt::Key_Space);
        QTRY_VERIFY(!options->property("visible").toBool());
        QVERIFY(!editor->previewActive());
        editor->undo();
        QCOMPARE(editor->image(), original);
        editor->begin({80, 80}, false);
        editor->extend({400, 300});
        editor->endGesture();
        QVERIFY(QMetaObject::invokeMethod(options, "open"));
        QTRY_VERIFY(options->property("visible").toBool());
        keyClick(Qt::Key_Escape);
        QTRY_VERIFY(!options->property("visible").toBool());
        QVERIFY(!editor->previewActive());
        QCOMPARE(editor->image(), original);
    }
    void brushMenuKeyboardAndLayout() {
        auto* editor = window->findChild<Editor*>(QStringLiteral("editor"));
        QVERIFY(QMetaObject::invokeMethod(
            window->findChild<QObject*>(QStringLiteral("lightThemeAction")), "trigger",
            Q_ARG(QObject*, nullptr)));
        QVERIFY(tabTo("brushesButton"));
        keyClick(Qt::Key_Space);
        auto* menu = window->findChild<QObject*>(QStringLiteral("brushesMenu"));
        QVERIFY(menu);
        QTRY_VERIFY(menu->property("visible").toBool());
        QCOMPARE(menu->property("count").toInt(), 9);
        window->resize(1024, 640);
        QVERIFY(capture(QStringLiteral("brush-menu-light.png")));
        window->resize(1280, 800);
        QVERIFY(capture(QStringLiteral("brush-menu-light-1280.png")));
        window->resize(1024, 640);
        keyClick(Qt::Key_Down);
        QCOMPARE(menu->property("currentIndex").toInt(), 0);
        keyClick(Qt::Key_Down);
        QCOMPARE(menu->property("currentIndex").toInt(), 1);
        keyClick(Qt::Key_Return);
        QTRY_VERIFY(!menu->property("visible").toBool());
        QCOMPARE(editor->tool(), Editor::Tool::Brush);
        QCOMPARE(editor->brushStyle, 1);
        QVERIFY(tabTo("brushOpacity"));
        keyClick(Qt::Key_Down);
        QCOMPARE(editor->opacity, 99);
        QVERIFY(QMetaObject::invokeMethod(
            window->findChild<QObject*>(QStringLiteral("darkThemeAction")), "trigger",
            Q_ARG(QObject*, nullptr)));
        QVERIFY(QMetaObject::invokeMethod(menu, "open"));
        QVERIFY(capture(QStringLiteral("brush-menu-dark.png")));
        window->resize(1280, 800);
        QVERIFY(capture(QStringLiteral("brush-menu-dark-1280.png")));
        keyClick(Qt::Key_Escape);
    }
    void fillOptionsLayout() {
        auto* editor = window->findChild<Editor*>(QStringLiteral("editor"));
        editor->setTool(Editor::Tool::Fill);
        window->setWidth(1024);
        window->setHeight(640);
        QVERIFY(QMetaObject::invokeMethod(
            window->findChild<QObject*>(QStringLiteral("lightThemeAction")), "trigger",
            Q_ARG(QObject*, nullptr)));
        QVERIFY(capture(QStringLiteral("fill-light-1024.png")));
        QVERIFY(QMetaObject::invokeMethod(
            window->findChild<QObject*>(QStringLiteral("darkThemeAction")), "trigger",
            Q_ARG(QObject*, nullptr)));
        QVERIFY(capture(QStringLiteral("fill-dark-1024.png")));
    }
    void keyboardControlsAndAccessibility() {
        auto* editor = window->findChild<Editor*>(QStringLiteral("editor"));
        QVERIFY(tabTo("pencilButton"));
        auto* focused = window->activeFocusItem();
        auto* accessible = QAccessible::queryAccessibleInterface(focused);
        QVERIFY(accessible);
        QCOMPARE(accessible->text(QAccessible::Name), QString("Pencil"));
        QVERIFY(accessible->state().focusable);
        QVERIFY(tabTo("eraserButton"));
        keyClick(Qt::Key_Space);
        QCOMPARE(editor->tool(), Editor::Tool::Eraser);
        QVERIFY(tabTo("pickerButton"));
        keyClick(Qt::Key_Space);
        QCOMPARE(editor->tool(), Editor::Tool::ColorPicker);
        accessible = QAccessible::queryAccessibleInterface(window->activeFocusItem());
        QVERIFY(accessible);
        QCOMPARE(accessible->text(QAccessible::Name), QString("Color picker"));
        QVERIFY(accessible->state().checked);
        keyClick(Qt::Key_Space);
        QVERIFY(accessible->state().checked);
        QVERIFY(tabTo("strokeSize"));
        const auto previousSize = editor->brushSize;
        keyClick(Qt::Key_Up);
        QCOMPARE(editor->brushSize, previousSize + 1);
        QVERIFY(tabTo("fillButton"));
        keyClick(Qt::Key_Space);
        QCOMPARE(editor->tool(), Editor::Tool::Fill);
        QVERIFY(tabTo("strokeSize"));
        keyClick(Qt::Key_Up);
        QCOMPARE(editor->tolerance, 1);
        QVERIFY(tabTo("pencilButton"));
        keyClick(Qt::Key_Space);
        QVERIFY(tabTo("editColorsButton"));
        keyClick(Qt::Key_Space);
        auto* colors = window->findChild<QObject*>(QStringLiteral("colorEditor"));
        QTRY_VERIFY(colors->property("visible").toBool());
        keyClick(Qt::Key_Escape);
        QTRY_VERIFY(!colors->property("visible").toBool());
        auto* canvas = window->findChild<QQuickItem*>(QStringLiteral("canvas"));
        accessible = QAccessible::queryAccessibleInterface(canvas);
        QVERIFY(accessible);
        QCOMPARE(accessible->text(QAccessible::Name), QString("Drawing canvas"));
        QCOMPARE(accessible->role(), QAccessible::Canvas);
    }

    void customColorEditor() {
        auto* editor = window->findChild<Editor*>(QStringLiteral("editor"));
        auto* dialog = window->findChild<QObject*>(QStringLiteral("colorEditor"));
        auto* selection = window->findChild<ColorSelection*>(QStringLiteral("colorSelection"));
        auto* primary = window->findChild<QQuickItem*>(QStringLiteral("primaryButton"));
        auto* secondary = window->findChild<QQuickItem*>(QStringLiteral("secondaryButton"));
        auto* spectrum = window->findChild<ColorSpectrum*>(QStringLiteral("colorSpectrum"));
        auto* palette = window->findChild<ColorPalette*>(QStringLiteral("customPalette"));
        auto click = [&](QQuickItem* item) {
            QTest::mouseClick(window, Qt::LeftButton, {},
                              item->mapToScene({item->width() / 2, item->height() / 2}).toPoint());
        };
        QVERIFY(editor && dialog && selection && primary && secondary && spectrum && palette);
        const auto original = editor->primary;
        const auto pixels = editor->image();
        const auto emptySlots =
            visualItems(window->contentItem(), QStringLiteral("customToolbarSwatch"));
        QCOMPARE(emptySlots.size(), 10);
        for (auto* slot : emptySlots)
            QVERIFY(slot->property("empty").toBool());
        click(emptySlots.first());
        QTRY_VERIFY(dialog->property("visible").toBool());
        QCOMPARE(editor->primary, original);
        const auto dialogSlots =
            visualItems(window->contentItem(), QStringLiteral("customDialogSwatch"));
        QCOMPARE(dialogSlots.size(), 24);
        for (auto* slot : dialogSlots)
            QVERIFY(slot->property("empty").toBool());
        auto* hex = window->findChild<QQuickItem*>(QStringLiteral("hexField"));
        auto* accept = window->findChild<QQuickItem*>(QStringLiteral("acceptColor"));
        QVERIFY(hex && accept);
        hex->forceActiveFocus();
        QTest::keyClick(window, Qt::Key_A, Qt::ControlModifier);
        for (const char key : std::string("12"))
            QTest::keyClick(window, key);
        QVERIFY(!accept->isEnabled());
        for (const char key : std::string("AB34"))
            QTest::keyClick(window, key);
        QVERIFY(accept->isEnabled());
        QCOMPARE(selection->color(), QColor("#12AB34"));
        selection->setHsv(210, 80, 90);
        QCOMPARE(editor->primary, original);
        QVERIFY(capture(QStringLiteral("color-editor-light.png")));
        auto* dark = window->findChild<QObject*>(QStringLiteral("darkThemeAction"));
        QVERIFY(QMetaObject::invokeMethod(dark, "trigger", Q_ARG(QObject*, nullptr)));
        window->resize(1024, 640);
        QVERIFY(capture(QStringLiteral("color-editor-dark-1024.png")));
        // Popup must fit even at the supported minimum size.
        QVERIFY(dialog->property("height").toReal() <= window->height());
        const auto center =
            spectrum->mapToScene({spectrum->width() / 2, spectrum->height() / 2}).toPoint();
        QTest::mouseClick(window, Qt::LeftButton, {}, center);
        QVERIFY(std::abs(selection->hue() - 180) < 2);
        QVERIFY(std::abs(selection->saturation() - 50) < 2);
        QCOMPARE(selection->value(), 90.0);
        auto* brightness = window->findChild<QQuickItem*>(QStringLiteral("brightnessSlider"));
        QVERIFY(brightness);
        brightness->forceActiveFocus();
        QTest::keyClick(window, Qt::Key_End);
        QCOMPARE(selection->value(), 100.0);
        QTest::keyClick(window, Qt::Key_Home);
        QCOMPARE(selection->color(), QColor(Qt::black));
        QTest::keyClick(window, Qt::Key_End);
        const auto beforeMode = selection->color();
        auto* mode = window->findChild<QQuickItem*>(QStringLiteral("colorMode"));
        QVERIFY(mode);
        click(mode);
        auto* modePopup = qvariant_cast<QObject*>(mode->property("popup"));
        QVERIFY(modePopup);
        QTRY_VERIFY(modePopup->property("visible").toBool());
        QVERIFY(capture(QStringLiteral("color-model-menu-dark.png")));
        QTest::keyClick(window, Qt::Key_Down);
        QTest::keyClick(window, Qt::Key_Return);
        QTRY_VERIFY(!modePopup->property("visible").toBool());
        QCOMPARE(mode->property("currentIndex").toInt(), 1);
        QCOMPARE(selection->color(), beforeMode);
        const auto channels = visualItems(window->contentItem(), QStringLiteral("channelInput0"));
        QCOMPARE(channels.size(), 1);
        auto* numericText = qvariant_cast<QQuickItem*>(channels.first()->property("contentItem"));
        QVERIFY(numericText);
        numericText->forceActiveFocus();
        QTest::keyClick(window, Qt::Key_A, Qt::ControlModifier);
        for (const char key : std::string("120"))
            QTest::keyClick(window, key);
        QTest::keyClick(window, Qt::Key_Return);
        QCOMPARE(selection->hue(), 120.0);
        const auto basic = visualItems(window->contentItem(), QStringLiteral("basicDialogSwatch"));
        QCOMPARE(basic.size(), 48);
        click(basic.last());
        QCOMPARE(selection->color(), QColor(Qt::white));
        click(dialogSlots.last());
        QCOMPARE(palette->colors(), QStringList{"#ffffff"});
        QTest::keyClick(window, Qt::Key_Escape);
        QTRY_VERIFY(!dialog->property("visible").toBool());
        QCOMPARE(editor->primary, original);
        click(secondary);
        QVERIFY(editor->secondaryActive());
        click(window->findChild<QQuickItem*>(QStringLiteral("editColorsButton")));
        QTRY_VERIFY(dialog->property("visible").toBool());
        QVERIFY(capture(QStringLiteral("color-secondary-dark-1024.png")));
        QVERIFY(selection->setHex("#12AB34"));
        auto* add = window->findChild<QQuickItem*>(QStringLiteral("addCustomColor"));
        QVERIFY(add);
        click(add);
        QCOMPARE(palette->colors().last(), QString("#12ab34"));
        auto* ok = window->findChild<QQuickItem*>(QStringLiteral("acceptColor"));
        QVERIFY(ok);
        QSignalSpy chosen(dialog, SIGNAL(colorChosen(QColor)));
        click(ok);
        QTRY_VERIFY(!dialog->property("visible").toBool());
        QCOMPARE(chosen.size(), 1);
        QCOMPARE(editor->primary, QColor("#12AB34"));
        QCOMPARE(editor->secondary, original);
        QCOMPARE(editor->image(), pixels);
        QVERIFY(!editor->dirty());
        for (int i = 2; i < 24; ++i)
            QVERIFY(palette->add(QColor::fromHsv(i * 15, 180, 220)));
        QTRY_COMPARE(
            visualItems(window->contentItem(), QStringLiteral("customToolbarSwatch")).size(), 10);
        QVERIFY(capture(QStringLiteral("custom-palette-dark-1024.png")));
        for (auto* swatch :
             visualItems(window->contentItem(), QStringLiteral("customToolbarSwatch"))) {
            const auto bounds = swatch->mapRectToScene(swatch->boundingRect());
            QVERIFY(QRectF(0, 0, window->width(), window->height()).contains(bounds));
        }
        click(primary);
        click(window->findChild<QQuickItem*>(QStringLiteral("editColorsButton")));
        QTRY_VERIFY(dialog->property("visible").toBool());
        QVERIFY(capture(QStringLiteral("color-editor-full-dark-1024.png")));
        for (auto* slot : dialogSlots)
            QVERIFY(!slot->property("empty").toBool());
        QVERIFY(!add->isEnabled());
        QTest::keyClick(window, Qt::Key_Escape);
        QVERIFY(palette->clear());
    }

    void toolbarSlotAndRoleWorkflow() {
        auto* editor = window->findChild<Editor*>(QStringLiteral("editor"));
        auto* dialog = window->findChild<QObject*>(QStringLiteral("colorEditor"));
        auto* draft = window->findChild<ColorSelection*>(QStringLiteral("colorSelection"));
        auto* palette = window->findChild<ColorPalette*>(QStringLiteral("customPalette"));
        const auto before = editor->image();
        QVERIFY(tabTo("secondaryButton"));
        keyClick(Qt::Key_Space);
        QVERIFY(editor->secondaryActive());
        QCOMPARE(editor->primary, QColor(Qt::white));
        QCOMPARE(editor->secondary, QColor(Qt::black));
        QVERIFY(!dialog->property("visible").toBool());
        editor->setTool(Editor::Tool::Text);
        editor->begin({20, 20}, false);
        QCOMPARE(editor->textColor(), QColor(Qt::white));
        QCOMPARE(editor->textBackgroundColor(), QColor(Qt::black));
        editor->cancel();
        editor->setTool(Editor::Tool::Pencil);
        const auto toolbarSlots = visualItems(window->contentItem(), "customToolbarSwatch");
        QVERIFY(tabToItem(toolbarSlots.at(7)));
        keyClick(Qt::Key_Space);
        QTRY_VERIFY(dialog->property("visible").toBool());
        QVERIFY(draft->setHex("#123456"));
        keyClick(Qt::Key_Escape);
        QCOMPARE(palette->toolbarColors().at(7), QString());
        QVERIFY(tabToItem(toolbarSlots.at(7)));
        keyClick(Qt::Key_Space);
        QTRY_VERIFY(dialog->property("visible").toBool());
        QVERIFY(draft->setHex("#123456"));
        QVERIFY(tabTo("acceptColor"));
        keyClick(Qt::Key_Space);
        QTRY_VERIFY(!dialog->property("visible").toBool());
        QCOMPARE(palette->toolbarColors().at(7), QString("#123456"));
        QCOMPARE(palette->toolbarColors().at(6), QString());
        QCOMPARE(editor->primary, QColor("#123456"));
        QVERIFY(tabToItem(toolbarSlots.at(7)));
        keyClick(Qt::Key_F10, Qt::ShiftModifier);
        keyClick(Qt::Key_Down);
        keyClick(Qt::Key_Return);
        QTRY_VERIFY(dialog->property("visible").toBool());
        QVERIFY(draft->setHex("#654321"));
        QVERIFY(tabTo("acceptColor"));
        keyClick(Qt::Key_Space);
        QTRY_VERIFY(!dialog->property("visible").toBool());
        QCOMPARE(editor->primary, QColor("#654321"));
        QCOMPARE(editor->image(), before);
        QVERIFY(!editor->dirty());
    }

    void renderTiming() {
        if (!qEnvironmentVariableIsSet("OPENPAINT_BENCHMARK"))
            return;
        auto* canvas = window->findChild<CanvasItem*>(QStringLiteral("canvas"));
        struct BenchmarkEditor : Editor {
            void resetSize(int size, bool noise = false) {
                QImage image(size, size, QImage::Format_ARGB32_Premultiplied);
                image.fill(Qt::white);
                image.setColorSpace(QColorSpace::SRgb);
                quint32 seed = 42;
                if (noise)
                    for (int y = 0; y < size; ++y) {
                        auto* row = reinterpret_cast<QRgb*>(image.scanLine(y));
                        for (int x = 0; x < size; ++x) {
                            seed = seed * 1664525U + 1013904223U;
                            row[x] = 0xff000000U | (seed & 0xffffffU);
                        }
                    }
                document.reset(image);
            }
            qsizetype historyBytes() const {
                return document.historyBytes();
            }
        } benchmark;
        benchmark.resetSize(2048);
        auto* previous = canvas->editor();
        canvas->setEditor(&benchmark);
        const auto restore = qScopeGuard([&] { canvas->setEditor(previous); });
        auto report = [](const char* name, QList<double> values) {
            std::sort(values.begin(), values.end());
            const auto p95 = qsizetype(std::ceil(values.size() * 0.95)) - 1;
            qInfo("M1 %s p50_ms %.3f p95_ms %.3f max_ms %.3f samples %lld", name,
                  values[values.size() / 2], values[p95], values.last(), qlonglong(values.size()));
        };
        auto measure = [&](const std::function<void(int)>& operation, QList<double>& samples,
                           QList<double>& intervals) {
            QElapsedTimer clock;
            clock.start();
            double lastFrame = 0;
            for (int i = 0; i < 65; ++i) {
                QSignalSpy frame(window, &QQuickWindow::frameSwapped);
                QElapsedTimer timer;
                timer.start();
                operation(i);
                window->requestUpdate();
                if (!frame.wait(1000))
                    return false;
                const double now = clock.nsecsElapsed() / 1000000.0;
                if (i >= 5) {
                    samples.append(timer.nsecsElapsed() / 1000000.0);
                    intervals.append(now - lastFrame);
                }
                lastFrame = now;
            }
            return true;
        };
        QList<double> samples, intervals;
        benchmark.begin({100, 100}, false);
        QVERIFY(measure([&](int i) { benchmark.extend({100.0 + i * 20, 100.0 + i * 10}); }, samples,
                        intervals));
        benchmark.finish();
        report("stroke_update_to_frame", samples);
        report("stroke_frame_interval", intervals);
        benchmark.resetSize(4096, true);
        canvas->fit();
        samples.clear();
        intervals.clear();
        QVERIFY(
            measure([&](int i) { canvas->zoomBy(i % 2 ? 1.01 : 1.0 / 1.01); }, samples, intervals));
        report("viewport_update_to_frame", samples);
        report("viewport_frame_interval", intervals);
        // Four large commands exercise trimming at the default 256 MiB budget.
        qsizetype peakHistory = 0;
        for (int i = 0; i < 4; ++i) {
            benchmark.primary = i % 2 ? Qt::red : Qt::blue;
            benchmark.begin({0, 0}, false);
            benchmark.extend({4095, 4095});
            benchmark.finish();
            QVERIFY(benchmark.historyBytes() <= 256 * 1024 * 1024);
            peakHistory = std::max(peakHistory, benchmark.historyBytes());
        }
        QTemporaryDir dir;
        const auto path = dir.filePath("benchmark.png");
        const auto snapshot = benchmark.image();
        QList<double> heartbeats;
        QElapsedTimer elapsed;
        elapsed.start();
        double last = 0;
        QTimer heartbeat;
        heartbeat.setInterval(5);
        connect(&heartbeat, &QTimer::timeout, window, [&] {
            const auto now = elapsed.nsecsElapsed() / 1000000.0;
            heartbeats.append(now - last);
            last = now;
        });
        heartbeat.start();
        benchmark.saveFile(QUrl::fromLocalFile(path));
        QVERIFY(benchmark.busy());
        benchmark.begin({200, 300}, false);
        benchmark.finish();
        QTRY_VERIFY_WITH_TIMEOUT(!benchmark.busy(), 30000);
        heartbeat.stop();
        QVERIFY(benchmark.dirty());
        QVERIFY(heartbeats.size() >= 30);
        report("save_gui_heartbeat", heartbeats);
        qInfo("M1 save_elapsed_ms %.3f", elapsed.nsecsElapsed() / 1000000.0);
        const auto loaded = openpaint::loadPng(path);
        QVERIFY2(loaded.ok(), qPrintable(loaded.error));
        QCOMPARE(loaded.image, snapshot);
        rusage usage{};
        QVERIFY(getrusage(RUSAGE_SELF, &usage) == 0);
        qInfo("M1 peak_rss_mib %.3f peak_history_mib %.3f", usage.ru_maxrss / 1024.0,
              peakHistory / (1024.0 * 1024.0));
    }

    void quitClosesWindow() {
        auto* quit = window->findChild<QObject*>(QStringLiteral("quitAction"));
        QVERIFY(quit);
        QVERIFY(QMetaObject::invokeMethod(quit, "trigger", Q_ARG(QObject*, nullptr)));
        QTRY_VERIFY(!window->isVisible());
    }

  private:
    QWindow* keyWindow() {
        QTest::qWait(10);
        QWindow* target = window;
        for (auto* candidate : QGuiApplication::topLevelWindows()) {
            if (candidate != window && candidate->isVisible() &&
                candidate->transientParent() == window)
                target = candidate;
        }
        // Xvfb has no window manager; offscreen can retain a hidden popup as focus.
        // Activate the visible test dialog, never another application's window.
        if (QGuiApplication::focusWindow() != target) {
            target->requestActivate();
            if (!QTest::qWaitFor([&] { return QGuiApplication::focusWindow() == target; }))
                QTest::qFail("Test window did not activate", __FILE__, __LINE__);
        }
        return target;
    }
    void keyClick(Qt::Key key, Qt::KeyboardModifiers modifiers = {}) {
        QTest::keyClick(keyWindow(), key, modifiers);
    }
    void keyClick(char key) {
        QTest::keyClick(keyWindow(), key);
    }
    bool tabUntil(const std::function<bool(QQuickItem*)>& matches) {
        for (int i = 0; i < 100; ++i) {
            auto* target = qobject_cast<QQuickWindow*>(keyWindow());
            for (auto* item = target ? target->activeFocusItem() : nullptr; item;
                 item = item->parentItem()) {
                if (matches(item))
                    return true;
            }
            keyClick(Qt::Key_Tab);
        }
        return false;
    }
    bool tabToItem(QQuickItem* item) {
        return tabUntil([&](QQuickItem* candidate) { return candidate == item; });
    }
    bool tabToButton(const QString& text) {
        return tabUntil([&](QQuickItem* candidate) {
            return candidate->property("text").toString().remove('&') == text;
        });
    }
    bool tabTo(const QString& name) {
        return tabUntil([&](QQuickItem* candidate) { return candidate->objectName() == name; });
    }
    // Optional synthetic evidence: capture only this test window, never the desktop.
    bool capture(const QString& name) {
        const auto directory = qEnvironmentVariable("OPENPAINT_CAPTURE_DIR");
        if (!directory.isEmpty() && !QDir().mkpath(directory))
            return false;
        // Synchronize layout/rendering even when no capture is requested. Waiting
        // for an extra frame can time out when the requested state already rendered.
        QGuiApplication::sync();
        const auto frame = window->grabWindow();
        return !frame.isNull() &&
               (directory.isEmpty() || frame.save(QDir(directory).filePath(name)));
    }

    std::unique_ptr<QQmlApplicationEngine> engine;
    QQuickWindow* window = nullptr;
    QStringList warnings;
};

int main(int argc, char* argv[]) {
    startupTimer.start();
    QTemporaryDir settings;
    if (!settings.isValid())
        return 1;
    qputenv("XDG_CONFIG_HOME", settings.path().toUtf8());
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle(QStringLiteral("Basic"));
    ShellTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "tst_shell.moc"
