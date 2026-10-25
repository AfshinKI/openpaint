// SPDX-License-Identifier: GPL-3.0-or-later
#include "render/CanvasItem.h"
#include <QFocusEvent>
#include <QHoverEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>
#include <QtTest>
#include <limits>

class TestEditor : public Editor {
  public:
    bool active() const {
        return document.active();
    }
};
class InputCanvas : public CanvasItem {
  public:
    InputCanvas() {
        setSize({1200, 696});
    }
    void mouse(QEvent::Type type, QPointF point, Qt::MouseButton button, Qt::MouseButtons buttons,
               Qt::KeyboardModifiers modifiers = Qt::NoModifier) {
        QMouseEvent event(type, point, point, button, buttons, modifiers);
        if (type == QEvent::MouseButtonPress)
            mousePressEvent(&event);
        else if (type == QEvent::MouseButtonRelease)
            mouseReleaseEvent(&event);
        else
            mouseMoveEvent(&event);
    }
    void press(QPointF point, Qt::MouseButton button = Qt::LeftButton) {
        mouse(QEvent::MouseButtonPress, fromDocument(point), button, button);
    }
    void move(QPointF point, Qt::MouseButtons buttons = Qt::LeftButton) {
        mouse(QEvent::MouseMove, fromDocument(point), Qt::NoButton, buttons);
    }
    void release(QPointF point, Qt::MouseButton button = Qt::LeftButton) {
        mouse(QEvent::MouseButtonRelease, fromDocument(point), button, Qt::NoButton);
    }
    void key(int key, bool press = true) {
        QKeyEvent event(press ? QEvent::KeyPress : QEvent::KeyRelease, key, Qt::NoModifier);
        if (press)
            keyPressEvent(&event);
        else
            keyReleaseEvent(&event);
    }
    void scroll() {
        QWheelEvent event({100, 100}, {100, 100}, {0, 30}, {}, Qt::NoButton, Qt::NoModifier,
                          Qt::ScrollUpdate, false);
        wheelEvent(&event);
    }
    void hover(QPointF point) {
        const auto local = fromDocument(point);
        QHoverEvent event(QEvent::HoverMove, local, local, local);
        hoverMoveEvent(&event);
    }
    void leave() {
        QHoverEvent event(QEvent::HoverLeave, QPointF(), QPointF(), QPointF());
        hoverLeaveEvent(&event);
    }
    void loseFocus() {
        QFocusEvent event(QEvent::FocusOut);
        focusOutEvent(&event);
    }
    void loseGrab() {
        mouseUngrabEvent();
    }
};
class CanvasTest : public QObject {
    Q_OBJECT
  private slots:
    void fitAvoidsToolInsetWithoutConstrainingZoom() {
        TestEditor editor;
        InputCanvas canvas;
        canvas.setEditor(&editor);
        const auto pixels = editor.image();
        canvas.setFitLeftInset(48);
        canvas.fit();
        QVERIFY(canvas.origin().x() >= 72);
        QVERIFY(canvas.fromDocument({qreal(editor.imageSize().width()), 0}).x() <=
                canvas.width() - 24);
        canvas.zoomAt(4, canvas.fromDocument({200, 200}));
        QVERIFY(canvas.origin().x() < 72); // Free zoom remains unrestricted.
        canvas.fit();
        QVERIFY(canvas.origin().x() >= 72);
        QCOMPARE(editor.image(), pixels);
    }
    void regionZoomIsViewOnly() {
        TestEditor editor;
        InputCanvas canvas;
        canvas.setEditor(&editor);
        editor.setTool(Editor::Tool::Zoom);
        const auto original = editor.image();
        const auto dirty = editor.dirty();
        const auto before = canvas.zoom();
        canvas.press({100, 100});
        canvas.move({200, 180});
        canvas.release({200, 180});
        QVERIFY(canvas.zoom() > before);
        const auto center = canvas.fromDocument({150, 140});
        QVERIFY(qAbs(center.y() - canvas.height() / 2) < 0.01);
        QVERIFY(canvas.fromDocument({100, 100}).x() >= 24);
        QVERIFY(canvas.fromDocument({200, 180}).x() <= canvas.width() - 24);
        const auto fittedZoom = canvas.zoom();
        canvas.press({120, 120});
        canvas.move({160, 150});
        canvas.key(Qt::Key_Escape);
        canvas.release({160, 150});
        QCOMPARE(canvas.zoom(), fittedZoom);
        canvas.press({120, 120});
        canvas.release({120, 120});
        QCOMPARE(canvas.zoom(), fittedZoom);
        canvas.press({160, 150});
        canvas.move({120, 120});
        canvas.loseFocus();
        canvas.release({120, 120});
        QCOMPARE(canvas.zoom(), fittedZoom);
        canvas.press({160, 150});
        canvas.release({120, 120});
        QVERIFY(canvas.zoom() > fittedZoom);
        QCOMPARE(editor.image(), original);
        QCOMPARE(editor.dirty(), dirty);
        QVERIFY(!editor.canUndo());
    }
    void smartGuideZoomAndBypass() {
        for (qreal zoom : {0.5, 1.0, 2.0}) {
            TestEditor editor;
            InputCanvas canvas;
            canvas.setEditor(&editor);
            canvas.zoomBy(zoom / canvas.zoom());
            editor.setSmartGuides(true);
            QImage patch(20, 20, QImage::Format_ARGB32_Premultiplied);
            patch.fill(Qt::red);
            editor.pasteImage(patch);
            canvas.press({10, 10});
            const QPointF near(10 + 4 / zoom, 110);
            canvas.move(near);
            QCOMPARE(editor.floatingPosition().x(), 0);
            QCOMPARE(editor.guidePosition().x(), 0.0);
            canvas.mouse(QEvent::MouseMove, canvas.fromDocument(near), Qt::NoButton, Qt::LeftButton,
                         Qt::ControlModifier);
            QCOMPARE(editor.floatingPosition().x(), int(4 / zoom));
            QCOMPARE(editor.guidePosition(), QPointF(-1, -1));
            canvas.move({10 + 8 / zoom, 110});
            QCOMPARE(editor.floatingPosition().x(), int(8 / zoom));
            canvas.release(near);
            QCOMPARE(editor.guidePosition(), QPointF(-1, -1));
            editor.cancel();
        }
    }
    void canvasDoesNotDrawOutsideArtwork() {
        TestEditor editor;
        InputCanvas canvas;
        canvas.setEditor(&editor);
        QImage rendered(1200, 696, QImage::Format_ARGB32_Premultiplied);
        rendered.fill(Qt::transparent);
        QPainter painter(&rendered);
        canvas.paint(&painter);
        painter.end();
        const auto origin = canvas.origin().toPoint();
        QCOMPARE(rendered.pixelColor(origin.x() - 1, origin.y() + 30).alpha(), 0);
        QCOMPARE(rendered.pixelColor(origin.x() + 30, origin.y() - 1).alpha(), 0);
        QCOMPARE(rendered.pixelColor(origin.x() + 30, origin.y() + 30), QColor(Qt::white));
    }
    void outsideClickDismissesWithoutNewGesture() {
        TestEditor editor;
        InputCanvas canvas;
        canvas.setEditor(&editor);
        const auto original = editor.image();
        editor.setTool(Editor::Tool::Select);
        editor.selectRect(20, 20, 40, 40);
        canvas.press({100, 100});
        canvas.move({200, 200});
        canvas.release({200, 200});
        QVERIFY(!editor.hasSelection());
        QCOMPARE(editor.image(), original);
        QVERIFY(!editor.canUndo());
        canvas.press({100, 100});
        canvas.release({200, 200});
        QVERIFY(editor.hasSelection());
        canvas.press({-10, -10});
        canvas.release({-10, -10});
        QVERIFY(!editor.hasSelection());
        editor.setTool(Editor::Tool::Shape);
        editor.shapeKind = 1;
        canvas.press({20, 20});
        canvas.release({60, 60});
        const auto shape = editor.image();
        canvas.press({100, 100});
        canvas.release({200, 200});
        QVERIFY(!editor.previewActive());
        QCOMPARE(editor.image(), shape);
        editor.undo();
        QCOMPARE(editor.image(), original);
        editor.redo();
        QCOMPARE(editor.image(), shape);
    }

