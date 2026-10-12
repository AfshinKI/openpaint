// SPDX-License-Identifier: GPL-3.0-or-later
#include "render/CanvasItem.h"
#include <QFocusEvent>
#include <QKeyEvent>
#include <QMouseEvent>
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
    void mouse(QEvent::Type type, QPointF point, Qt::MouseButton button, Qt::MouseButtons buttons) {
        QMouseEvent event(type, point, point, button, buttons, Qt::NoModifier);
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
        QVERIFY(!editor.canUndo());
        canvas.loseFocus();
        canvas.zoomAt(1.5, {300, 300});
        QVERIFY(editor.previewActive());
        editor.setProperty("secondary", QColor(Qt::red));
        editor.setProperty("shapeFill", true);
        QCOMPARE(editor.image().pixelColor(150, 150), QColor(Qt::red));
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
        canvas.press({300, 300}, Qt::RightButton);
        canvas.release({400, 400}, Qt::RightButton);
        QCOMPARE(editor.image().pixelColor(350, 350), QColor(Qt::black));
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
        QCOMPARE(editor.secondary, QColor(Qt::red));
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
        QTest::newRow("middle") << false;
        QTest::newRow("space-left") << true;
    }
    void panIncludesReleasePosition() {
        QFETCH(bool, space);
        TestEditor editor;
        InputCanvas canvas;
        canvas.setEditor(&editor);
        const auto original = editor.image();
        const auto origin = canvas.fromDocument({0, 0});
        if (space)
            canvas.key(Qt::Key_Space);
        const auto button = space ? Qt::LeftButton : Qt::MiddleButton;
        canvas.mouse(QEvent::MouseButtonPress, {200, 200}, button, button);
        canvas.mouse(QEvent::MouseMove, {240, 220}, Qt::NoButton, button);
        canvas.mouse(QEvent::MouseButtonRelease, {260, 250}, button, Qt::NoButton);
        QCOMPARE(canvas.fromDocument({0, 0}), origin + QPointF(60, 50));
        QCOMPARE(editor.image(), original);
        QVERIFY(!editor.dirty());
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
        canvas.press({100, 200}, Qt::RightButton);
        canvas.move({200, 200}, Qt::RightButton);
        QVERIFY(editor.image() != original);
        canvas.key(Qt::Key_Escape);
        canvas.release({300, 200}, Qt::RightButton);
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
