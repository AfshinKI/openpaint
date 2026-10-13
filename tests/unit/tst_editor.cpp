// SPDX-License-Identifier: GPL-3.0-or-later
#include "app/Editor.h"
#include <QColorSpace>
#include <QTemporaryDir>
#include <QtTest>
#include <limits>
class SampleEditor : public Editor {
  public:
    void reset(const QImage& image) {
        document.reset(image);
    }
};
class EditorTest : public QObject {
    Q_OBJECT
  private slots:
    void fixedColorRoles() {
        Editor editor;
        const auto original = editor.image();
        editor.primary = QColor(200, 10, 20, 100);
        editor.secondary = Qt::blue;
        const auto foreground = editor.primary;
        editor.setEditingSecondary(true);
        QCOMPARE(editor.primary, foreground);
        QCOMPARE(editor.secondary, QColor(Qt::blue));
        QCOMPARE(editor.selectedColor(), QColor(Qt::blue));
        QVERIFY(!editor.dirty());
        editor.setSelectedColor(Qt::green);
        QCOMPARE(editor.secondary, QColor(Qt::green));
        QCOMPARE(editor.primary, foreground);
        editor.setEditingSecondary(false);
        editor.setSelectedColor(Qt::red);
        editor.setEditingSecondary(true);
        editor.begin({40, 40}, false);
        editor.finish();
        QCOMPARE(editor.image().pixelColor(40, 40), QColor(Qt::red));
        editor.undo();
        editor.begin({40, 40}, true);
        editor.finish();
        QCOMPARE(editor.image().pixelColor(40, 40), QColor(Qt::green));
        editor.undo();
        editor.setTool(Editor::Tool::Fill);
        editor.begin({50, 50}, false);
        QTRY_VERIFY(!editor.busy());
        QCOMPARE(editor.image().pixelColor(50, 50), QColor(Qt::red));
        editor.undo();
        editor.setTool(Editor::Tool::Shape);
        editor.shapeKind = 1;
        editor.shapeFill = true;
        editor.begin({20, 20}, false);
        editor.extend({80, 80});
        QCOMPARE(editor.image().pixelColor(50, 50), QColor(Qt::green));
        editor.cancel();
        QCOMPARE(editor.image(), original);
    }
    void rectangularSelectionLifecycle() {
        Editor editor;
        const auto base = editor.image();
        editor.setTool(Editor::Tool::Select);
        editor.begin({50, 50}, false);
        editor.extend({20, 30});
        editor.endGesture();
        QCOMPARE(editor.selectionRect(), QRect(20, 30, 31, 21));
        QVERIFY(!editor.dirty());
        QVERIFY(!editor.canUndo());
        editor.begin({80, 80}, false);
        editor.extend({100, 100});
        editor.cancel();
        QCOMPARE(editor.selectionRect(), QRect(20, 30, 31, 21));
        editor.selectRect(-1, 0, 20, 20);
        QCOMPARE(editor.selectionRect(), QRect(20, 30, 31, 21));
        editor.setTool(Editor::Tool::Fill);
        editor.primary = Qt::red;
        editor.begin({30, 40}, false);
        QVERIFY(editor.filling());
        editor.deselect();
        QVERIFY(editor.hasSelection());
        QTRY_VERIFY(!editor.busy());
        QCOMPARE(editor.image().pixelColor(30, 40), QColor(Qt::red));
        QCOMPARE(editor.image().pixelColor(10, 40), QColor(Qt::white));
        editor.undo();
        QCOMPARE(editor.image(), base);
        editor.setTool(Editor::Tool::Shape);
        editor.shapeKind = 1;
        editor.shapeFill = true;
        editor.begin({0, 0}, false);
        editor.extend({80, 80});
        const auto bounds = editor.selectionRect();
        editor.selectAll();
        QCOMPARE(editor.selectionRect(), bounds);
        editor.cancel();
        QCOMPARE(editor.image(), base);
        QVERIFY(editor.hasSelection());
        editor.deleteSelection();
        QCOMPARE(editor.image().pixelColor(30, 40).alpha(), 0);
        editor.undo();
        QCOMPARE(editor.image(), base);
        editor.deselect();
        QVERIFY(!editor.hasSelection());
        editor.selectAll();
        QCOMPARE(editor.selectionRect(), base.rect());
        editor.newDocument();
        QVERIFY(!editor.hasSelection());
        QVERIFY(!editor.dirty());
    }
    void polygonAndConstraints() {
        Editor editor;
        const auto original = editor.image();
        editor.setTool(Editor::Tool::Shape);
        editor.shapeKind = 15;
        editor.begin({20, 20}, false);
        editor.extend({100, 20});
        editor.endGesture();
        editor.finish();
        QCOMPARE(editor.image(), original);
        QVERIFY(!editor.canUndo());
        editor.begin({20, 20}, false);
        editor.extend({100, 20});
        editor.endGesture();
        editor.begin({100, 100}, false);
        editor.endGesture();
        const auto triangle = editor.image();
        editor.begin({20, 100}, false);
        editor.endGesture();
        QVERIFY(editor.image() != triangle);
        editor.removeShapePoint();
        QCOMPARE(editor.image(), triangle);
        editor.finish();
        editor.undo();
        QCOMPARE(editor.image(), original);
        editor.redo();
        QCOMPARE(editor.image(), triangle);
        editor.shapeKind = 1;
        editor.begin({200, 200}, false);
        editor.extend({230, 220}, true);
        QCOMPARE(editor.shapeEnd(), QPointF(230, 230));
        editor.undo();
        QCOMPARE(editor.image(), triangle); // Cancels only the draft.
        editor.shapeKind = 0;
        editor.begin({200, 200}, false);
        editor.extend({230, 204}, true);
        QCOMPARE(editor.shapeEnd().y(), 200.0);
        editor.cancel();
    }