    void pickerHoverPreview() {
        TestEditor editor;
        InputCanvas canvas;
        canvas.setEditor(&editor);
        editor.setTool(Editor::Tool::ColorPicker);
        const auto original = editor.image();
        const auto primary = editor.primary;
        auto render = [&] {
            QImage result(1200, 696, QImage::Format_ARGB32_Premultiplied);
            result.fill(Qt::transparent);
            QPainter painter(&result);
            canvas.paint(&painter);
            return result;
        };
        const auto base = render();
        canvas.hover({100, 100});
        QVERIFY(render() != base);
        QCOMPARE(editor.image(), original);
        QCOMPARE(editor.primary, primary);
        QVERIFY(!editor.canUndo());
        canvas.hover({-10, -10});
        QCOMPARE(render(), base);
        canvas.hover({100, 100});
        canvas.leave();
        QCOMPARE(render(), base);
        canvas.hover({100, 100});
        editor.setTool(Editor::Tool::Pencil);
        QCOMPARE(render(), base);
    }

    void scrollNotifiesInlineOverlay() {
        TestEditor editor;
        InputCanvas canvas;
        canvas.setEditor(&editor);
        QSignalSpy changed(&canvas, &CanvasItem::viewportChanged);
        const auto origin = canvas.origin();
        canvas.scroll();
        QCOMPARE(changed.size(), 1);
        QCOMPARE(canvas.origin(), origin + QPointF(0, 30));
    }
    void shapeShiftInputAtZoom() {
        TestEditor editor;
        InputCanvas canvas;
        canvas.setEditor(&editor);
        canvas.zoomAt(1.75, {400, 300});
        editor.setTool(Editor::Tool::Shape);
        editor.shapeKind = 2;
        canvas.press({100, 100});
        canvas.mouse(QEvent::MouseButtonRelease, canvas.fromDocument({150, 130}), Qt::LeftButton,
                     Qt::NoButton, Qt::ShiftModifier);
        QVERIFY(QLineF(editor.shapeEnd(), QPointF(150, 150)).length() < 1e-8);
        QVERIFY(editor.previewActive());
        canvas.key(Qt::Key_Escape);
        QVERIFY(!editor.dirty());
    }

