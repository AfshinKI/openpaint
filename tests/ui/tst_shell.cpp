// SPDX-License-Identifier: GPL-3.0-or-later
#include "color/ColorPalette.h"
#include "color/ColorSpectrum.h"
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
        QCOMPARE(editor->secondary, QColor("#12AB34"));
        QCOMPARE(editor->primary, original);
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
        QTRY_VERIFY(dialog->property("visible").toBool());
        QVERIFY(capture(QStringLiteral("color-editor-full-dark-1024.png")));
        for (auto* slot : dialogSlots)
            QVERIFY(!slot->property("empty").toBool());
        QVERIFY(!add->isEnabled());
        QTest::keyClick(window, Qt::Key_Escape);
        QVERIFY(palette->clear());
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
