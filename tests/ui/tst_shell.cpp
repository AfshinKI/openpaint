// SPDX-License-Identifier: GPL-3.0-or-later
#include "color/ColorPalette.h"
#include "color/ColorSpectrum.h"
#include "io/ImageIo.h"
#include "io/OraIo.h"
#include "platform/AppInfo.h"
#include "platform/Appearance.h"
#include "platform/BlurSupport.h"
#include "platform/WindowStartup.h"
#include "render/CanvasItem.h"
#include <QAccessible>
#include <QClipboard>
#include <QColorSpace>
#include <QDesktopServices>
#include <QDir>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QInputMethodEvent>
#include <QMimeData>
#include <QPainter>
#include <QPainterPath>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlError>
#include <QQmlExpression>
#include <QQmlProperty>
#include <QQuickStyle>
#include <QQuickTextDocument>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QScopeGuard>
#include <QSemaphore>
#include <QSettings>
#include <QSignalSpy>
#include <QStyleHints>
#include <QTemporaryDir>
#include <QTest>
#include <QTextCursor>
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
        // Shell tests never contact the network; updater policy has fake-transport tests.
        QSettings settings(QSettings::IniFormat, QSettings::UserScope, "OpenPaint", "OpenPaint");
        settings.setValue("updates/automatic", false);
        settings.setValue("onboarding/welcomeRevision", 1);
        settings.sync();
        engine = std::make_unique<QQmlApplicationEngine>();
        connect(engine.get(), &QQmlApplicationEngine::warnings, this,
                [this](const QList<QQmlError>& errors) {
                    for (const auto& error : errors)
                        warnings.append(error.toString());
                });
        // Rendering tests use deterministic capability evidence, never host tools.
        auto* blur = engine->singletonInstance<BlurSupport*>("OpenPaint", "BlurSupport");
        QTRY_VERIFY(!blur->busy());
        blur->setProbe([] { return BlurSupport::Result{true, {}, {}}; });
        engine->loadFromModule("OpenPaint", "Main");
        QCOMPARE(engine->rootObjects().size(), 1);
        window = qobject_cast<QQuickWindow*>(engine->rootObjects().first());
        QVERIFY(window);
        auto* updates = window->findChild<QObject*>("updateService");
        QVERIFY(updates);
        updates->setProperty("automatic", false);
        revealAfterFirstFrame(window);
        QCOMPARE(window->opacity(), 0.0);
        QVERIFY(QTest::qWaitForWindowExposed(window));
        QTRY_COMPARE(window->opacity(), 1.0);
        window->requestActivate();
        QVERIFY(QTest::qWaitForWindowActive(window));
        QGuiApplication::sync();
        QVERIFY(!window->grabWindow().isNull());
        QTRY_VERIFY(blur->ready());
        if (qEnvironmentVariableIsSet("OPENPAINT_BENCHMARK_STARTUP"))
            qInfo("M1 startup_ms %.3f", startupTimer.nsecsElapsed() / 1000000.0);
    }

    void cleanup() {
        engine.reset();
        window = nullptr;
        QVERIFY2(warnings.isEmpty(), qPrintable(warnings.join('\n')));
    }

    void zoomSliderWorkflow() {
        auto* editor = window->findChild<Editor*>("editor");
        auto* canvas = window->findChild<CanvasItem*>("canvas");
        auto* slider = window->findChild<QQuickItem*>("zoomSlider");
        QVERIFY(slider);
        const auto original = editor->image();
        const bool dirty = editor->dirty();
        window->resize(1024, 640);
        QVERIFY(tabTo("zoomSlider"));
        const auto before = canvas->zoom();
        keyClick(Qt::Key_Right);
        QVERIFY(canvas->zoom() > before);
        canvas->zoomBy(2 / canvas->zoom());
        QTRY_VERIFY(qAbs(slider->property("value").toDouble() - 1) < 0.001);
        QTest::mouseClick(
            window, Qt::LeftButton, {},
            slider->mapToScene({slider->width() * 0.75, slider->height() / 2}).toPoint());
        QVERIFY(qAbs(canvas->zoom() - 2) > 0.1);
        canvas->zoomBy(100000);
        QCOMPARE(canvas->zoom(), 32.0);
        QTRY_VERIFY(qAbs(slider->property("value").toDouble() - slider->property("to").toDouble()) <
                    0.001);
        canvas->zoomBy(0.0000001);
        QCOMPARE(canvas->zoom(), 0.01);
        canvas->fit();
        QTRY_VERIFY(qAbs(slider->property("value").toDouble() - std::log2(canvas->zoom())) < 0.001);
        QVERIFY(capture("zoom-slider-1024.png"));
        QVERIFY(slider->mapToScene({slider->width(), 0}).x() < window->width() - 30);
        QCOMPARE(editor->image(), original);
        QCOMPARE(editor->dirty(), dirty);
    }
    void coloredRibbonWorkflow() {
        auto* editor = window->findChild<Editor*>("editor");
        auto* fill = window->findChild<QQuickItem*>("fillButton");
        auto* brush = window->findChild<QQuickItem*>("brushesButton");
        auto* text = window->findChild<QQuickItem*>("textButton");
        auto* invert = window->findChild<QQuickItem*>("invertColorsButton");
        QVERIFY(fill && brush && text && invert);
        const auto paintsColor = [&](QQuickItem* item, QColor color) {
            QGuiApplication::sync();
            const auto frame = window->grabWindow();
            const qreal scale = qreal(frame.width()) / window->width();
            const auto position = item->mapToScene({0, 0});
            int matching = 0;
            for (int y = int(position.y() * scale);
                 y < int((position.y() + item->height()) * scale); ++y)
                for (int x = int(position.x() * scale);
                     x < int((position.x() + item->width()) * scale); ++x)
                    if (frame.pixelColor(x, y).rgb() == color.rgb())
                        ++matching;
            return matching >= 2;
        };
        window->resize(1024, 640);
        auto* appearance = engine->singletonInstance<Appearance*>("OpenPaint", "Appearance");
        appearance->setThemeMode(1);
        editor->setEditingSecondary(false);
        editor->setSelectedColor(QColor("#db5036"));
        QVERIFY(!fill->property("colored").toBool());
        QTRY_VERIFY(paintsColor(brush, QColor("#f0dec3")));
        editor->setTool(Editor::Tool::Fill);
        QTRY_VERIFY(fill->property("checked").toBool());
        QVERIFY(capture("colored-ribbon-light.png"));
        QVERIFY(std::abs(text->mapToScene({0, text->height() / 2}).y() -
                         invert->mapToScene({0, invert->height() / 2}).y()) <= 2);
        editor->setEditingSecondary(true);
        editor->setSelectedColor(QColor("#348bd5"));
        appearance->setThemeMode(2);
        QVERIFY(!fill->property("colored").toBool());
        QTRY_VERIFY(paintsColor(brush, QColor("#f0dec3")));
        QVERIFY(capture("colored-ribbon-dark.png"));
        auto* layers = window->findChild<QQuickItem*>("layersButton");
        QVERIFY(layers->mapToScene({layers->width(), 0}).x() <= window->width() - 4);
        editor->setSelectedColor(Qt::transparent);
        QVERIFY(!fill->property("colored").toBool());
        QTRY_VERIFY(paintsColor(brush, QColor("#f0dec3")));
        QVERIFY(tabTo("textButton"));
        keyClick(Qt::Key_Space);
        QCOMPARE(editor->tool(), Editor::Tool::Text);
        QTRY_VERIFY(!fill->property("checked").toBool());
    }
    void invertColorsWorkflow() {
        auto* editor = window->findChild<Editor*>("editor");
        const auto original = editor->image();
        QVERIFY(tabTo("invertColorsButton"));
        keyClick(Qt::Key_Space);
        QTRY_VERIFY(!editor->busy());
        QCOMPARE(editor->image().pixelColor(0, 0), QColor(Qt::black));
        editor->undo();
        QCOMPARE(editor->image(), original);
        editor->redo();
        QCOMPARE(editor->image().pixelColor(0, 0), QColor(Qt::black));
        window->resize(1024, 640);
        QVERIFY(capture("invert-colors-ribbon.png"));
        auto* button = window->findChild<QQuickItem*>("invertColorsButton");
        QVERIFY(button->mapToScene({button->width(), 0}).x() < window->width());
        auto* layers = window->findChild<QQuickItem*>("layersButton");
        QVERIFY(layers->mapToScene({layers->width(), 0}).x() <= window->width() - 4);
    }
    void layersWorkflow() {
        auto* editor = window->findChild<Editor*>("editor");
        auto* panel = window->findChild<QQuickItem*>("layersPanel");
        auto* canvas = window->findChild<CanvasItem*>("canvas");
        QVERIFY(panel);
        QVERIFY(!panel->isVisible());
        QVERIFY(tabTo("layersButton"));
        keyClick(Qt::Key_Space);
        QTRY_VERIFY(panel->isVisible());
        QCOMPARE(editor->layers().size(), 2);
        QCOMPARE(editor->activeLayer(), 1);
        auto* addLayer = window->findChild<QQuickItem*>("addLayerButton");
        QVERIFY(addLayer);
        auto* layersButton = window->findChild<QQuickItem*>("layersButton");
        QVERIFY(layersButton->width() >= 48);
        QVERIFY(layersButton->height() >= 56);
        QVERIFY(window->findChild<QQuickItem*>("layersRibbonDivider"));
        keyClick(Qt::Key_Space);
        QTRY_VERIFY(!panel->isVisible());
        QVERIFY(tabTo("layersButton"));
        keyClick(Qt::Key_Space);
        QTRY_VERIFY(panel->isVisible());
        QCOMPARE(editor->layers().size(), 2);
        QTest::mouseClick(window, Qt::LeftButton, {}, addLayer->mapToScene({16, 16}).toPoint());
        QCOMPARE(editor->layers().size(), 3);
        editor->undo();
        QCOMPARE(editor->layers().size(), 2);
        editor->renameLayer("Foreground");
        editor->primary = QColor("#e87945");
        editor->brushSize = 100;
        editor->begin({200, 200}, false);
        editor->extend({650, 350});
        editor->finish();
        editor->setLayerOpacity(65);
        const auto tile = [&](const QString& name) -> QQuickItem* {
            const auto matches = visualItems(window->contentItem(), name);
            return matches.isEmpty() ? nullptr : matches.first();
        };
        canvas->forceActiveFocus();
        keyClick(Qt::Key_N, Qt::ControlModifier | Qt::ShiftModifier);
        QCOMPARE(editor->layers().size(), 3);
        editor->renameLayer("Top");
        QVERIFY(capture("layers-before-drag.png"));
        auto* upper = tile("layerTile2");
        auto* lower = tile("layerTile1");
        QVERIFY(upper);
        QVERIFY(lower);
        const auto start = upper->mapToScene({upper->width() / 2, upper->height() / 2}).toPoint();
        const auto end = lower->mapToScene({lower->width() / 2, lower->height() / 2}).toPoint();
        QTest::mousePress(window, Qt::LeftButton, {}, start);
        QTest::mouseMove(window, start + QPoint(0, 20), 20);
        QTest::mouseMove(window, end, 20);
        QTest::mouseRelease(window, Qt::LeftButton, {}, end);
        QCOMPARE(editor->layers()[1].toMap()["name"].toString(), QString("Top"));
        QCOMPARE(editor->activeLayer(), 1);
        editor->undo();
        QCOMPARE(editor->layers()[0].toMap()["name"].toString(), QString("Top"));
        editor->redo();
        QCOMPARE(editor->layers()[1].toMap()["name"].toString(), QString("Top"));
        // Escape cancels a held drag without changing stack order.
        upper = tile("layerTile2");
        QTest::mousePress(window, Qt::LeftButton, {}, start);
        QTest::mouseMove(window, start + QPoint(0, 20), 20);
        keyClick(Qt::Key_Escape);
        QTest::mouseMove(window, end, 20);
        QTest::mouseRelease(window, Qt::LeftButton, {}, end);
        QCOMPARE(editor->layers()[0].toMap()["name"].toString(), QString("Foreground"));
        auto* background = tile("backgroundLayerTile");
        QVERIFY(background);
        auto* eye = tile("layerEye0");
        QVERIFY(eye);
        QVERIFY(eye->x() > background->width() / 2);
        canvas->forceActiveFocus();
        QTest::mouseMove(window, QPoint(100, 200));
        QTRY_VERIFY(!eye->isVisible());
        QTest::mouseMove(window, background->mapToScene({80, 40}).toPoint());
        QTRY_VERIFY(eye->isVisible());
        const int active = editor->activeLayer();
        QTest::mouseClick(window, Qt::LeftButton, {}, eye->mapToScene({16, 16}).toPoint());
        QCOMPARE(editor->activeLayer(), active);
        QVERIFY(!editor->layers().last().toMap()["visible"].toBool());
        QCOMPARE(eye->property("symbol").toString(), QString("layer-eye-hidden"));
        QTest::mouseClick(window, Qt::LeftButton, {}, eye->mapToScene({16, 16}).toPoint());
        QVERIFY(editor->layers().last().toMap()["visible"].toBool());
        // The background never participates in dragging.
        const auto base = background->mapToScene({80, 50}).toPoint();
        QTest::mousePress(window, Qt::LeftButton, {}, base);
        QTest::mouseMove(window, start, 20);
        QTest::mouseRelease(window, Qt::LeftButton, {}, start);
        QCOMPARE(editor->layers().last().toMap()["name"].toString(), QString("Background"));
        upper = tile("layerTile2");
        upper->forceActiveFocus();
        keyClick(Qt::Key_F2);
        auto* dialog = window->findChild<QObject*>("layerPropertiesDialog");
        QVERIFY(dialog);
        QTRY_VERIFY(dialog->property("visible").toBool());
        QVERIFY(tabTo("layerNameField"));
        keyClick(Qt::Key_A, Qt::ControlModifier);
        keyClick(Qt::Key_R);
        keyClick(Qt::Key_E);
        keyClick(Qt::Key_N);
        QVERIFY(QMetaObject::invokeMethod(dialog, "accept"));
        QCOMPARE(editor->layers()[0].toMap()["name"].toString(), QString("ren"));
        editor->undo();
        QCOMPARE(editor->layers()[0].toMap()["name"].toString(), QString("Foreground"));
        // Layer operations remain discoverable from the thumbnail context menu.
        QTest::mouseClick(window, Qt::RightButton, {}, start);
        auto* menu = window->findChild<QObject*>("layerContextMenu");
        QVERIFY(menu);
        QTRY_VERIFY(menu->property("visible").toBool());
        QVERIFY(capture("layers-context-menu.png"));
        keyClick(Qt::Key_Escape);
        auto* appearance = engine->singletonInstance<Appearance*>("OpenPaint", "Appearance");
        appearance->setGlassEnabled(false);
        QTRY_COMPARE(panel->property("surfaceAlpha").toDouble(), 1.0);
        appearance->setSurfaceOpacity(40);
        appearance->setGlassEnabled(true);
        QTRY_COMPARE(panel->property("surfaceAlpha").toDouble(), 0.4);
        if (window->rendererInterface()->graphicsApi() != QSGRendererInterface::Software)
            QTRY_VERIFY(panel->property("backdropBlurEnabled").toBool());
        appearance->setThemeMode(1);
        window->resize(1280, 800);
        QVERIFY(capture("layers-light-1280.png"));
        appearance->setThemeMode(2);
        window->resize(1024, 640);
        QVERIFY(capture("layers-dark-1024.png"));
        QVERIFY(layersButton->mapToScene({layersButton->width(), 0}).x() <= window->width() - 4);
        const auto shielded = editor->image();
        QTest::mouseClick(window, Qt::LeftButton, {}, panel->mapToScene({2, 200}).toPoint());
        QCOMPARE(editor->image(), shielded);
        const auto directory = qEnvironmentVariable("OPENPAINT_CAPTURE_DIR");
        if (!directory.isEmpty()) {
            editor->saveFile(QUrl::fromLocalFile(QDir(directory).filePath("layers-synthetic.ora")));
            QTRY_VERIFY(!editor->busy());
            QVERIFY(!editor->dirty());
        }
    }
    void selectAllKeyboard() {
        auto* editor = window->findChild<Editor*>(QStringLiteral("editor"));
        auto* canvas = window->findChild<QQuickItem*>(QStringLiteral("canvas"));
        canvas->forceActiveFocus();
        keyClick(Qt::Key_A, Qt::ControlModifier);
        QCOMPARE(editor->selectionRect(), editor->image().rect());
        QCOMPARE(editor->tool(), Editor::Tool::Select);
        QVERIFY(!editor->dirty());
        editor->setTool(Editor::Tool::Shape);
        editor->shapeKind = 1;
        editor->begin({20, 20}, false);
        editor->extend({80, 80});
        editor->endGesture();
        keyClick(Qt::Key_A, Qt::ControlModifier);
        QVERIFY(!editor->previewActive());
        QCOMPARE(editor->selectionRect(), editor->image().rect());
    }
    void toolCursorsAndIndicators() {
        auto* canvas = window->findChild<CanvasItem*>("canvas");
        auto* editor = canvas->editor();
        const auto original = editor->image();
        auto* appearance = engine->singletonInstance<Appearance*>("OpenPaint", "Appearance");
        appearance->restoreDefaults();
        QImage cursors(128, 40, QImage::Format_ARGB32_Premultiplied);
        cursors.fill(Qt::white);
        QPainter painter(&cursors);
        int column = 0;
        QImage previous;
        for (auto tool : {Editor::Tool::Pencil, Editor::Tool::ColorPicker, Editor::Tool::Eraser,
                          Editor::Tool::Brush}) {
            editor->setTool(tool);
            QCOMPARE(canvas->cursor().shape(), Qt::BitmapCursor);
            QVERIFY(!canvas->cursor().pixmap().isNull());
            const auto glyph = canvas->cursor().pixmap().toImage();
            QVERIFY(glyph != previous);
            previous = glyph;
            QCOMPARE(canvas->cursor().hotSpot(),
                     tool == Editor::Tool::Eraser ? QPoint(5, 22) : QPoint(6, 22));
            painter.drawPixmap(column++ * 32, 6, canvas->cursor().pixmap());
            canvas->forceActiveFocus();
            QTest::keyPress(window, Qt::Key_Space);
            QCOMPARE(canvas->cursor().shape(), Qt::OpenHandCursor);
            QTest::keyRelease(window, Qt::Key_Space);
            QCOMPARE(canvas->cursor().shape(), Qt::BitmapCursor);
            QCOMPARE(canvas->cursor().pixmap().toImage(), glyph);
            const auto point = canvas->mapToScene(canvas->fromDocument({100, 100})).toPoint();
            QTest::mousePress(window, Qt::MiddleButton, {}, point);
            QCOMPARE(canvas->cursor().shape(), Qt::ClosedHandCursor);
            QTest::mouseRelease(window, Qt::MiddleButton, {}, point);
            QCOMPARE(canvas->cursor().shape(), Qt::BitmapCursor);
        }
        painter.end();
        const auto directory = qEnvironmentVariable("OPENPAINT_CAPTURE_DIR");
        if (!directory.isEmpty()) {
            QVERIFY(QDir().mkpath(directory));
            QVERIFY(cursors.save(directory + "/tool-cursors.png"));
        }
        editor->setTool(Editor::Tool::Shape);
        QCOMPARE(canvas->cursor().shape(), Qt::CrossCursor);
        editor->setProperty("shapeOutline", false);
        QCOMPARE(window->findChild<QObject*>("shapeOutlineButton")->property("symbol").toString(),
                 QString("shape-outline-off"));
        editor->setProperty("shapeFill", true);
        QCOMPARE(window->findChild<QObject*>("shapeFillButton")->property("symbol").toString(),
                 QString("fill-solid"));
        auto* primary = window->findChild<QQuickItem*>("primaryButton");
        auto* secondary = window->findChild<QQuickItem*>("secondaryButton");
        QVERIFY(primary && secondary);
        const auto primaryRings = visualItems(primary, "swatchSelectionRing");
        const auto secondaryRings = visualItems(secondary, "swatchSelectionRing");
        QVERIFY(primaryRings.size() == 1 && secondaryRings.size() == 1);
        QCOMPARE(QQmlProperty::read(primaryRings.first(), "border.color").value<QColor>(),
                 QColor("#F4FAFF"));
        editor->setEditingSecondary(false);
        QVERIFY(primaryRings.first()->isVisible());
        QVERIFY(!secondaryRings.first()->isVisible());
        editor->setEditingSecondary(true);
        QVERIFY(!primaryRings.first()->isVisible());
        QVERIFY(secondaryRings.first()->isVisible());
        editor->setTool(Editor::Tool::Pencil);
        const auto idleShapes = visualItems(window->contentItem(), "ribbonShape0");
        QVERIFY(!idleShapes.isEmpty());
        const auto idleSurfaces = visualItems(idleShapes.first(), "selectedToolSurface");
        QVERIFY(!idleSurfaces.isEmpty());
        QVERIFY(!idleSurfaces.first()->isVisible());
        for (const auto& theme : {QString("light"), QString("dark")}) {
            QVERIFY(QMetaObject::invokeMethod(window->findChild<QObject*>(theme + "ThemeAction"),
                                              "trigger", Q_ARG(QObject*, nullptr)));
            QVERIFY(capture("tool-indicators-" + theme + ".png"));
        }
        QCOMPARE(editor->image(), original);
        QVERIFY(!editor->dirty());
        QVERIFY(!editor->canUndo());
    }

    void eraserModeMenu() {
        auto* pickerPosition = window->findChild<QQuickItem*>("pickerButton");
        auto* eraserPosition = window->findChild<QQuickItem*>("eraserButton");
        QVERIFY(pickerPosition && eraserPosition);
        const auto pickerPoint = pickerPosition->mapToScene(QPointF());
        const auto eraserPoint = eraserPosition->mapToScene(QPointF());
        QCOMPARE(pickerPoint.y(), eraserPoint.y());
        QVERIFY(pickerPoint.x() < eraserPoint.x());
        auto* editor = window->findChild<Editor*>("editor");
        const auto original = editor->image();
        QVERIFY(tabTo("eraserButton"));
        keyClick(Qt::Key_Space);
        QCOMPARE(editor->tool(), Editor::Tool::Eraser);
        keyClick(Qt::Key_Space);
        QVERIFY(window->findChild<QObject*>("eraserButton")->property("checked").toBool());
        const auto toolbarWidth = window->findChild<QQuickItem*>("mainToolbar")->width();
        QVERIFY(tabTo("eraserMenuButton"));
        keyClick(Qt::Key_Space);
        auto* menu = window->findChild<QObject*>("eraserMenu");
        QTRY_VERIFY(menu->property("visible").toBool());
        auto* paper = window->findChild<QObject*>("erasePaperItem");
        auto* transparent = window->findChild<QObject*>("eraseTransparentItem");
        QVERIFY(paper && transparent);
        QCOMPARE(paper->property("symbol").toString(), QString("eraser-paper"));
        QCOMPARE(transparent->property("symbol").toString(), QString("eraser"));
        auto* eraser = window->findChild<QObject*>("eraserButton");
        QCOMPARE(eraser->property("symbol").toString(), QString("eraser-paper"));
        const auto paperCursor =
            window->findChild<CanvasItem*>("canvas")->cursor().pixmap().toImage();
        QVERIFY(paper->property("checked").toBool());
        keyClick(Qt::Key_Down);
        keyClick(Qt::Key_Down);
        keyClick(Qt::Key_Return);
        QTRY_VERIFY(editor->transparentEraser);
        QCOMPARE(eraser->property("symbol").toString(), QString("eraser"));
        QVERIFY(window->findChild<CanvasItem*>("canvas")->cursor().pixmap().toImage() !=
                paperCursor);
        QVERIFY(transparent->property("checked").toBool());
        QVERIFY(!paper->property("checked").toBool());
        QVERIFY(QMetaObject::invokeMethod(menu, "open"));
        QTRY_VERIFY(menu->property("visible").toBool());
        QVERIFY(capture("eraser-menu.png"));
        QVERIFY(QMetaObject::invokeMethod(paper, "click"));
        QVERIFY(!editor->transparentEraser);
        QCOMPARE(window->findChild<QQuickItem*>("mainToolbar")->width(), toolbarWidth);
        QCOMPARE(editor->image(), original);
        QVERIFY(!editor->dirty());
        QVERIFY(!editor->canUndo());
    }

    void themedApplicationIcon() {
        auto* appearance = engine->singletonInstance<Appearance*>("OpenPaint", "Appearance");
        const auto image = window->findChild<Editor*>("editor")->image();
        appearance->setThemeMode(1);
        QVERIFY(appearance->applicationIcon().path().endsWith("/app.svg"));
        const auto light = QGuiApplication::windowIcon().pixmap(64, 64).toImage();
        QVERIFY(!light.isNull());
        appearance->setThemeMode(2);
        QVERIFY(appearance->applicationIcon().path().endsWith("/app-dark.svg"));
        const auto dark = QGuiApplication::windowIcon().pixmap(64, 64).toImage();
        QVERIFY(!dark.isNull());
        QVERIFY(dark != light);
        QCOMPARE(window->icon().pixmap(64, 64).toImage(), dark);
        const auto originalScheme = QGuiApplication::styleHints()->colorScheme();
        const auto notifyScheme = [](Qt::ColorScheme scheme) {
            return QMetaObject::invokeMethod(QGuiApplication::styleHints(), "colorSchemeChanged",
                                             Q_ARG(Qt::ColorScheme, scheme));
        };
        const auto resetScheme = qScopeGuard([&] { notifyScheme(originalScheme); });
        appearance->setThemeMode(0);
        // Inject platform notifications: the offscreen plugin cannot change an OS theme.
        QVERIFY(notifyScheme(Qt::ColorScheme::Dark));
        QTRY_VERIFY(appearance->applicationIcon().path().endsWith("/app-dark.svg"));
        QCOMPARE(QGuiApplication::windowIcon().pixmap(64, 64).toImage(), dark);
        QVERIFY(notifyScheme(Qt::ColorScheme::Light));
        QTRY_VERIFY(appearance->applicationIcon().path().endsWith("/app.svg"));
        QCOMPARE(QGuiApplication::windowIcon().pixmap(64, 64).toImage(), light);
        QCOMPARE(window->findChild<Editor*>("editor")->image(), image);
    }

    void splineKeyboardWorkflow() {
        auto* editor = window->findChild<Editor*>("editor");
        auto* canvas = window->findChild<CanvasItem*>("canvas");
        const auto original = editor->image();
        QVERIFY(tabTo("ribbonShape19"));
        keyClick(Qt::Key_Space);
        QCOMPARE(editor->shapeKind, 19);
        QCOMPARE(editor->tool(), Editor::Tool::Shape);
        const auto at = [&](QPointF p) {
            return canvas->mapToScene(canvas->fromDocument(p)).toPoint();
        };
        QTest::mousePress(window, Qt::LeftButton, {}, at({80, 100}));
        QTest::mouseMove(window, at({180, 40}));
        QTest::mouseRelease(window, Qt::LeftButton, {}, at({180, 40}));
        QTest::mouseClick(window, Qt::LeftButton, {}, at({280, 160}));
        const auto threePoints = editor->image();
        QTest::mouseClick(window, Qt::LeftButton, {}, at({380, 70}));
        QVERIFY(editor->image() != threePoints);
        keyClick(Qt::Key_Backspace);
        QCOMPARE(editor->image(), threePoints);
        QVERIFY(capture("spline-preview.png"));
        keyClick(Qt::Key_Return);
        QVERIFY(!editor->previewActive());
        QCOMPARE(editor->image(), threePoints);
        keyClick(Qt::Key_Z, Qt::ControlModifier);
        QCOMPARE(editor->image(), original);
        keyClick(Qt::Key_Y, Qt::ControlModifier);
        QCOMPARE(editor->image(), threePoints);
        QTest::mousePress(window, Qt::LeftButton, {}, at({80, 200}));
        QTest::mouseMove(window, at({180, 240}));
        QTest::mouseRelease(window, Qt::LeftButton, {}, at({180, 240}));
        keyClick(Qt::Key_Escape);
        QCOMPARE(editor->image(), threePoints);
    }

    void roundedClientCorners() {
        auto* editor = window->findChild<Editor*>("editor");
        const auto image = editor->image();
        window->resize(1024, 640);
#ifdef Q_OS_MACOS
        // Cocoa owns the native frame/corners; do not apply the Linux client mask.
        QCOMPARE(window->property("clientRadius").toDouble(), 0.0);
        QVERIFY(window->mask().isEmpty());
        QVERIFY(!window->grabWindow().isNull());
        QCOMPARE(editor->image(), image);
        QVERIFY(!editor->dirty());
        return;
#endif
        QVERIFY(window->mask().contains({0, 0}));
        QTRY_VERIFY(!window->mask().contains({0, window->height() - 1}));
        QVERIFY(window->mask().contains({100, 100}));
        for (const auto& theme : {QString("light"), QString("dark")}) {
            QVERIFY(QMetaObject::invokeMethod(window->findChild<QObject*>(theme + "ThemeAction"),
                                              "trigger", Q_ARG(QObject*, nullptr)));
            QVERIFY(capture("rounded-window-" + theme + ".png"));
            const auto frame = window->grabWindow();
            QVERIFY(frame.pixelColor(0, 0).alpha() > 0);
            QVERIFY(frame.pixelColor(frame.width() - 1, 0).alpha() > 0);
            QCOMPARE(frame.pixelColor(0, frame.height() - 1).alpha(), 0);
            QCOMPARE(frame.pixelColor(frame.width() - 1, frame.height() - 1).alpha(), 0);
        }
        window->showMaximized();
        QTRY_COMPARE(window->windowState(), Qt::WindowMaximized);
        QVERIFY(window->mask().isEmpty());
        QCOMPARE(window->property("clientRadius").toDouble(), 0.0);
        window->showNormal();
        QTRY_COMPARE(window->windowState(), Qt::WindowNoState);
        QVERIFY(window->mask().contains({0, 0}));
        QTRY_VERIFY(!window->mask().contains({0, window->height() - 1}));
        QCOMPARE(editor->image(), image);
        QVERIFY(!editor->dirty());
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

    void floatingToolControls() {
        auto* editor = window->findChild<Editor*>(QStringLiteral("editor"));
        auto* canvas = window->findChild<QQuickItem*>(QStringLiteral("canvas"));
        const auto original = editor->image();
        const auto canvasRect = canvas->mapRectToScene(canvas->boundingRect());
        QVERIFY(tabTo("strokeSize"));
        keyClick(Qt::Key_Up);
        QCOMPARE(editor->brushSize, 4);
        editor->setTool(Editor::Tool::Brush);
        QVERIFY(tabTo("brushOpacity"));
        keyClick(Qt::Key_Down);
        QCOMPARE(editor->opacity, 99);
        QCOMPARE(canvas->mapRectToScene(canvas->boundingRect()), canvasRect);
        auto* rail = window->findChild<QQuickItem*>(QStringLiteral("floatingToolControls"));
        QVERIFY(rail);
        const auto point = rail->mapToScene({3, 3}).toPoint();
        QTest::mouseClick(window, Qt::LeftButton, {}, point);
        QCOMPARE(editor->image(), original);
        QVERIFY(!editor->dirty());
        window->resize(1024, 640);
        QVERIFY(capture(QStringLiteral("floating-controls-1024.png")));
        QTRY_VERIFY(QRectF(0, 0, window->width(), window->height())
                        .contains(rail->mapRectToScene(rail->boundingRect())));
        editor->setTool(Editor::Tool::Fill);
        auto* size = window->findChild<QQuickItem*>(QStringLiteral("strokeSize"));
        QVERIFY(size && !size->isVisible());
        QVERIFY(!window->findChild<QObject*>("fillTolerance"));
        QCOMPARE(editor->brushSize, 4);
        QCOMPARE(editor->opacity, 99);
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
        auto* workspace = window->findChild<QObject*>(QStringLiteral("workspaceSurface"));
        QVERIFY(light && dark && system && workspace);
        QVERIFY(QMetaObject::invokeMethod(dark, "trigger", Q_ARG(QObject*, nullptr)));
        QTRY_COMPARE(workspace->property("color").value<QColor>().name(),
                     QStringLiteral("#111418"));
        QVERIFY(dark->property("checked").toBool());
        QVERIFY(!light->property("checked").toBool());
        QVERIFY(capture(QStringLiteral("shell-dark-1280.png")));
        auto* menu = window->findChild<QObject*>(QStringLiteral("fileMenu"));
        QVERIFY(menu);
        QVERIFY(QMetaObject::invokeMethod(menu, "open"));
        QTRY_VERIFY(menu->property("visible").toBool());
        QVERIFY(menu->property("width").toReal() >= 240);
        QVERIFY(capture(QStringLiteral("menu-dark.png")));
        const auto hints = visualItems(window->contentItem(), "menuShortcutHint");
        QVERIFY(!hints.isEmpty());
        for (const auto* hint : hints)
            QCOMPARE(hint->property("color").value<QColor>(), QColor("#ADB8C5"));
        QVERIFY(QMetaObject::invokeMethod(menu, "close"));
        window->resize(1024, 640);
        QVERIFY(capture(QStringLiteral("shell-dark-1024.png")));
        QVERIFY(QMetaObject::invokeMethod(light, "trigger", Q_ARG(QObject*, nullptr)));
        QTRY_COMPARE(workspace->property("color").value<QColor>().name(),
                     QStringLiteral("#e2e5ea"));
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

    void interruptedStrokeAndRightPan() {
        auto* canvas = window->findChild<CanvasItem*>(QStringLiteral("canvas"));
        auto* editor = canvas->editor();
        editor->primary = Qt::red;
        canvas->zoomAt(1.5, {200, 200});
        auto at = [&](QPointF p) { return canvas->mapToScene(canvas->fromDocument(p)).toPoint(); };
        QTest::mousePress(window, Qt::LeftButton, {}, at({100, 100}));
        QTest::mouseRelease(window, Qt::LeftButton, {}, at({200, 100}));
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
        const auto original = editor->image();
        const auto origin = canvas->fromDocument({0, 0});
        const QPoint start = canvas->mapToScene({200, 200}).toPoint();
        QTest::mousePress(window, Qt::RightButton, {}, start);
        QCOMPARE(canvas->cursor().shape(), Qt::ClosedHandCursor);
        QTest::mouseMove(window, start + QPoint(40, 20));
        QTest::mouseRelease(window, Qt::RightButton, {}, start + QPoint(60, 50));
        QCOMPARE(canvas->fromDocument({0, 0}), origin + QPointF(60, 50));
        QVERIFY(canvas->cursor().shape() != Qt::ClosedHandCursor);
        QCOMPARE(editor->image(), original);
        QVERIFY(!editor->canUndo());
    }

    void captureReleaseUrl(const QUrl& url) {
        QCOMPARE(url, QUrl("https://github.com/AfshinKI/openpaint/releases"));
    }

    void captureContributingUrl(const QUrl& url) {
        QCOMPARE(url, QUrl("https://github.com/AfshinKI/openpaint/blob/main/CONTRIBUTING.md"));
    }

    void welcomeWorkflow() {
        auto* appearance = engine->singletonInstance<Appearance*>("OpenPaint", "Appearance");
        auto* blur = engine->singletonInstance<BlurSupport*>("OpenPaint", "BlurSupport");
        auto* welcome = window->findChild<QObject*>("welcomeDialog");
        auto* setup = window->findChild<QObject*>("glassSetupDialog");
        auto* editor = window->findChild<Editor*>("editor");
        QSettings settings(QSettings::IniFormat, QSettings::UserScope, "OpenPaint", "OpenPaint");
        settings.remove("onboarding/welcomeRevision");
        settings.sync();
        appearance->restoreDefaults();
        QTRY_VERIFY(!blur->busy());
        blur->setProbe([] { return BlurSupport::Result{}; });
        blur->refresh();
        editor->begin({20, 20}, false);
        editor->extend({70, 40});
        QTest::qWait(900);
        QVERIFY(!welcome->property("visible").toBool());
        editor->finish();
        const auto artwork = editor->image();
        QTRY_VERIFY(welcome->property("visible").toBool());
        QVERIFY(!setup->property("visible").toBool());
        QDesktopServices::setUrlHandler("https", this, "captureContributingUrl");
        const auto resetHandler = qScopeGuard([] { QDesktopServices::unsetUrlHandler("https"); });
        QVERIFY(
            QMetaObject::invokeMethod(window->findChild<QObject*>("welcomeContribute"), "clicked"));
        window->resize(1024, 640);
        for (const auto& theme : {QString("light"), QString("dark")}) {
            QVERIFY(QMetaObject::invokeMethod(window->findChild<QObject*>(theme + "ThemeAction"),
                                              "trigger", Q_ARG(QObject*, nullptr)));
            QVERIFY(capture("welcome-" + theme + "-1024.png"));
        }
        QVERIFY(QMetaObject::invokeMethod(window->findChild<QObject*>("welcomeStart"), "clicked"));
        QTRY_VERIFY(!welcome->property("visible").toBool());
        QVERIFY(!appearance->welcomePending());
        appearance->restoreDefaults();
        QVERIFY(!appearance->welcomePending());
        QCOMPARE(editor->image(), artwork);
        QVERIFY(editor->dirty());
    }

    void glassSetupWorkflow() {
        auto* blur = engine->singletonInstance<BlurSupport*>("OpenPaint", "BlurSupport");
        auto* appearance = engine->singletonInstance<Appearance*>("OpenPaint", "Appearance");
        auto* setup = window->findChild<QObject*>("glassSetupDialog");
        auto* updates = window->findChild<QObject*>("updatesDialog");
        auto* settings = window->findChild<QObject*>("appearanceDialog");
        auto* toggle = window->findChild<QObject*>("glassEnabled");
        auto* enable = window->findChild<QObject*>("enableGlassSetup");
        auto* editor = window->findChild<Editor*>("editor");
        const auto artwork = editor->image();
        appearance->restoreDefaults();
        blur->setProbe([] {
            return BlurSupport::evaluate("ubuntu:GNOME", "x11", "xcb", "ubuntu", "24.04", false, {},
                                         [](const QString& program, const QStringList&) {
                                             if (program == "gnome-shell")
                                                 return QString("GNOME Shell 46.0");
                                             if (program == "apt-cache")
                                                 return QString("Candidate: 0.5.0-1build1");
                                             return QString{};
                                         });
        });
        blur->refresh();
        QTRY_VERIFY(blur->setupAvailable());
        QVERIFY(!setup->property("visible").toBool());
        QVERIFY(QMetaObject::invokeMethod(settings, "open"));
        QTRY_VERIFY(settings->property("visible").toBool());
        QVERIFY(!toggle->property("checked").toBool());
        QVERIFY(!toggle->property("enabled").toBool());
        QVERIFY(appearance->glassEnabled()); // Saved preference is preserved.
        window->resize(1024, 640);
        QVERIFY(capture("glass-settings-disabled-1024.png"));
        QVERIFY(enable->property("visible").toBool());
        QVERIFY(QMetaObject::invokeMethod(enable, "clicked"));
        QTRY_VERIFY(setup->property("visible").toBool());
        auto* copy = window->findChild<QObject*>("copyGlassSetup");
        QVERIFY(QMetaObject::invokeMethod(copy, "clicked"));
        QCOMPARE(QGuiApplication::clipboard()->text(), blur->command());
        for (const auto& theme : {QString("light"), QString("dark")}) {
            QVERIFY(QMetaObject::invokeMethod(window->findChild<QObject*>(theme + "ThemeAction"),
                                              "trigger", Q_ARG(QObject*, nullptr)));
            QVERIFY(capture("enable-glass-" + theme + "-1024.png"));
        }
        QTRY_VERIFY(!blur->busy());
        blur->setProbe([] {
            return BlurSupport::evaluate("GNOME", "x11", "xcb", "ubuntu", "24.04", false, {},
                                         [](const QString& program, const QStringList&) {
                                             if (program == "gnome-shell")
                                                 return QString("GNOME Shell 46.0");
                                             if (program == "dpkg-query")
                                                 return QString("installed");
                                             return QString{};
                                         });
        });
        QVERIFY(QMetaObject::invokeMethod(window->findChild<QObject*>("refreshBlurSupport"),
                                          "clicked"));
        QTRY_VERIFY(!blur->busy());
        QVERIFY(blur->command().startsWith("/usr/bin/python3"));
        QVERIFY(!blur->command().contains("sudo apt install"));
        QVERIFY(window->findChild<QObject*>("blurSetupInstructions")
                    ->property("text")
                    .toString()
                    .contains("No sudo or Extension Manager"));
        QVERIFY(capture("glass-one-block-setup.png"));
        keyClick(Qt::Key_Escape);
        QTRY_VERIFY(!setup->property("visible").toBool());
        QTRY_VERIFY(enable->property("activeFocus").toBool());
        keyClick(Qt::Key_Escape);
        QTRY_VERIFY(!settings->property("visible").toBool());
        QVERIFY(QMetaObject::invokeMethod(updates, "open"));
        QTRY_VERIFY(updates->property("visible").toBool());
        auto* help = window->findChild<QObject*>("backgroundBlurHelp");
        QCOMPARE(help->property("text").toString(), QString("Enable Glass"));
        QVERIFY(help->property("visible").toBool());
        QVERIFY(QMetaObject::invokeMethod(help, "clicked"));
        QTRY_VERIFY(setup->property("visible").toBool());
        QTRY_VERIFY(!blur->busy());
        blur->setProbe([] { return BlurSupport::Result{}; });
        blur->refresh();
        QTRY_VERIFY(!blur->setupAvailable());
        QVERIFY(!window->findChild<QObject*>("glassCommandWell")->property("visible").toBool());
        QVERIFY(setup->property("visible").toBool());
        QVERIFY(!appearance->effectiveGlassEnabled());
        QCOMPARE(window->property("backgroundAlpha").toDouble(), 1.0);
        QVERIFY(capture("enable-glass-fallback-1024.png"));
        QTRY_VERIFY(!blur->busy());
        blur->setProbe([] { return BlurSupport::Result{true, {}, {}}; });
        blur->refresh();
        QTRY_VERIFY(blur->ready());
        blur->setProbe([] { return BlurSupport::Result{true, "repair-command", {}}; });
        blur->refresh();
        QTRY_VERIFY(blur->setupAvailable());
        QVERIFY(copy->property("visible").toBool());
        QCOMPARE(enable->property("text").toString(), QString("Glass setup"));
        auto* useGlass = window->findChild<QObject*>("useGlass");
        QVERIFY(useGlass->property("visible").toBool());
        QVERIFY(QMetaObject::invokeMethod(useGlass, "clicked"));
        QTRY_VERIFY(!setup->property("visible").toBool());
        QVERIFY(appearance->effectiveGlassEnabled());
        QVERIFY(!help->property("visible").toBool());
        QTRY_VERIFY(window->findChild<QObject*>("updatesDone")->property("activeFocus").toBool());
        QCOMPARE(editor->image(), artwork);
        keyClick(Qt::Key_Escape);
    }

    void updatesWorkflow() {
        QDesktopServices::setUrlHandler("https", this, "captureReleaseUrl");
        const auto resetHandler = qScopeGuard([] { QDesktopServices::unsetUrlHandler("https"); });
        auto* editor = window->findChild<Editor*>(QStringLiteral("editor"));
        auto* action = window->findChild<QObject*>(QStringLiteral("updatesMenuItem"));
        auto* dialog = window->findChild<QObject*>(QStringLiteral("updatesDialog"));
        QVERIFY(editor && action && dialog);
        auto* credit = window->findChild<QObject*>("designerCredit");
        QVERIFY(credit);
        QCOMPARE(credit->property("text").toString(),
                 QString("Designer: Afshin Kashani, PhD, P.Eng"));
        auto* service = window->findChild<QObject*>("updateService");
        QVERIFY(service);
        QVERIFY(!service->property("automatic").toBool());
        QVERIFY(!service->property("busy").toBool());
        editor->begin({30, 30}, false);
        editor->extend({80, 50});
        editor->finish();
        const auto artwork = editor->image();
#ifdef Q_OS_MACOS
        // macOS has no Alt-letter menu mnemonics; exercise its visible menu.
        auto* fileButton = window->findChild<QQuickItem*>("fileMenuButton");
        QVERIFY(fileButton);
        QTest::mouseClick(window, Qt::LeftButton, {},
                          fileButton->mapToScene(fileButton->boundingRect().center()).toPoint());
#else
        keyClick(Qt::Key_F, Qt::AltModifier);
#endif
        for (int i = 0; i < 6; ++i)
            keyClick(Qt::Key_Down); // New/Open/Save/Save As/Export/Updates.
        keyClick(Qt::Key_Return);
        QTRY_VERIFY(dialog->property("visible").toBool());
        QVERIFY(!window->findChild<QObject*>("checkReleaseButton"));
        QVERIFY(!window->findChild<QObject*>("automaticReleaseChecks"));
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
        QTRY_VERIFY(options->property("opened").toBool());
        // Keep the pointer outside the popup so scaled X11 hover events cannot
        // override the keyboard-highlighted format.
        QTest::mouseMove(window, QPoint(1, 1));
        QVERIFY(tabTo("exportFormat"));
        keyClick(Qt::Key_Space);
        auto* formats = window->findChild<QObject*>("exportFormatsMenu");
        QVERIFY(formats);
        QTRY_VERIFY(formats->property("opened").toBool());
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
        QVERIFY(file->property("defaultSuffix").toString().isEmpty());
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

    void ribbonActionMemory() {
        auto* canvas = window->findChild<CanvasItem*>("canvas");
        auto* editor = canvas->editor();
        auto* selection = window->findChild<QObject*>("selectionButton");
        editor->setProperty("freeFormSelection", true);
        QCOMPARE(selection->property("symbol").toString(), QStringLiteral("lasso"));
        editor->setProperty("freeFormSelection", false);
        QCOMPARE(selection->property("symbol").toString(), QStringLiteral("selection"));
        editor->begin({15, 25}, false);
        editor->extend({60, 40});
        editor->endGesture();
        const auto original = editor->image();
        for (const auto& name :
             {"rotateLeft", "rotateHalf", "rotateRight", "flipVertical", "flipHorizontal"}) {
            auto* action = window->findChild<QObject*>(QString::fromLatin1(name) + "Action");
            QVERIFY(action);
            QVERIFY(QMetaObject::invokeMethod(action, "trigger", Q_ARG(QObject*, nullptr)));
            QTRY_VERIFY(!editor->busy());
            const auto once = editor->image();
            const bool rotation = QString::fromLatin1(name).startsWith("rotate");
            auto* button = window->findChild<QObject*>(rotation ? "rotateButton" : "flipButton");
            QCOMPARE(button->property("action").value<QObject*>(), action);
            QCOMPARE(button->property("text"), action->property("text"));
            editor->undo();
            QCOMPARE(editor->image(), original);
            QVERIFY(QMetaObject::invokeMethod(button, "click"));
            QTRY_VERIFY(!editor->busy());
            QCOMPARE(editor->image(), once);
            editor->undo();
            QCOMPARE(editor->image(), original);
        }
        QVERIFY(capture(QStringLiteral("ribbon-action-memory.png")));
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

    void colorSelectionWorkflow() {
        auto* canvas = window->findChild<CanvasItem*>("canvas");
        auto* editor = canvas->editor();
        editor->primary = Qt::red;
        editor->brushSize = 4;
        editor->begin({100, 100}, false);
        editor->extend({150, 100});
        editor->finish();
        editor->begin({300, 250}, false);
        editor->extend({350, 250});
        editor->finish();
        const auto source = editor->image();
        QVERIFY(tabTo("selectionMenuButton"));
        keyClick(Qt::Key_Space);
        auto* menu = window->findChild<QObject*>("selectionMenu");
        QTRY_VERIFY(menu->property("visible").toBool());
        keyClick(Qt::Key_Down);
        keyClick(Qt::Key_Down);
        keyClick(Qt::Key_Down);
        keyClick(Qt::Key_Return);
        QTRY_VERIFY(editor->colorSelection);
        auto* button = window->findChild<QObject*>("selectionButton");
        QCOMPARE(button->property("symbol").toString(), QString("wand"));
        QCOMPARE(button->property("text").toString(), QString("Select by color"));
        auto at = [&](QPointF p) { return canvas->mapToScene(canvas->fromDocument(p)).toPoint(); };
        QTest::mouseClick(window, Qt::LeftButton, {}, at({120, 100}));
        QTRY_VERIFY(!editor->busy());
        QVERIFY(editor->selection().contains(QPoint(120, 100)));
        QVERIFY(editor->selection().contains(QPoint(320, 250)));
        QVERIFY(!editor->selection().contains(QPoint(220, 180)));
        QTest::mouseClick(window, Qt::LeftButton, {}, at({220, 180}));
        QTRY_VERIFY(!editor->busy());
        QVERIFY(editor->selection().contains(QPoint(220, 180)));
        QVERIFY(!editor->selection().contains(QPoint(120, 100)));
        QCOMPARE(editor->image(), source);
        QVERIFY(QMetaObject::invokeMethod(menu, "open"));
        QTRY_VERIFY(menu->property("visible").toBool());
        window->resize(1024, 640);
        QVERIFY(capture("color-selection-menu.png"));
        keyClick(Qt::Key_Escape);
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
        QCOMPARE(editor->image().pixelColor(150, 150), editor->secondary);
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

    void fontPickerWorkflow() {
        auto* editor = window->findChild<Editor*>("editor");
        auto* input = window->findChild<QQuickItem*>("inlineTextInput");
        auto* picker = window->findChild<QObject*>("fontPicker");
        auto* search = window->findChild<QQuickItem*>("fontSearch");
        auto* list = window->findChild<QQuickItem*>("fontList");
        QVERIFY(editor && picker && search && list);
        editor->setTool(Editor::Tool::Text);
        editor->begin({80, 80}, false);
        editor->extend({400, 180});
        editor->endGesture();
        QTRY_VERIFY(editor->textReady());
        editor->setTextDraft("Font preview");
        const auto family = editor->textFamily;
        const auto size = editor->textSize;
        const bool dirty = editor->dirty();
        const bool undo = editor->canUndo();
        QVERIFY(QMetaObject::invokeMethod(picker, "open"));
        QTRY_VERIFY(search->hasActiveFocus());
        search->setProperty("text", "no-such-font-987654321");
        QCOMPARE(list->property("count").toInt(), 0);
        keyClick(Qt::Key_Escape);
        QTRY_VERIFY(!picker->property("visible").toBool());
        QCOMPARE(editor->textFamily, family);
        QTRY_VERIFY(input->hasActiveFocus());
        QVERIFY(QMetaObject::invokeMethod(picker, "open"));
        search->setProperty("text", "");
        QVERIFY(list->property("count").toInt() > 0);
        keyClick(Qt::Key_Down);
        QTRY_VERIFY(list->hasActiveFocus());
        keyClick(Qt::Key_Space);
        const auto chosen = picker->property("draftFamily").toString();
        QVERIFY(!chosen.isEmpty());
        window->resize(1024, 640);
        for (const auto& theme : {QString("light"), QString("dark")}) {
            QVERIFY(QMetaObject::invokeMethod(window->findChild<QObject*>(theme + "ThemeAction"),
                                              "trigger", Q_ARG(QObject*, nullptr)));
            QVERIFY(capture("font-picker-" + theme + "-1024.png"));
        }
        list->forceActiveFocus();
        QTRY_VERIFY(list->hasActiveFocus());
        keyClick(Qt::Key_Return);
        QTRY_VERIFY(!picker->property("visible").toBool());
        QCOMPARE(editor->textFamily, chosen);
        QCOMPARE(editor->textSize, size);
        QVERIFY(editor->textActive());
        QTRY_VERIFY(input->hasActiveFocus());
        QCOMPARE(editor->dirty(), dirty);
        QCOMPARE(editor->canUndo(), undo);
        QVERIFY(QMetaObject::invokeMethod(picker, "open"));
        picker->setProperty("draftFamily", family);
        QVERIFY(tabToButton("Apply"));
        keyClick(Qt::Key_Space);
        QTRY_VERIFY(!picker->property("visible").toBool());
        QCOMPARE(editor->textFamily, family);
        QTRY_VERIFY(input->hasActiveFocus());
    }

    void textRangeFormattingWorkflow() {
        auto* editor = window->findChild<Editor*>("editor");
        auto* canvas = window->findChild<CanvasItem*>("canvas");
        auto* input = window->findChild<QQuickItem*>("inlineTextInput");
        auto* picker = window->findChild<QObject*>("fontPicker");
        const auto original = editor->image();
        editor->setTool(Editor::Tool::Text);
        editor->begin({80, 80}, false);
        editor->extend({600, 260});
        editor->endGesture();
        QTRY_VERIFY(input->hasActiveFocus());
        for (char c : std::string("Alpha Beta"))
            keyClick(c);
        auto* wrapper = input->property("textDocument").value<QQuickTextDocument*>();
        QVERIFY(wrapper);
        auto* document = wrapper->textDocument();
        auto fontAt = [&](int position) {
            QTextCursor cursor(document);
            cursor.setPosition(position);
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
            return cursor.charFormat().font().resolve(document->defaultFont());
        };
        const auto originalFont = fontAt(6);
        QVERIFY(QMetaObject::invokeMethod(input, "select", Q_ARG(int, 0), Q_ARG(int, 5)));
        QVERIFY(QMetaObject::invokeMethod(picker, "open"));
        QTRY_VERIFY(picker->property("visible").toBool());
        picker->setProperty("draftFamily", "Monospace");
        QVERIFY(QMetaObject::invokeMethod(picker, "accept"));
        QTRY_VERIFY(input->hasActiveFocus());
        QCOMPARE(fontAt(0).family(), QString("Monospace"));
        QCOMPARE(fontAt(6).family(), originalFont.family());
        QCOMPARE(input->property("selectionStart").toInt(), 0);
        QCOMPARE(input->property("selectionEnd").toInt(), 5);
        editor->setProperty("textBold", true);
        editor->setProperty("textSize", 32);
        editor->setProperty("textFamily", "Serif");
        QCOMPARE(fontAt(0).family(), QString("Serif"));
        QCOMPARE(fontAt(0).pixelSize(), 32);
        QVERIFY(fontAt(0).bold());
        QCOMPARE(fontAt(6), originalFont);
        // Family changes must preserve different sizes and styles within a range.
        QVERIFY(QMetaObject::invokeMethod(input, "select", Q_ARG(int, 0), Q_ARG(int, 10)));
        editor->setProperty("textFamily", "Monospace");
        QCOMPARE(fontAt(0).pixelSize(), 32);
        QVERIFY(fontAt(0).bold());
        QCOMPARE(fontAt(6).pixelSize(), originalFont.pixelSize());
        QCOMPARE(fontAt(6).bold(), originalFont.bold());
        keyClick(Qt::Key_Z, Qt::ControlModifier);
        QCOMPARE(fontAt(0).family(), QString("Serif"));
        QCOMPARE(fontAt(6).family(), originalFont.family());
        const auto beforeCancel = editor->image();
        QVERIFY(QMetaObject::invokeMethod(picker, "open"));
        picker->setProperty("draftFamily", "Monospace");
        QVERIFY(QMetaObject::invokeMethod(picker, "reject"));
        QTRY_VERIFY(input->hasActiveFocus());
        QCOMPARE(editor->image(), beforeCancel);
        QVERIFY(QMetaObject::invokeMethod(input, "deselect"));
        input->setProperty("cursorPosition", 10);
        const auto beforeTyping = editor->image();
        editor->setProperty("textFamily", "Monospace");
        QCOMPARE(editor->image(), beforeTyping);
        keyClick('X');
        QCOMPARE(editor->textDraft(), QString("Alpha BetaX"));
        QCOMPARE(fontAt(10).family(), QString("Monospace"));
        QCOMPARE(fontAt(9).family(), originalFont.family());
        keyClick(Qt::Key_Z, Qt::ControlModifier);
        QCOMPARE(editor->textDraft(), QString("Alpha Beta"));
        keyClick(Qt::Key_Y, Qt::ControlModifier);
        QCOMPARE(editor->textDraft(), QString("Alpha BetaX"));
        QCOMPARE(fontAt(10).family(), QString("Monospace"));
        const auto beforePaste = editor->image();
        QGuiApplication::clipboard()->setText(QString(10001, 'x'));
        keyClick(Qt::Key_V, Qt::ControlModifier);
        QCOMPARE(editor->textDraft(), QString("Alpha BetaX"));
        QCOMPARE(editor->image(), beforePaste);
        QCOMPARE(fontAt(0).family(), QString("Serif"));
        auto* error = window->findChild<QObject*>("errorDialog");
        QTRY_VERIFY(error->property("visible").toBool());
        QVERIFY(QMetaObject::invokeMethod(error, "accept"));
        QTRY_VERIFY(!error->property("visible").toBool());
        const auto rendered = editor->image();
        canvas->zoomAt(1.5, QPointF(200, 200));
        QCoreApplication::processEvents();
        QCOMPARE(editor->image(), rendered);
        QVERIFY(capture("mixed-font-text.png"));
        QTemporaryDir directory;
        const auto path = directory.filePath("mixed-font.png");
        editor->saveFile(QUrl::fromLocalFile(path));
        QTRY_VERIFY(!editor->busy());
        QVERIFY(!editor->textActive());
        QCOMPARE(openpaint::loadPng(path).image, rendered);
        editor->undo();
        QCOMPARE(editor->image(), original);
        editor->redo();
        QCOMPARE(editor->image(), rendered);
    }

    void textInsertionFormatsWorkflow() {
        auto* editor = window->findChild<Editor*>("editor");
        auto* input = window->findChild<QQuickItem*>("inlineTextInput");
        editor->setTool(Editor::Tool::Text);
        editor->begin({80, 80}, false);
        editor->extend({600, 280});
        editor->endGesture();
        QTRY_VERIFY(input->hasActiveFocus());
        for (char c : std::string("before after"))
            keyClick(c);
        auto* document =
            input->property("textDocument").value<QQuickTextDocument*>()->textDocument();
        auto fontAt = [&](int position) {
            QTextCursor cursor(document);
            cursor.setPosition(position);
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
            return cursor.charFormat().font().resolve(document->defaultFont());
        };
        const auto originalFont = fontAt(0);
        input->setProperty("cursorPosition", 3); // Inside an existing word.
        const auto original = editor->image();
        editor->setProperty("textFamily", "Monospace");
        editor->setProperty("textSize", 28);
        editor->setProperty("textItalic", true);
        QCOMPARE(editor->image(), original);
        keyClick('1');
        keyClick('2');
        QInputMethodEvent commit;
        commit.setCommitString(QString::fromUtf8("Ω"));
        QCoreApplication::sendEvent(input, &commit);
        QCOMPARE(editor->textDraft(), QString::fromUtf8("bef12Ωore after"));
        for (int i = 3; i < 6; ++i) {
            QCOMPARE(fontAt(i).family(), QString("Monospace"));
            QCOMPARE(fontAt(i).pixelSize(), 28);
            QVERIFY(fontAt(i).italic());
        }
        QCOMPARE(fontAt(2), originalFont);
        QCOMPARE(fontAt(6), originalFont);
        auto* mime = new QMimeData;
        mime->setHtml("<b>rich</b>");
        mime->setText("<b>plain</b>");
        QGuiApplication::clipboard()->setMimeData(mime);
        keyClick(Qt::Key_V, Qt::ControlModifier);
        QVERIFY(editor->textDraft().contains("<b>plain</b>"));
        QCOMPARE(fontAt(6).family(), QString("Monospace"));
        QVERIFY(!fontAt(6).bold());
        input->setProperty("cursorPosition", 0);
        keyClick('Z');
        QCOMPARE(fontAt(0), originalFont); // Moving the caret uses its local font.
        editor->cancel();
        QVERIFY(!editor->textActive());
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
        QTRY_VERIFY(!visualItems(input, "textCaret").isEmpty());
        auto* caret = visualItems(input, "textCaret").first();
        QVERIFY(caret->height() > 0);
        QCOMPARE(caret->property("color").value<QColor>(), QColor(Qt::black));
        auto* outline = window->findChild<QQuickItem*>("textOutline");
        QVERIFY(outline && outline->isVisible());
        QVERIFY(editor->textDraft().isEmpty());
        const auto caretPixel = [&] {
            const auto frame = window->grabWindow();
            const auto point = caret->mapToScene({caret->width() / 2, caret->height() / 2});
            // Software grabs can round DPR metadata at fractional display scales.
            const qreal scaleX = qreal(frame.width()) / window->width();
            const qreal scaleY = qreal(frame.height()) / window->height();
            return frame.pixelColor(qRound(point.x() * scaleX), qRound(point.y() * scaleY));
        };
        QTRY_VERIFY(caretPixel().lightness() < 30);
        QVERIFY(capture("empty-text-caret.png"));
        QTRY_VERIFY(caretPixel().lightness() > 220);
        QTRY_VERIFY(caretPixel().lightness() < 30);
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
        QTRY_VERIFY(options->property("visible").toBool());
        QVERIFY(capture(QStringLiteral("text-options-light.png")));
        auto* dark = window->findChild<QObject*>(QStringLiteral("darkThemeAction"));
        QVERIFY(QMetaObject::invokeMethod(dark, "trigger", Q_ARG(QObject*, nullptr)));
        QVERIFY(capture(QStringLiteral("text-options-dark.png")));
        input->forceActiveFocus();
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

    void transparentBackgroundChoice() {
        auto* canvas = window->findChild<CanvasItem*>("canvas");
        auto* editor = canvas->editor();
        const auto original = editor->image();
        const auto foreground = editor->primary;
        auto* button = window->findChild<QObject*>("transparentBackgroundButton");
        QVERIFY(button);
        QVERIFY(QMetaObject::invokeMethod(button, "click"));
        QCOMPARE(editor->secondary.alpha(), 0);
        QCOMPARE(editor->primary, foreground);
        QVERIFY(editor->editingSecondary());
        QCOMPARE(editor->image(), original);
        QVERIFY(!editor->canUndo());
        editor->resizeCanvas(0, 0, original.width() + 1, original.height());
        QTRY_VERIFY(!editor->busy());
        QCOMPARE(editor->image().pixelColor(original.width(), 0).alpha(), 0);
        editor->undo();
        QCOMPARE(editor->image(), original);
        QVERIFY(capture(QStringLiteral("transparent-background.png")));
    }
    void canvasCornerResize() {
        auto* canvas = window->findChild<CanvasItem*>("canvas");
        auto* editor = canvas->editor();
        const auto original = editor->image();
        const auto handles = visualItems(window->contentItem(), "canvasResizeCorner3");
        QCOMPARE(handles.size(), 1);
        auto* handle = handles.first();
        QVERIFY(handle->isVisible());
        const auto start = handle->mapToScene({8, 8}).toPoint();
        const auto end = start - QPoint(70, 50);
        QTest::mousePress(window, Qt::LeftButton, {}, start);
        QTest::mouseMove(window, end);
        QCOMPARE(editor->image(), original);
        keyClick(Qt::Key_Escape);
        QTest::mouseRelease(window, Qt::LeftButton, {}, end);
        QCOMPARE(editor->image(), original);
        QTest::mousePress(window, Qt::LeftButton, {}, start);
        QTest::mouseMove(window, end);
        QTest::mouseRelease(window, Qt::LeftButton, {}, end);
        QTRY_VERIFY(!editor->busy());
        QVERIFY(editor->imageSize().width() < original.width());
        QVERIFY(editor->imageSize().height() < original.height());
        editor->undo();
        QCOMPARE(editor->image(), original);
        QVERIFY(!editor->canUndo());
        for (int index = 4; index < 8; ++index) {
            const auto edges =
                visualItems(window->contentItem(), "canvasResizeCorner" + QString::number(index));
            QCOMPARE(edges.size(), 1);
            auto* edge = edges.first();
            QVERIFY(edge->isVisible());
            const auto from = edge->mapToScene({8, 8}).toPoint();
            const auto to = from + QPoint(index == 4 ? 30 : -30, index == 6 ? 20 : -20);
            QTest::mousePress(window, Qt::LeftButton, {}, from);
            QTest::mouseMove(window, to);
            QCOMPARE(editor->image(), original);
            QTest::mouseRelease(window, Qt::LeftButton, {}, to);
            QTRY_VERIFY(!editor->busy());
            const auto resized = editor->image();
            if (index < 6) {
                QVERIFY(resized.width() < original.width());
                QCOMPARE(resized.height(), original.height());
            } else {
                QCOMPARE(resized.width(), original.width());
                QVERIFY(resized.height() < original.height());
            }
            editor->undo();
            QCOMPARE(editor->image(), original);
            editor->redo();
            QCOMPARE(editor->image(), resized);
            editor->undo();
        }
        const auto right = visualItems(window->contentItem(), "canvasResizeCorner5").first();
        right->forceActiveFocus();
        keyClick(Qt::Key_Left);
        QTRY_VERIFY(!editor->busy());
        QCOMPARE(editor->imageSize(), QSize(original.width() - 1, original.height()));
        editor->undo();
        QCOMPARE(editor->image(), original);
        QVERIFY(capture(QStringLiteral("canvas-edge-handles.png")));
    }
    void cornerResizeHandles() {
        auto* canvas = window->findChild<CanvasItem*>(QStringLiteral("canvas"));
        auto* editor = canvas->editor();
        const auto original = editor->image();
        editor->setTool(Editor::Tool::Text);
        editor->begin({100, 180}, false);
        editor->endGesture();
        for (char letter : std::string("Resizable text"))
            keyClick(letter);
        QCOMPARE(editor->textDraft(), QStringLiteral("Resizable text"));
        QGuiApplication::sync();
        const auto before = editor->textRect();
        const auto corners = visualItems(window->contentItem(), QStringLiteral("resizeCorner3"));
        QCOMPARE(corners.size(), 1);
        auto* corner = corners.first();
        QVERIFY(corner && corner->isVisible());
        auto center = [&] { return corner->mapToScene({8, 8}).toPoint(); };
        const auto start = center();
        const auto end = start + QPoint(qRound(80 * canvas->zoom()), qRound(40 * canvas->zoom()));
        QTest::mousePress(window, Qt::LeftButton, {}, start);
        QTest::mouseMove(window, end);
        QCOMPARE(editor->textRect(), before); // Dragging only previews the bounds.
        keyClick(Qt::Key_Escape);
        QTest::mouseRelease(window, Qt::LeftButton, {}, end);
        QCOMPARE(editor->textRect(), before);
        QVERIFY(editor->textActive());
        QTest::mousePress(window, Qt::LeftButton, {}, center());
        QTest::mouseMove(window, end);
        QTest::mouseRelease(window, Qt::LeftButton, {}, end);
        QVERIFY(editor->textActive());
        QCOMPARE(editor->textRect().topLeft(), before.topLeft());
        QVERIFY(std::abs(editor->textRect().width() - before.width() - 80) <= 1);
        QVERIFY(std::abs(editor->textRect().height() - before.height() - 40) <= 1);
        QVERIFY(capture(QStringLiteral("text-resize-handles.png")));
        const auto draft = editor->image();
        editor->commitPreview();
        editor->undo();
        QCOMPARE(editor->image(), original);
        editor->redo();
        QCOMPARE(editor->image(), draft);
        editor->selectRect(100, 180, 100, 80);
        QGuiApplication::sync();
        QVERIFY(corner->isVisible());
        QCOMPARE(corner->width(), 16.0);
        canvas->zoomBy(1.5);
        QCOMPARE(corner->width(), 16.0);
        corner->forceActiveFocus();
        keyClick(Qt::Key_Right);
        QTRY_VERIFY(!editor->busy());
        QVERIFY(editor->floatingActive());
        QCOMPARE(editor->resizeBounds().width(), 101.0);
        editor->cancel();
        QCOMPARE(editor->image(), draft);
    }

    void textToolbarOutsideClickCommit() {
        auto* canvas = window->findChild<CanvasItem*>(QStringLiteral("canvas"));
        auto* editor = canvas->editor();
        auto* options = window->findChild<QQuickItem*>(QStringLiteral("textOptions"));
        const auto original = editor->image();
        editor->setTool(Editor::Tool::Text);
        QVERIFY(options->isVisible());
        auto at = [&](QPointF p) { return canvas->mapToScene(canvas->fromDocument(p)).toPoint(); };
        QTest::mousePress(window, Qt::LeftButton, {}, at({80, 180}));
        QTest::mouseMove(window, at({400, 280}));
        QTest::mouseRelease(window, Qt::LeftButton, {}, at({400, 280}));
        QTRY_VERIFY(editor->textReady());
        keyClick('x');
        QVERIFY(tabTo("textBold"));
        keyClick(Qt::Key_Space);
        QVERIFY(editor->textBold);
        QVERIFY(editor->textActive());
        auto* apply = window->findChild<QQuickItem*>(QStringLiteral("applyPreviewButton"));
        QVERIFY(apply && !apply->isVisible());
        const auto draft = editor->image();
        QTest::mouseClick(window, Qt::LeftButton, {}, at({600, 400}));
        QVERIFY(!editor->textActive());
        QCOMPARE(editor->image(), draft);
        QVERIFY(editor->dirty());
        editor->undo();
        QCOMPARE(editor->image(), original);
        editor->redo();
        QCOMPARE(editor->image(), draft);
        window->resize(1024, 640);
        QVERIFY(capture(QStringLiteral("text-toolbar-1024.png")));
        QVERIFY(QRectF(0, 0, window->width(), window->height())
                    .contains(options->mapRectToScene(options->boundingRect())));
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

    void saveDialogsPreserveAuthorizedFilename() {
        auto* editor = window->findChild<Editor*>(QStringLiteral("editor"));
        auto* save = window->findChild<QObject*>(QStringLiteral("saveDialog"));
        auto* exportFile = window->findChild<QObject*>(QStringLiteral("exportFileDialog"));
        auto* error = window->findChild<QObject*>(QStringLiteral("errorDialog"));
        QVERIFY(editor && save && exportFile && error);
        QTemporaryDir directory;
        const auto path = directory.filePath("authorized-name");
        QFile original(path);
        QVERIFY(original.open(QIODevice::WriteOnly));
        QCOMPARE(original.write("original"), 8);
        original.close();
        const auto url = QUrl::fromLocalFile(path);
        editor->begin({40, 40}, false);
        editor->extend({90, 90});
        editor->finish();
        const auto artwork = editor->image();
        QSignalSpy saved(editor, &Editor::saved);
        QSignalSpy exported(editor, &Editor::exported);
        for (auto* dialog : {save, exportFile}) {
            QVERIFY(dialog->setProperty("selectedFile", url));
            QCOMPARE(dialog->property("selectedFile").toUrl(), url);
            QVERIFY(QMetaObject::invokeMethod(dialog, "accepted"));
            QTRY_VERIFY(error->property("visible").toBool());
            QVERIFY(!editor->busy());
            QVERIFY(editor->dirty());
            QVERIFY(editor->fileUrl().isEmpty());
            QCOMPARE(editor->image(), artwork);
            QVERIFY(!QFile::exists(path + ".png"));
            QVERIFY(original.open(QIODevice::ReadOnly));
            QCOMPARE(original.readAll(), QByteArray("original"));
            original.close();
            QVERIFY(QMetaObject::invokeMethod(error, "accept"));
            QTRY_VERIFY(!error->property("visible").toBool());
        }
        QCOMPARE(saved.size(), 1);
        QCOMPARE(saved.first().first().toBool(), false);
        QCOMPARE(exported.size(), 1);
        QCOMPARE(exported.first().first().toBool(), false);
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
        keyClick(Qt::Key_Down);
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

    void frostedToolSelection() {
        auto* editor = window->findChild<Editor*>("editor");
        const auto image = editor->image();
        auto* pencil = window->findChild<QQuickItem*>("pencilButton");
        auto* eraser = window->findChild<QQuickItem*>("eraserButton");
        QVERIFY(pencil && eraser);
        const auto pencilSurfaces = visualItems(pencil, "selectedToolSurface");
        const auto eraserSurfaces = visualItems(eraser, "selectedToolSurface");
        QVERIFY(!pencilSurfaces.isEmpty() && !eraserSurfaces.isEmpty());
        QQmlExpression glass(qmlContext(pencil), pencil,
                             "Appearance.glassEnabled = true; Appearance.surfaceOpacity = 40");
        glass.evaluate();
        QVERIFY(!glass.hasError());
        editor->setTool(Editor::Tool::Pencil);
        QVERIFY(pencilSurfaces.first()->isVisible());
        QVERIFY(!eraserSurfaces.first()->property("selectedSurface").toBool());
        QVERIFY(tabTo("eraserButton"));
        keyClick(Qt::Key_Space);
        QCOMPARE(editor->tool(), Editor::Tool::Eraser);
        QVERIFY(!pencilSurfaces.first()->property("selectedSurface").toBool());
        QVERIFY(eraserSurfaces.first()->isVisible());
        window->resize(1024, 640);
        for (const auto& theme : {QString("light"), QString("dark")}) {
            QVERIFY(QMetaObject::invokeMethod(window->findChild<QObject*>(theme + "ThemeAction"),
                                              "trigger", Q_ARG(QObject*, nullptr)));
            QVERIFY(capture("frosted-selection-" + theme + ".png"));
        }
        editor->setTool(Editor::Tool::Select);
        QVERIFY(window->findChild<QQuickItem*>("selectSelectedSurface")->isVisible());
        editor->setTool(Editor::Tool::Brush);
        QVERIFY(window->findChild<QQuickItem*>("brushSelectedSurface")->isVisible());
        QQmlExpression solid(qmlContext(pencil), pencil, "Appearance.glassEnabled = false");
        solid.evaluate();
        QVERIFY(!solid.hasError());
        editor->setTool(Editor::Tool::Eraser);
        QVERIFY(!eraserSurfaces.first()->isVisible());
        QVERIFY(capture("selection-glass-disabled.png"));
        QCOMPARE(editor->image(), image);
        QVERIFY(!editor->dirty());
        QVERIFY(!editor->canUndo());
    }

    void glassAppearanceSettings() {
        auto* editor = window->findChild<Editor*>("editor");
        auto* canvas = window->findChild<CanvasItem*>("canvas");
        const auto artwork = editor->image();
        auto* dialog = window->findChild<QObject*>("appearanceDialog");
        auto* surface = window->findChild<QObject*>("ribbonSurface");
        auto* toggle = window->findChild<QObject*>("glassEnabled");
        auto* slider = window->findChild<QObject*>("glassOpacity");
        auto* workspace = window->findChild<QObject*>("workspaceSurface");
        QVERIFY(dialog && surface && toggle && slider && workspace);
        QVERIFY(QMetaObject::invokeMethod(dialog, "open"));
        QVERIFY(tabTo("restoreAppearance"));
        keyClick(Qt::Key_Space);
        QCOMPARE(slider->property("value").toInt(), 40);
        QCOMPARE(slider->property("from").toInt(), 40);
        QVERIFY(!window->findChild<QObject*>("glassTint"));
        QVERIFY(tabTo("glassOpacity"));
        keyClick(Qt::Key_Right);
        QCOMPARE(slider->property("value").toInt(), 41);
        QCOMPARE(surface->property("surfaceAlpha").toDouble(), 0.41);
        QVERIFY(qAbs(workspace->property("color").value<QColor>().alphaF() - 0.41) < 0.005);
        QCOMPARE(window->color().alpha(), 0);
        // The base covers the toolbar margins too; no clear slit between bars.
        auto* base = qobject_cast<QQuickItem*>(workspace);
        QVERIFY(base);
        QCOMPARE(base->width(), qreal(window->width()));
        QCOMPARE(base->height(), qreal(window->height()));
        auto* toolbar = window->findChild<QQuickItem*>("mainToolbar");
        const auto gap = toolbar->mapToScene(QPointF(toolbar->width() / 2, 1));
        const auto gapAlpha = [&] {
            const auto frame = window->grabWindow();
            const auto scale = frame.devicePixelRatio();
            return frame.pixelColor(qRound(gap.x() * scale), qRound(gap.y() * scale)).alphaF();
        };
        QTRY_VERIFY(gapAlpha() >= 0.40);
        QVERIFY(capture("glass-toolbar-gap.png"));
        QVERIFY(tabTo("glassEnabled"));
        keyClick(Qt::Key_Space);
        QCOMPARE(surface->property("surfaceAlpha").toDouble(), 1.0);
        QCOMPARE(workspace->property("color").value<QColor>().alpha(), 255);
        QCOMPARE(window->color().alpha(), 0); // Clear corners; client surface is opaque.
        QVERIFY(!slider->property("enabled").toBool());
        QVERIFY(capture("glass-disabled.png"));
        keyClick(Qt::Key_Space);
        QCOMPARE(surface->property("surfaceAlpha").toDouble(), 0.41);
        QVERIFY(tabTo("glassOpacity"));
        for (int step = 0; step < 59; ++step)
            keyClick(Qt::Key_Right);
        QCOMPARE(slider->property("value").toInt(), 100);
        QCOMPARE(window->color().alpha(), 0); // Clear corners; client surface is opaque.
        QCOMPARE(workspace->property("color").value<QColor>().alpha(), 255);
        keyClick(Qt::Key_Left);
        QCOMPARE(slider->property("value").toInt(), 99);
        QCOMPARE(window->color().alpha(), 0);
        QCOMPARE(canvas->opacity(), 1.0);
        QCOMPARE(window->findChild<QQuickItem*>("settingsButton")->opacity(), 1.0);
        QCOMPARE(editor->image(), artwork);
        QVERIFY(!editor->dirty());
        QVERIFY(!editor->canUndo());
        window->resize(1024, 640);
        QVERIFY(capture("glass-settings-light-1024.png"));
        QVERIFY(QMetaObject::invokeMethod(window->findChild<QObject*>("darkThemeAction"), "trigger",
                                          Q_ARG(QObject*, nullptr)));
        QVERIFY(capture("glass-settings-dark-1024.png"));
        QVERIFY(tabTo("restoreAppearance"));
        keyClick(Qt::Key_Space);
        keyClick(Qt::Key_Escape);
        window->resize(1280, 800);
        QVERIFY(capture("glass-dark-1280.png"));
        QVERIFY(QMetaObject::invokeMethod(window->findChild<QObject*>("lightThemeAction"),
                                          "trigger", Q_ARG(QObject*, nullptr)));
        QVERIFY(capture("glass-light-1280.png"));
    }
    // Explicit documentation capture, not part of CTest. Uses only generated content.
    void readmeFrostPreview() {
        window->resize(1449, 1008);
        auto* appearance = engine->singletonInstance<Appearance*>("OpenPaint", "Appearance");
        appearance->setGlassEnabled(true);
        appearance->setSurfaceOpacity(54);
        QVERIFY(QMetaObject::invokeMethod(window->findChild<QObject*>("lightThemeAction"),
                                          "trigger", Q_ARG(QObject*, nullptr)));
        // Staged multicolor backdrop, generated locally without desktop capture.
        QQmlComponent backdrop(engine.get());
        backdrop.setData(R"QML(import QtQuick
            Rectangle {
                width: 1449; height: 1008; z: -100
                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop { position: 0; color: "#234A70" }
                    GradientStop { position: 0.35; color: "#B17D98" }
                    GradientStop { position: 0.65; color: "#DDB777" }
                    GradientStop { position: 1; color: "#398B89" }
                }
            })QML",
                         QUrl());
        QScopedPointer<QObject> backing(backdrop.create());
        QVERIFY(backing);
        qobject_cast<QQuickItem*>(backing.data())->setParentItem(window->contentItem());
        QImage art(1152, 648, QImage::Format_RGB32);
        QPainter painter(&art);
        painter.setRenderHint(QPainter::Antialiasing);
        QLinearGradient sky(0, 0, 0, 648);
        sky.setColorAt(0, QColor("#303D72"));
        sky.setColorAt(0.6, QColor("#DC9C9C"));
        sky.setColorAt(1, QColor("#F4DDBB"));
        painter.fillRect(art.rect(), sky);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#FFE7AD"));
        painter.drawEllipse(QPointF(785, 208), 82, 82);
        painter.setBrush(QColor("#82779D"));
        painter.drawPolygon(QPolygonF{{0, 430},
                                      {165, 245},
                                      {310, 370},
                                      {470, 188},
                                      {660, 391},
                                      {870, 281},
                                      {1152, 422},
                                      {1152, 648},
                                      {0, 648}});
        painter.setBrush(QColor("#375F7C"));
        painter.drawPolygon(QPolygonF{{0, 505},
                                      {246, 355},
                                      {430, 480},
                                      {695, 326},
                                      {970, 475},
                                      {1152, 379},
                                      {1152, 648},
                                      {0, 648}});
        QPainterPath foreground;
        foreground.moveTo(0, 530);
        foreground.cubicTo(270, 435, 415, 610, 660, 504);
        foreground.cubicTo(890, 405, 1040, 550, 1152, 510);
        foreground.lineTo(1152, 648);
        foreground.lineTo(0, 648);
        foreground.closeSubpath();
        painter.fillPath(foreground, QColor("#204E59"));
        painter.end();
        QTemporaryDir dir;
        const auto path = dir.filePath("Frost study.png");
        QVERIFY(art.save(path));
        auto* editor = window->findChild<Editor*>("editor");
        editor->openFile(QUrl::fromLocalFile(path));
        QTRY_VERIFY(!editor->loading());
        QCOMPARE(editor->image().size(), art.size());
        auto* menu = window->findChild<QObject*>("brushesMenu");
        QVERIFY(QMetaObject::invokeMethod(menu, "open"));
        QTRY_VERIFY(menu->property("visible").toBool());
        QTest::qWait(250);
        QVERIFY(capture("openpaint-frost-light.png"));
    }
    // Explicit release screenshots; isolated settings and generated artwork only.
    void websiteReleasePreview() {
        const auto prefix = QStringLiteral("openpaint-") + AppInfo::version();
        window->resize(1449, 1008);
        auto* appearance = engine->singletonInstance<Appearance*>("OpenPaint", "Appearance");
        const bool stagedGlass = qEnvironmentVariableIsSet("OPENPAINT_RELEASE_GLASS");
        appearance->setGlassEnabled(stagedGlass);
        appearance->setSurfaceOpacity(40);
        // Optional documentation capture: original backdrop inside the test window.
        // This demonstrates real app surfaces, not desktop-compositor certification.
        QQmlComponent backdrop(engine.get());
        backdrop.setData(R"QML(import QtQuick
            Rectangle {
                width: 1449; height: 1008; z: -100
                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop { position: 0; color: "#234A70" }
                    GradientStop { position: 0.35; color: "#B17D98" }
                    GradientStop { position: 0.65; color: "#DDB777" }
                    GradientStop { position: 1; color: "#398B89" }
                }
            })QML",
                         QUrl());
        QScopedPointer<QObject> backing(stagedGlass ? backdrop.create() : nullptr);
        if (stagedGlass) {
            QVERIFY(backing);
            qobject_cast<QQuickItem*>(backing.data())->setParentItem(window->contentItem());
        }
        const auto releaseCapture = [&](const QString& name) {
            QGuiApplication::sync();
            auto frame = window->grabWindow().convertToFormat(QImage::Format_ARGB32);
            if (frame.isNull())
                return false;
            QImage mask(frame.size() * 4, QImage::Format_ARGB32);
            mask.fill(Qt::transparent);
            QPainter paint(&mask);
            paint.setRenderHint(QPainter::Antialiasing);
            paint.setPen(Qt::NoPen);
            paint.setBrush(Qt::white);
            paint.drawRoundedRect(QRectF(mask.rect()), 22 * 4, 22 * 4);
            paint.end();
            frame.setAlphaChannel(
                mask.scaled(frame.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            const auto directory = qEnvironmentVariable("OPENPAINT_CAPTURE_DIR");
            return directory.isEmpty() ||
                   (QDir().mkpath(directory) && frame.save(QDir(directory).filePath(name)));
        };
        QVERIFY(QMetaObject::invokeMethod(window->findChild<QObject*>("lightThemeAction"),
                                          "trigger", Q_ARG(QObject*, nullptr)));
        QImage art(1152, 648, QImage::Format_RGB32);
        QPainter painter(&art);
        painter.setRenderHint(QPainter::Antialiasing);
        QLinearGradient sky(0, 0, 0, 648);
        sky.setColorAt(0, QColor("#303D72"));
        sky.setColorAt(0.6, QColor("#DC9C9C"));
        sky.setColorAt(1, QColor("#F4DDBB"));
        painter.fillRect(art.rect(), sky);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#FFE7AD"));
        painter.drawEllipse(QPointF(785, 208), 82, 82);
        painter.end();
        QImage distant(art.size(), QImage::Format_ARGB32_Premultiplied);
        distant.fill(Qt::transparent);
        painter.begin(&distant);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#82779D"));
        painter.drawPolygon(QPolygonF{{0, 430},
                                      {165, 245},
                                      {310, 370},
                                      {470, 188},
                                      {660, 391},
                                      {870, 281},
                                      {1152, 422},
                                      {1152, 648},
                                      {0, 648}});
        painter.end();
        QImage near(art.size(), QImage::Format_ARGB32_Premultiplied);
        near.fill(Qt::transparent);
        painter.begin(&near);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#375F7C"));
        painter.drawPolygon(QPolygonF{{0, 505},
                                      {246, 355},
                                      {430, 480},
                                      {695, 326},
                                      {970, 475},
                                      {1152, 379},
                                      {1152, 648},
                                      {0, 648}});
        painter.end();
        QImage hills(art.size(), QImage::Format_ARGB32_Premultiplied);
        hills.fill(Qt::transparent);
        painter.begin(&hills);
        painter.setRenderHint(QPainter::Antialiasing);
        QPainterPath foreground;
        foreground.moveTo(0, 530);
        foreground.cubicTo(270, 435, 415, 610, 660, 504);
        foreground.cubicTo(890, 405, 1040, 550, 1152, 510);
        foreground.lineTo(1152, 648);
        foreground.lineTo(0, 648);
        foreground.closeSubpath();
        painter.fillPath(foreground, QColor("#204E59"));
        painter.end();
        QTemporaryDir dir;
        const auto path = dir.filePath("Frost study.ora");
        const openpaint::Layers artwork{
            {1, "Background", art.convertToFormat(QImage::Format_ARGB32_Premultiplied), true, 1.0},
            {2, "Distant peaks", distant, true, 1.0},
            {3, "Mountain ridge", near, true, 1.0},
            {4, "Foreground hills", hills, true, 1.0}};
        QVERIFY(openpaint::saveOra(path, artwork).error.isEmpty());
        auto* editor = window->findChild<Editor*>("editor");
        editor->openFile(QUrl::fromLocalFile(path));
        QTRY_VERIFY(!editor->loading());
        QCOMPARE(editor->image().size(), art.size());
        QCOMPARE(editor->layers().size(), 4);
        QTRY_VERIFY(window->findChild<QQuickItem*>("layersPanel")->isVisible());
        editor->selectLayer(2);
        for (const auto& theme : {QStringLiteral("light"), QStringLiteral("dark")}) {
            auto* action = window->findChild<QObject*>(theme + "ThemeAction");
            QVERIFY(QMetaObject::invokeMethod(action, "trigger", Q_ARG(QObject*, nullptr)));
            QTest::qWait(250);
            QVERIFY(releaseCapture(prefix + "-" + theme + ".png"));
        }
        auto* colorDialog = window->findChild<QObject*>("colorEditor");
        QVERIFY(colorDialog);
        QVERIFY(
            QMetaObject::invokeMethod(window->findChild<QObject*>("editColorsButton"), "clicked"));
        QTRY_VERIFY(colorDialog->property("visible").toBool());
        QTest::qWait(250);
        QVERIFY(releaseCapture(prefix + "-colors-dark.png"));
        QVERIFY(QMetaObject::invokeMethod(colorDialog, "close"));
        QTRY_VERIFY(!colorDialog->property("visible").toBool());
        QVERIFY(QMetaObject::invokeMethod(window->findChild<QObject*>("lightThemeAction"),
                                          "trigger", Q_ARG(QObject*, nullptr)));
        auto* resizeDialog = window->findChild<QObject*>("resizeDialog");
        QVERIFY(resizeDialog);
        QVERIFY(QMetaObject::invokeMethod(resizeDialog, "open"));
        QTRY_VERIFY(resizeDialog->property("visible").toBool());
        QTest::qWait(250);
        QVERIFY(releaseCapture(prefix + "-resize-light.png"));
    }
    void fileMenuBackdropBlur_data() {
        QTest::addColumn<bool>("dark");
        QTest::newRow("light") << false;
        QTest::newRow("dark") << true;
    }
    void fileMenuBackdropBlur() {
        QFETCH(bool, dark);
        auto* theme = window->findChild<QObject*>(dark ? "darkThemeAction" : "lightThemeAction");
        QVERIFY(QMetaObject::invokeMethod(theme, "trigger", Q_ARG(QObject*, nullptr)));
        auto* appearance = engine->singletonInstance<Appearance*>("OpenPaint", "Appearance");
        appearance->setSurfaceOpacity(40);
        appearance->setGlassEnabled(true);
        auto* toolbar = window->findChild<QQuickItem*>("mainToolbar");
        auto* menu = window->findChild<QObject*>("fileMenu");
        QVERIFY(toolbar && menu);
        QQmlComponent component(engine.get());
        component.setData(R"(
            import QtQuick
            Item {
                anchors.fill: parent; z: 100
                Repeater {
                    model: 20
                    Rectangle {
                        required property int index
                        y: index * 8; height: 8; width: parent.width
                        color: index % 2 ? "white" : "black"
                    }
                }
            }
        )",
                          QUrl("qrc:/toolbar-stripes.qml"));
        std::unique_ptr<QObject> fixture(component.create());
        QVERIFY2(fixture != nullptr, qPrintable(component.errorString()));
        qobject_cast<QQuickItem*>(fixture.get())->setParentItem(toolbar);
        QVERIFY(QMetaObject::invokeMethod(menu, "open"));
        QTRY_VERIFY(menu->property("visible").toBool());
        auto* panel = menu->property("background").value<QQuickItem*>();
        QVERIFY(panel);
        const bool software =
            window->rendererInterface()->graphicsApi() == QSGRendererInterface::Software;
        QTRY_COMPARE(panel->property("backdropBlurEnabled").toBool(), !software);
        // Neutral menu material must protect text from saturated/dark artwork.
        QVERIFY(panel->property("tintAlpha").toDouble() >= 0.72);
        QCOMPARE(appearance->surfaceOpacity(), 40);
        if (software) {
            QCOMPARE(panel->property("surfaceAlpha").toDouble(), 1.0);
            QVERIFY(QMetaObject::invokeMethod(menu, "close"));
            return;
        }
        const auto contrast = [&](const QImage& frame) {
            int low = 255, high = 0;
            const auto left = panel->mapToScene(QPointF(panel->width() - 4, 0)).x();
            for (int y = 16; y < int(toolbar->height()) - 16; ++y) {
                const auto sceneY = toolbar->mapToScene(QPointF(0, y)).y();
                const auto scale = frame.devicePixelRatio();
                const int v = frame.pixelColor(qRound(left * scale), qRound(sceneY * scale)).red();
                low = std::min(low, v);
                high = std::max(high, v);
            }
            return high - low;
        };
        QTest::qWait(200);
        const auto blurred = window->grabWindow();
        QVERIFY(capture(dark ? "file-menu-dark-blurred.png" : "file-menu-light-blurred.png"));
        panel->setProperty("backdropSource", QVariant::fromValue<QQuickItem*>(nullptr));
        QTest::qWait(200);
        const auto plain = window->grabWindow();
        QVERIFY(capture(dark ? "file-menu-dark-plain.png" : "file-menu-light-plain.png"));
        // Opaque fixture stripes keep the baseline above 50 even after the menu tint.
        QVERIFY2(contrast(plain) > 50, qPrintable(QString::number(contrast(plain))));
        if (!software)
            QVERIFY2(contrast(blurred) < contrast(plain) / 3,
                     "File menu must blur the toolbar behind it");
        QVERIFY(QMetaObject::invokeMethod(menu, "close"));
    }
    void actualFloatingPanelBlur_data() {
        QTest::addColumn<bool>("fillPanel");
        QTest::newRow("size") << false;
    }
    void actualFloatingPanelBlur() {
        QFETCH(bool, fillPanel);
        auto* editor = window->findChild<Editor*>("editor");
        auto* canvas = window->findChild<CanvasItem*>("canvas");
        auto* appearance = engine->singletonInstance<Appearance*>("OpenPaint", "Appearance");
        QVERIFY(editor && canvas && appearance);
        appearance->setSurfaceOpacity(40);
        appearance->setGlassEnabled(true);
        QTemporaryDir dir;
        QImage stripes(1152, 648, QImage::Format_RGB32);
        for (int y = 0; y < stripes.height(); ++y)
            for (int x = 0; x < stripes.width(); ++x)
                stripes.setPixelColor(x, y, (y / 8) % 2 ? Qt::white : Qt::black);
        const auto path = dir.filePath("stripes.png");
        QVERIFY(stripes.save(path));
        editor->openFile(QUrl::fromLocalFile(path));
        QTRY_VERIFY(!editor->loading());
        QCOMPARE(editor->image().size(), stripes.size());
        canvas->zoomBy(2);
        auto* rail = window->findChild<QQuickItem*>("floatingToolControls");
        auto* slider = window->findChild<QQuickItem*>("strokeSize");
        QVERIFY(rail && slider);
        QQuickItem* panel = slider;
        while (panel && panel->metaObject()->indexOfProperty("backdropSource") < 0)
            panel = panel->parentItem();
        if (fillPanel) {
            editor->setTool(Editor::Tool::Fill);
            panel = window->findChild<QQuickItem*>("fillControlsSurface");
        }
        QVERIFY(panel);
        QCOMPARE(panel->property("backdropSource").value<QQuickItem*>(), canvas);
        const bool software =
            window->rendererInterface()->graphicsApi() == QSGRendererInterface::Software;
        QTRY_COMPARE(panel->property("backdropBlurEnabled").toBool(), !software);
        QTest::qWait(200);
        const auto contrast = [&](const QImage& frame) {
            int low = 255, high = 0;
            for (int y = 28; y < std::min(180, int(panel->height()) - 12); ++y) {
                const auto p = panel->mapToScene(QPointF(5, y)) * frame.devicePixelRatio();
                const int v = frame.pixelColor(qRound(p.x()), qRound(p.y())).red();
                low = std::min(low, v);
                high = std::max(high, v);
            }
            return high - low;
        };
        const auto blurred = window->grabWindow();
        QVERIFY(capture(fillPanel ? "actual-fill-blurred.png" : "actual-size-blurred.png"));
        panel->setProperty("backdropSource", QVariant::fromValue<QQuickItem*>(nullptr));
        QTest::qWait(200);
        const auto plain = window->grabWindow();
        QVERIFY(capture(fillPanel ? "actual-fill-plain.png" : "actual-size-plain.png"));
        QVERIFY(contrast(plain) > 100);
        if (!software)
            QVERIFY(contrast(blurred) < contrast(plain) / 3);
        QVERIFY(!editor->dirty());
        QVERIFY(!editor->canUndo());
    }
    void glassBackdropRendering() {
        QQmlComponent component(engine.get());
        component.setData(R"(
            import QtQuick
            import OpenPaint
            Item {
                width: 512; height: 200; z: 100
                Component.onCompleted: {
                    Appearance.glassEnabled = true;
                    Appearance.surfaceOpacity = 40;
                    Appearance.themeMode = 1;
                }
                Item {
                    id: stripes
                    anchors.fill: parent
                    Repeater {
                        model: 64
                        Rectangle {
                            required property int index
                            x: index * 8; width: 8; height: 200
                            color: index % 2 ? "white" : "black"
                        }
                    }
                }
                GlassSurface {
                    objectName: "blurredFixture"
                    width: 256; height: 200
                    backdropSource: stripes
                    Rectangle { x: 60; y: 130; width: 80; height: 4; color: "black" }
                }
                GlassSurface { x: 256; width: 256; height: 200; backdropSource: null }
            }
        )",
                          QUrl("qrc:/glass-fixture.qml"));
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> fixture(component.create());
        QVERIFY(fixture);
        auto* item = qobject_cast<QQuickItem*>(fixture.get());
        QVERIFY(item);
        item->setParentItem(window->contentItem());
        auto* panel = fixture->findChild<QObject*>("blurredFixture");
        QVERIFY(panel);
        const bool software =
            window->rendererInterface()->graphicsApi() == QSGRendererInterface::Software;
        QTRY_COMPARE(panel->property("backdropBlurEnabled").toBool(), !software);
        QTest::qWait(100);
        QGuiApplication::sync();
        auto frame = window->grabWindow();
        const auto sample = [&](int x, int y) {
            // grabWindow metadata can round fractional DPR on the software backend.
            const auto scene = item->mapToScene(QPointF(x, y));
            const qreal scaleX = qreal(frame.width()) / window->width();
            const qreal scaleY = qreal(frame.height()) / window->height();
            return frame.pixelColor(qRound(scene.x() * scaleX), qRound(scene.y() * scaleY)).red();
        };
        const auto contrast = [&](int offset) {
            int low = 255, high = 0;
            for (int x = 64; x < 192; ++x) {
                const auto value = sample(x + offset, 90);
                low = std::min(low, value);
                high = std::max(high, value);
            }
            return high - low;
        };
        // The threaded renderer may still return the preceding frame initially.
        QTRY_VERIFY(([&] {
            frame = window->grabWindow();
            return contrast(256) > 100;
        })());
        if (!software)
            QVERIFY2(contrast(0) < contrast(256) / 3,
                     "Backdrop stripes must be blurred, not just tinted");
        QCOMPARE(sample(90, 132), 0); // Foreground content stays sharp and opaque.
        QVERIFY(capture("glass-backdrop-comparison.png"));
    }
    void topRightControls() {
        auto* settings = window->findChild<QObject*>("settingsButton");
        auto* updates = window->findChild<QObject*>("quickUpdatesButton");
        auto* menu = window->findChild<QObject*>("settingsMenu");
        QVERIFY(settings && updates && menu);
        QVERIFY(QMetaObject::invokeMethod(settings, "click"));
        QTRY_VERIFY(menu->property("visible").toBool());
        keyClick(Qt::Key_Escape);
        QTRY_VERIFY(!menu->property("visible").toBool());
        QVERIFY(QMetaObject::invokeMethod(updates, "click"));
        auto* dialog = window->findChild<QObject*>("updatesDialog");
        QVERIFY(dialog);
        QTRY_VERIFY(dialog->property("visible").toBool());
        keyClick(Qt::Key_Escape);
        QVERIFY(capture(QStringLiteral("completed-ribbon.png")));
    }
    void brushMainActivation() {
        auto* editor = window->findChild<Editor*>("editor");
        auto* menu = window->findChild<QObject*>("brushesMenu");
        auto* main = window->findChild<QObject*>("brushesButton");
        editor->brushStyle = 4;
        QVERIFY(QMetaObject::invokeMethod(main, "click"));
        QCOMPARE(editor->tool(), Editor::Tool::Brush);
        QCOMPARE(editor->brushStyle, 4);
        QVERIFY(!menu->property("visible").toBool());
        editor->setTool(Editor::Tool::Pencil);
        QCOMPARE(editor->tool(), Editor::Tool::Pencil);
        QCOMPARE(editor->brushStyle, 4);
    }
    void ribbonShapeKeyboard() {
        auto* editor = window->findChild<Editor*>(QStringLiteral("editor"));
        QVERIFY(tabTo("ribbonShape0"));
        for (int i = 0; i < 19; ++i)
            keyClick(Qt::Key_Right);
        QCOMPARE(window->activeFocusItem()->objectName(), QString("ribbonShape19"));
        keyClick(Qt::Key_Space);
        QCOMPARE(editor->shapeKind, 19);
        QCOMPARE(editor->tool(), Editor::Tool::Shape);
        const auto* item = window->activeFocusItem();
        const auto bounds = item->mapRectToScene(item->boundingRect());
        QVERIFY(bounds.top() >= 40 && bounds.bottom() <= 152);
        QVERIFY(!editor->dirty());
    }

    void shapeStyleMenus() {
        auto* editor = window->findChild<Editor*>("editor");
        for (const auto& kind : {QStringLiteral("Outline"), QStringLiteral("Fill")}) {
            QVERIFY(tabTo("shape" + kind + "Button"));
            keyClick(Qt::Key_Space);
            auto* menu = window->findChild<QObject*>("shape" + kind + "Menu");
            QTRY_VERIFY(menu->property("visible").toBool());
            auto* none = window->findChild<QObject*>("no" + kind + "Item");
            QVERIFY(QMetaObject::invokeMethod(none, "click"));
            QVERIFY(!editor->property(("shape" + kind).toUtf8()).toBool());
            QVERIFY(QMetaObject::invokeMethod(none, "click"));
            QVERIFY(none->property("checked").toBool());
            auto* solid = window->findChild<QObject*>("solid" + kind + "Item");
            QVERIFY(QMetaObject::invokeMethod(solid, "click"));
            QVERIFY(editor->property(("shape" + kind).toUtf8()).toBool());
            QVERIFY(QMetaObject::invokeMethod(solid, "click"));
            QVERIFY(solid->property("checked").toBool());
            QVERIFY(!none->property("checked").toBool());
            QMetaObject::invokeMethod(menu, "close");
        }
        QVERIFY(!editor->dirty());
    }
    void shapeOptionsKeyboard() {
        auto* editor = window->findChild<Editor*>(QStringLiteral("editor"));
        const auto original = editor->image();
        QVERIFY(tabTo("ribbonShape1"));
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
        QVERIFY(tabTo("brushMenuButton"));
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
        QVERIFY(!window->findChild<QObject*>("fillControlsSurface"));
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
    void keyboardToolsAndAccessibility() {
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
    }

    void keyboardControlsAndAccessibility() {
        auto* editor = window->findChild<Editor*>(QStringLiteral("editor"));
        QVERIFY(tabTo("pencilButton"));
        keyClick(Qt::Key_Space);
        QVERIFY(tabTo("strokeSize"));
        const auto previousSize = editor->brushSize;
        keyClick(Qt::Key_Up);
        QCOMPARE(editor->brushSize, previousSize + 1);
        QVERIFY(tabTo("fillButton"));
        keyClick(Qt::Key_Space);
        QCOMPARE(editor->tool(), Editor::Tool::Fill);
        QVERIFY(!window->findChild<QObject*>("fillTolerance"));
        QVERIFY(tabTo("pencilButton"));
        keyClick(Qt::Key_Space);
        QVERIFY(tabTo("editColorsButton"));
        keyClick(Qt::Key_Space);
        auto* colors = window->findChild<QObject*>(QStringLiteral("colorEditor"));
        QTRY_VERIFY(colors->property("visible").toBool());
        keyClick(Qt::Key_Escape);
        QTRY_VERIFY(!colors->property("visible").toBool());
        auto* canvas = window->findChild<QQuickItem*>(QStringLiteral("canvas"));
        auto* accessible = QAccessible::queryAccessibleInterface(canvas);
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
        keyClick(Qt::Key_A, Qt::ControlModifier);
        for (const char key : std::string("12"))
            keyClick(key);
        QVERIFY(!accept->isEnabled());
        for (const char key : std::string("AB34"))
            keyClick(key);
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
        keyClick(Qt::Key_End);
        QCOMPARE(selection->value(), 100.0);
        keyClick(Qt::Key_Home);
        QCOMPARE(selection->color(), QColor(Qt::black));
        keyClick(Qt::Key_End);
        const auto beforeMode = selection->color();
        auto* mode = window->findChild<QQuickItem*>(QStringLiteral("colorMode"));
        QVERIFY(mode);
        click(mode);
        auto* modePopup = qvariant_cast<QObject*>(mode->property("popup"));
        QVERIFY(modePopup);
        QTRY_VERIFY(modePopup->property("visible").toBool());
        QVERIFY(capture(QStringLiteral("color-model-menu-dark.png")));
        keyClick(Qt::Key_Down);
        keyClick(Qt::Key_Return);
        QTRY_VERIFY(!modePopup->property("visible").toBool());
        QCOMPARE(mode->property("currentIndex").toInt(), 1);
        QCOMPARE(selection->color(), beforeMode);
        const auto channels = visualItems(window->contentItem(), QStringLiteral("channelInput0"));
        QCOMPARE(channels.size(), 1);
        auto* numericText = qvariant_cast<QQuickItem*>(channels.first()->property("contentItem"));
        QVERIFY(numericText);
        numericText->forceActiveFocus();
        keyClick(Qt::Key_A, Qt::ControlModifier);
        for (const char key : std::string("120"))
            keyClick(key);
        keyClick(Qt::Key_Return);
        QCOMPARE(selection->hue(), 120.0);
        const auto basic = visualItems(window->contentItem(), QStringLiteral("basicDialogSwatch"));
        QCOMPARE(basic.size(), 48);
        click(basic.last());
        QCOMPARE(selection->color(), QColor(Qt::white));
        click(dialogSlots.last());
        QCOMPARE(palette->colors(), QStringList{"#ffffff"});
        keyClick(Qt::Key_Escape);
        QTRY_VERIFY(!dialog->property("visible").toBool());
        QCOMPARE(editor->primary, original);
        click(secondary);
        QVERIFY(editor->editingSecondary());
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
        click(window->findChild<QQuickItem*>(QStringLiteral("editColorsButton")));
        QTRY_VERIFY(dialog->property("visible").toBool());
        QVERIFY(capture(QStringLiteral("color-editor-full-dark-1024.png")));
        for (auto* slot : dialogSlots)
            QVERIFY(!slot->property("empty").toBool());
        QVERIFY(!add->isEnabled());
        keyClick(Qt::Key_Escape);
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
        QVERIFY(editor->editingSecondary());
        QCOMPARE(editor->primary, QColor(Qt::black));
        QCOMPARE(editor->secondary, QColor(Qt::white));
        QVERIFY(!dialog->property("visible").toBool());
        editor->setTool(Editor::Tool::Text);
        editor->begin({20, 20}, false);
        QCOMPARE(editor->textColor(), QColor(Qt::black));
        QCOMPARE(editor->textBackgroundColor(), QColor(Qt::white));
        editor->cancel();
        editor->setTool(Editor::Tool::Pencil);
        const auto toolbarSlots = visualItems(window->contentItem(), "customToolbarSwatch");
        QVERIFY(tabToItem(toolbarSlots.at(7)));
        keyClick(Qt::Key_Space);
        QTRY_VERIFY(dialog->property("visible").toBool());
        QVERIFY(draft->setHex("#123456"));
        keyClick(Qt::Key_Escape);
        QCOMPARE(palette->toolbarColors(), QStringList(10, QString()));
        QVERIFY(palette->colors().isEmpty());
        QVERIFY(tabToItem(toolbarSlots.at(7)));
        keyClick(Qt::Key_Space);
        QTRY_VERIFY(dialog->property("visible").toBool());
        QVERIFY(draft->setHex("#123456"));
        QVERIFY(tabTo("acceptColor"));
        keyClick(Qt::Key_Space);
        QTRY_VERIFY(!dialog->property("visible").toBool());
        QCOMPARE(palette->toolbarColors().first(), QString("#123456"));
        QCOMPARE(palette->toolbarColors().at(7), QString());
        QCOMPARE(palette->colors(), QStringList{"#123456"});
        QCOMPARE(editor->secondary, QColor("#123456"));
        QCOMPARE(editor->primary, QColor(Qt::black));
        QVERIFY(tabToItem(toolbarSlots.first()));
        keyClick(Qt::Key_F10, Qt::ShiftModifier);
        keyClick(Qt::Key_Down);
        keyClick(Qt::Key_Return);
        QTRY_VERIFY(dialog->property("visible").toBool());
        QVERIFY(draft->setHex("#654321"));
        QVERIFY(tabTo("acceptColor"));
        keyClick(Qt::Key_Space);
        QTRY_VERIFY(!dialog->property("visible").toBool());
        QCOMPARE(editor->secondary, QColor("#654321"));
        QCOMPARE(palette->toolbarColors().first(), QString("#654321"));
        QCOMPARE(palette->colors().last(), QString("#654321"));
        QVERIFY(tabTo("primaryButton"));
        keyClick(Qt::Key_Space);
        QCOMPARE(editor->secondary, QColor("#654321"));
        QCOMPARE(editor->primary, QColor(Qt::black));
        QVERIFY(tabToItem(toolbarSlots.first()));
        keyClick(Qt::Key_Space);
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
            if (!target)
                return false;
            for (auto* item = target->activeFocusItem(); item; item = item->parentItem()) {
                if (matches(item))
                    return true;
            }
            // keyWindow already synchronized and activated this target above.
            // Do not repeat its native-window wait for the same Tab event.
            QTest::keyClick(target, Qt::Key_Tab);
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
    QQuickWindow::setDefaultAlphaBuffer(true);
    QCoreApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
    QGuiApplication app(argc, argv);
    app.styleHints()->setTabFocusBehavior(Qt::TabFocusAllControls);
    QQuickStyle::setStyle(QStringLiteral("Basic"));
    ShellTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "tst_shell.moc"