    void retainedShapePreview() {
        TestEditor editor;
        InputCanvas canvas;
        canvas.setEditor(&editor);
        editor.setTool(Editor::Tool::Shape);
        editor.shapeKind = 1;
        const auto original = editor.image();
        canvas.press({100, 100});
        canvas.release({200, 200});
        QVERIFY(editor.previewActive());
        QVERIFY(editor.active());
        QVERIFY(editor.canUndo()); // Undo cancels the retained preview.
        canvas.loseFocus();
        canvas.zoomAt(1.5, {300, 300});
        QVERIFY(editor.previewActive());
        editor.setProperty("secondary", QColor(Qt::red));
        editor.setProperty("shapeFill", true);
        QCOMPARE(editor.image().pixelColor(150, 150), QColor(Qt::red));
        const auto preview = editor.image();
        canvas.mouse(QEvent::MouseButtonPress, {400, 400}, Qt::RightButton, Qt::RightButton);
        canvas.mouse(QEvent::MouseButtonRelease, {420, 430}, Qt::RightButton, Qt::NoButton);
        QVERIFY(editor.previewActive());
        QCOMPARE(editor.image(), preview);
        canvas.key(Qt::Key_Escape);
        QCOMPARE(editor.image(), original);
        QVERIFY(!editor.previewActive());
        canvas.press({100, 100});
        canvas.release({200, 200});
        const auto shape = editor.image();
        canvas.key(Qt::Key_Return);
        QVERIFY(!editor.previewActive());
        editor.undo();
        QCOMPARE(editor.image(), original);
        editor.redo();
        QCOMPARE(editor.image(), shape);
        canvas.press({300, 300});
        canvas.release({400, 400});
        QCOMPARE(editor.image().pixelColor(350, 350), QColor(Qt::red));
        editor.setTool(Editor::Tool::Pencil);
        QVERIFY(!editor.previewActive());
        editor.undo();
        QCOMPARE(editor.image(), shape);
    }