    void shapeSaveCommitsPreview() {
        Editor editor;
        editor.setTool(Editor::Tool::Shape);
        editor.shapeKind = 2;
        editor.begin({20, 20}, false);
        editor.extend({150, 100});
        editor.endGesture();
        QVERIFY(editor.previewActive());
        editor.setShapeGeometry(30, 30, 160, 110);
        const auto candidate = editor.image();
        QTemporaryDir dir;
        const auto url = QUrl::fromLocalFile(dir.filePath("shape.png"));
        editor.saveFile(url);
        QTRY_VERIFY(!editor.busy());
        QVERIFY(!editor.previewActive());
        QVERIFY(!editor.dirty());
        QCOMPARE(openpaint::loadPng(url.toLocalFile()).image, candidate);
        editor.undo();
        QVERIFY(editor.dirty());
        editor.redo();
        QVERIFY(!editor.dirty());
    }

    void secondaryBrushOpacityAndSave() {
        Editor editor;
        editor.setTool(Editor::Tool::Brush);
        editor.brushSize = 18;
        editor.opacity = 50;
        editor.secondary = Qt::blue;
        const auto original = editor.image();
        editor.begin({50, 50}, true);
        QCOMPARE(editor.image().pixelColor(50, 50), QColor(127, 127, 255));
        editor.cancel();
        QCOMPARE(editor.image(), original);
        editor.begin({50, 50}, true);
        editor.extend({100, 50});
        editor.finish();
        const auto painted = editor.image();
        editor.undo();
        QCOMPARE(editor.image(), original);
        editor.redo();
        QCOMPARE(editor.image(), painted);
        QTemporaryDir dir;
        const auto url = QUrl::fromLocalFile(dir.filePath("brush.png"));
        editor.saveFile(url);
        QTRY_VERIFY(!editor.busy());
        QCOMPARE(openpaint::loadPng(url.toLocalFile()).image, painted);
    }

    void fillCancelAndSave() {
        Editor editor;
        editor.setTool(Editor::Tool::Fill);
        const auto original = editor.image();
        editor.begin({0, 0}, false);
        QVERIFY(editor.filling());
        editor.cancel();
        QTRY_VERIFY(!editor.busy());
        QCOMPARE(editor.image(), original);
        QVERIFY(!editor.dirty());
        editor.begin({0, 0}, false);
        editor.setTool(Editor::Tool::Pencil);
        QTRY_VERIFY(!editor.busy());
        QCOMPARE(editor.image(), original);
        editor.setTool(Editor::Tool::Fill);
        editor.secondary = Qt::red;
        editor.begin({0, 0}, true);
        QTRY_VERIFY(!editor.busy());
        QCOMPARE(editor.image().pixelColor(500, 500), QColor(Qt::red));
        const auto filled = editor.image();
        editor.undo();
        QCOMPARE(editor.image(), original);
        editor.redo();
        QCOMPARE(editor.image(), filled);
        QTemporaryDir dir;
        const auto url = QUrl::fromLocalFile(dir.filePath("fill.png"));
        editor.saveFile(url);
        QTRY_VERIFY(!editor.busy());
        QCOMPARE(openpaint::loadPng(url.toLocalFile()).image, filled);
        QVERIFY(!editor.dirty());
    }