    void pickerGestureAndToolSwitch() {
        TestEditor editor;
        InputCanvas canvas;
        canvas.setEditor(&editor);
        editor.primary = Qt::red;
        editor.brushSize = 12;
        canvas.press({100, 100});
        canvas.move({200, 100});
        editor.setTool(Editor::Tool::ColorPicker);
        QVERIFY(!editor.active());
        QVERIFY(editor.canUndo());
        canvas.release({300, 100});
        const auto painted = editor.image();
        editor.setProperty("primary", QColor(Qt::black));
        canvas.zoomAt(2.5, {413.25, 312.75});
        canvas.press({100.75, 100.25});
        QCOMPARE(editor.primary, QColor(Qt::red));
        canvas.mouse(QEvent::MouseButtonPress, canvas.fromDocument({400, 400}), Qt::RightButton,
                     Qt::LeftButton | Qt::RightButton);
        QCOMPARE(editor.secondary, QColor(Qt::white));
        canvas.move({400, 400});
        canvas.release({400, 400});
        QCOMPARE(editor.primary, QColor(Qt::red));
        QCOMPARE(editor.image(), painted);
        canvas.press({100, 100}, Qt::RightButton);
        canvas.release({100, 100}, Qt::RightButton);
        QCOMPARE(editor.secondary, QColor(Qt::white));
        canvas.key(Qt::Key_Space);
        canvas.press({400, 400});
        canvas.release({450, 450});
        canvas.key(Qt::Key_Space, false);
        QCOMPARE(editor.primary, QColor(Qt::red));
        editor.undo();
        QVERIFY(!editor.canUndo());
        QVERIFY(!editor.dirty());
        editor.redo();
        QCOMPARE(editor.image(), painted);
        editor.setTool(Editor::Tool::Pencil);
        canvas.press({400, 400});
        canvas.release({400, 400});
        QCOMPARE(editor.image().pixelColor(400, 400), QColor(Qt::red));
    }
    void extraButtonsKeepOriginalGesture_data() {
        QTest::addColumn<int>("extra");
        QTest::newRow("right") << int(Qt::RightButton);
        QTest::newRow("middle") << int(Qt::MiddleButton);
    }
    void extraButtonsKeepOriginalGesture() {
        QFETCH(int, extra);
        TestEditor editor;
        InputCanvas canvas;
        canvas.setEditor(&editor);
        const auto initial = editor.image();
        const auto origin = canvas.fromDocument({0, 0});
        canvas.press({100, 100});
        canvas.move({150, 100});
        const auto button = Qt::MouseButton(extra);
        canvas.mouse(QEvent::MouseButtonPress, canvas.fromDocument({150, 100}), button,
                     Qt::LeftButton | button);
        canvas.mouse(QEvent::MouseButtonRelease, canvas.fromDocument({150, 100}), button,
                     Qt::LeftButton);
        QVERIFY(editor.active());
        canvas.move({250, 100});
        canvas.release({300, 100});
        QCOMPARE(editor.image().pixelColor(225, 100), QColor(Qt::black));
        QCOMPARE(canvas.fromDocument({0, 0}), origin);
        const auto painted = editor.image();
        editor.undo();
        QCOMPARE(editor.image(), initial);
        QVERIFY(!editor.canUndo());
        editor.redo();
        QCOMPARE(editor.image(), painted);
    }
    void interruptions_data() {
        QTest::addColumn<QString>("reason");
        for (const auto& reason : {"focus", "grab", "settings", "zoom", "resize", "lost-button"})
            QTest::newRow(reason) << QString(reason);
    }
    void interruptions() {
        QFETCH(QString, reason);
        TestEditor editor;
        InputCanvas canvas;
        canvas.setEditor(&editor);
        canvas.zoomAt(1.25, {400, 300});
        const auto initial = editor.image();
        canvas.press({100, 100});
        canvas.move({200, 100});
        const auto last = editor.image();
        if (reason == "focus")
            canvas.loseFocus();
        else if (reason == "grab")
            canvas.loseGrab();
        else if (reason == "settings")
            editor.setProperty("brushSize", 12);
        else if (reason == "zoom")
            canvas.zoomAt(1.5, {300, 200});
        else if (reason == "resize")
            canvas.setWidth(1100);
        else
            canvas.move({500, 100}, Qt::NoButton);
        QVERIFY(!editor.active());
        canvas.move({600, 100});
        canvas.release({700, 100});
        QCOMPARE(editor.image(), last);
        editor.undo();
        QCOMPARE(editor.image(), initial);
        QVERIFY(!editor.canUndo());
    }
    void editorReplacementFinishesOldStroke() {
        TestEditor first, second;
        InputCanvas canvas;
        canvas.setEditor(&first);
        canvas.press({100, 100});
        canvas.move({200, 100});
        const auto secondPixels = second.image();
        canvas.setEditor(&second);
        QVERIFY(!first.active());
        QVERIFY(first.canUndo());
        canvas.move({300, 100});
        canvas.release({400, 100});
        QCOMPARE(second.image(), secondPixels);
        QVERIFY(!second.active());
    }
    void panIncludesReleasePosition_data() {
        QTest::addColumn<bool>("space");
        QTest::addColumn<int>("panButton");
        QTest::newRow("middle") << false << int(Qt::MiddleButton);
        QTest::newRow("right") << false << int(Qt::RightButton);
        QTest::newRow("space-left") << true << int(Qt::LeftButton);
    }
    void panIncludesReleasePosition() {
        QFETCH(bool, space);
        QFETCH(int, panButton);
        TestEditor editor;
        InputCanvas canvas;
        canvas.setEditor(&editor);
        const auto original = editor.image();
        const auto origin = canvas.fromDocument({0, 0});
        if (space)
            canvas.key(Qt::Key_Space);
        const auto button = Qt::MouseButton(panButton);
        canvas.mouse(QEvent::MouseButtonPress, {200, 200}, button, button);
        QCOMPARE(canvas.cursor().shape(), Qt::ClosedHandCursor);
        canvas.mouse(QEvent::MouseMove, {240, 220}, Qt::NoButton, button);
        canvas.mouse(QEvent::MouseButtonRelease, {260, 250}, button, Qt::NoButton);
        QCOMPARE(canvas.fromDocument({0, 0}), origin + QPointF(60, 50));
        QVERIFY(canvas.cursor().shape() != Qt::ClosedHandCursor);
        QVERIFY(!editor.canUndo());
        QCOMPARE(editor.image(), original);
        QVERIFY(!editor.dirty());
    }
    void rightPanDoesNotUseToolAndReleasesCursor() {
        for (const auto tool : {Editor::Tool::Pencil, Editor::Tool::Fill, Editor::Tool::ColorPicker,
                                Editor::Tool::Text}) {
            TestEditor editor;
            InputCanvas canvas;
            canvas.setEditor(&editor);
            editor.setTool(tool);
            const auto original = editor.image();
            const auto secondary = editor.secondary;
            canvas.mouse(QEvent::MouseButtonPress, {200, 200}, Qt::RightButton, Qt::RightButton);
            QCOMPARE(canvas.cursor().shape(), Qt::ClosedHandCursor);
            canvas.mouse(QEvent::MouseMove, {240, 220}, Qt::NoButton, Qt::RightButton);
            const auto origin = canvas.fromDocument({0, 0});
            canvas.loseFocus();
            QVERIFY(canvas.cursor().shape() != Qt::ClosedHandCursor);
            canvas.mouse(QEvent::MouseMove, {300, 250}, Qt::NoButton, Qt::NoButton);
            QCOMPARE(canvas.fromDocument({0, 0}), origin);
            QCOMPARE(editor.image(), original);
            QCOMPARE(editor.secondary, secondary);
            QVERIFY(!editor.canUndo());
            QVERIFY(!editor.previewActive());
            canvas.mouse(QEvent::MouseButtonPress, {200, 200}, Qt::RightButton, Qt::RightButton);
            canvas.key(Qt::Key_Escape);
            QVERIFY(canvas.cursor().shape() != Qt::ClosedHandCursor);
        }
    }
    void clippingCancelAndCoordinateMapping() {
        TestEditor editor;
        InputCanvas canvas;
        canvas.setEditor(&editor);
        const auto original = editor.image();
        for (const double zoom : {0.5, 1.25, 2.0}) {
            canvas.zoomAt(zoom / canvas.zoom(), {321.5, 212.75});
            const QPointF point(127.25, 303.875);
            QVERIFY(QLineF(canvas.toDocument(canvas.fromDocument(point)), point).length() < 1e-8);
        }
        canvas.press({10, 100});
        canvas.move({-100, 100});
        canvas.move({200, 100});
        canvas.release({300, 100});
        QCOMPARE(editor.image().pixelColor(0, 100), QColor(Qt::black));
        QCOMPARE(editor.image().pixelColor(200, 100), QColor(Qt::black));
        editor.undo();
        QCOMPARE(editor.image(), original);
        editor.secondary = Qt::red;
        canvas.press({100, 200});
        canvas.move({200, 200});
        QVERIFY(editor.image() != original);
        canvas.key(Qt::Key_Escape);
        canvas.release({300, 200});
        QCOMPARE(editor.image(), original);
        QVERIFY(!editor.active());
    }
    void invalidAnchorDoesNotPoisonTransform() {
        TestEditor editor;
        InputCanvas canvas;
        canvas.setEditor(&editor);
        const auto before = canvas.fromDocument({100, 100});
        const auto zoom = canvas.zoom();
        canvas.zoomAt(2, {std::numeric_limits<double>::quiet_NaN(), 50});
        QCOMPARE(canvas.zoom(), zoom);
        QCOMPARE(canvas.fromDocument({100, 100}), before);
        canvas.zoomAt(2, {50, std::numeric_limits<double>::infinity()});
        QCOMPARE(canvas.fromDocument({100, 100}), before);
    }
    void fractionalStartBounds_data() {
        QTest::addColumn<QPointF>("point");
        QTest::addColumn<bool>("inside");
        QTest::newRow("left-outside") << QPointF(-0.25, 100) << false;
        QTest::newRow("top-outside") << QPointF(100, -0.25) << false;
        QTest::newRow("right-inside") << QPointF(1151.75, 100) << true;
        QTest::newRow("bottom-inside") << QPointF(100, 647.75) << true;
        QTest::newRow("right-outside") << QPointF(1152, 100) << false;
        QTest::newRow("bottom-outside") << QPointF(100, 648) << false;
    }
    void fractionalStartBounds() {
        QFETCH(QPointF, point);
        QFETCH(bool, inside);
        TestEditor editor;
        InputCanvas canvas;
        canvas.setEditor(&editor);
        const auto original = editor.image();
        canvas.press(point);
        QCOMPARE(editor.active(), inside);
        canvas.release(point);
        QCOMPARE(editor.dirty(), inside);
        if (!inside)
            QCOMPARE(editor.image(), original);
        else {
            editor.undo();
            QCOMPARE(editor.image(), original);
        }
    }
};
QTEST_MAIN(CanvasTest)
#include "tst_canvas.moc"