    void pickerSamplesWithoutEditing() {
        SampleEditor editor;
        QImage fixture(3, 2, QImage::Format_ARGB32_Premultiplied);
        fixture.fill(Qt::white);
        fixture.setColorSpace(QColorSpace::SRgb);
        fixture.setPixelColor(0, 0, QColor(40, 80, 120, 128));
        fixture.setPixelColor(2, 1, Qt::transparent);
        editor.reset(fixture);
        editor.begin({1, 1}, false);
        editor.finish();
        editor.undo();
        QVERIFY(editor.canRedo());
        editor.setTool(Editor::Tool::ColorPicker);
        editor.begin({0.75, 0.75}, false);
        QCOMPARE(editor.primary, fixture.pixelColor(0, 0));
        editor.begin({2.99, 1.99}, true);
        QCOMPARE(editor.secondary, QColor(Qt::transparent));
        editor.extend({1, 1});
        editor.finish();
        editor.cancel();
        QCOMPARE(editor.image(), fixture);
        QVERIFY(!editor.dirty());
        QVERIFY(!editor.canUndo());
        QVERIFY(editor.canRedo());
        const auto primary = editor.primary;
        for (const auto point : {QPointF(-0.01, 0), QPointF(3, 0), QPointF(0, 2),
                                 QPointF(std::numeric_limits<double>::quiet_NaN(), 0),
                                 QPointF(0, std::numeric_limits<double>::infinity())})
            editor.begin(point, false);
        QCOMPARE(editor.primary, primary);
        editor.redo();
        QVERIFY(editor.dirty());
        const auto changed = editor.image();
        editor.begin({0, 0}, false);
        QCOMPARE(editor.image(), changed);
        QVERIFY(editor.dirty());
        editor.undo();
        QCOMPARE(editor.image(), fixture);
    }
    void pickerDuringLoadIsIgnored() {
        QTemporaryDir dir;
        const auto url = QUrl::fromLocalFile(dir.filePath("sample.png"));
        Editor editor;
        QVERIFY(openpaint::savePng(url.toLocalFile(), editor.image()).ok());
        editor.setTool(Editor::Tool::ColorPicker);
        editor.openFile(url);
        QVERIFY(editor.loading());
        editor.begin({0, 0}, false);
        QCOMPARE(editor.primary, QColor(Qt::black));
        QTRY_VERIFY(!editor.busy());
        editor.begin({0, 0}, false);
        QCOMPARE(editor.primary, QColor(Qt::white));
        QVERIFY(!editor.dirty());
    }
    void snapshotSaveAndReopen() {
        QTemporaryDir dir;
        const auto url = QUrl::fromLocalFile(dir.filePath("drawing.png"));
        Editor editor;
        editor.begin({50, 50}, false);
        editor.extend({100, 100});
        editor.finish();
        const auto snapshot = editor.image();
        QSignalSpy saved(&editor, &Editor::saved);
        editor.saveFile(url);
        QVERIFY(editor.busy());
        editor.begin({200, 200}, false);
        editor.finish();
        QTRY_VERIFY(!editor.busy());
        QCOMPARE(saved.size(), 1);
        QVERIFY(saved.first().first().toBool());
        QVERIFY(editor.dirty());
        QCOMPARE(openpaint::loadPng(url.toLocalFile()).image, snapshot);
        editor.undo();
        QVERIFY(!editor.dirty());
        editor.redo();
        const auto final = editor.image();
        editor.saveFile(url);
        QTRY_VERIFY(!editor.busy());
        QVERIFY(!editor.dirty());
        editor.newDocument();
        QVERIFY(editor.fileUrl().isEmpty());
        editor.openFile(url);
        QVERIFY(editor.loading());
        editor.begin({10, 10}, false); // Opening freezes edits until replacement completes.
        QTRY_VERIFY(!editor.busy());
        QCOMPARE(editor.image(), final);
        QVERIFY(!editor.dirty());
        QVERIFY(!editor.canUndo());
    }
    void failedOperationsPreserveDocument() {
        QTemporaryDir dir;
        Editor editor;
        editor.begin({20, 20}, false);
        editor.finish();
        const auto original = editor.image();
        QSignalSpy errors(&editor, &Editor::errorChanged);
        editor.saveFile(QUrl::fromLocalFile(dir.filePath("missing/drawing.png")));
        QTRY_VERIFY(!editor.busy());
        QVERIFY(editor.dirty());
        QVERIFY(editor.fileUrl().isEmpty());
        QCOMPARE(editor.image(), original);
        QCOMPARE(errors.size(), 1);
        editor.openFile(QUrl::fromLocalFile(dir.filePath("missing.png")));
        QTRY_VERIFY(!editor.busy());
        QCOMPARE(editor.image(), original);
        QVERIFY(editor.dirty());
        QCOMPARE(errors.size(), 2);
        editor.openFile(QUrl("https://example.com/image.png"));
        QVERIFY(!editor.busy());
        QCOMPARE(errors.size(), 3);
    }
};
QTEST_GUILESS_MAIN(EditorTest)
#include "tst_editor.moc"
