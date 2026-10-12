// SPDX-License-Identifier: GPL-3.0-or-later
#include "app/Editor.h"
#include "tools/Transform.h"
#include <QColorSpace>
#include <QTemporaryDir>
#include <QtTest>
class TransformEditor : public Editor {
  public:
    void reset(const QImage& image) {
        document.reset(image);
    }
    void select(const QRegion& region) {
        document.setSelection(region);
    }
};
class TransformTest : public QObject {
    Q_OBJECT
    QImage fixture() {
        QImage image(6, 4, QImage::Format_ARGB32_Premultiplied);
        image.setColorSpace(QColorSpace::SRgb);
        for (int y = 0; y < image.height(); ++y)
            for (int x = 0; x < image.width(); ++x)
                image.setPixelColor(x, y, QColor(x * 35, y * 60, 100, 80 + x * 25));
        return image;
    }
  private slots:
    void exactMappingsAndMasks() {
        const auto source = fixture();
        const auto token = std::make_shared<std::atomic_bool>(false);
        const QRegion mask(QRect(1, 1, 3, 2));
        for (int kind = 0; kind < 5; ++kind) {
            const auto result =
                openpaint::transformImage(source, mask, openpaint::TransformKind(kind), token);
            QVERIFY(result.error.isEmpty());
            QVERIFY(!result.cancelled);
            QCOMPARE(result.image.size(), kind < 2 ? QSize(4, 6) : source.size());
            for (int y = 0; y < source.height(); ++y) {
                for (int x = 0; x < source.width(); ++x) {
                    const QPoint destination = kind == 0   ? QPoint(3 - y, x)
                                               : kind == 1 ? QPoint(y, 5 - x)
                                               : kind == 2 ? QPoint(5 - x, 3 - y)
                                               : kind == 3 ? QPoint(5 - x, y)
                                                           : QPoint(x, 3 - y);
                    QCOMPARE(result.image.pixel(destination), source.pixel(x, y));
                    QCOMPARE(result.mask.contains(destination), mask.contains(QPoint(x, y)));
                }
            }
        }
        QVERIFY(!openpaint::transformImage(source, {}, openpaint::TransformKind(90), token)
                     .error.isEmpty());
        token->store(true);
        QVERIFY(openpaint::transformImage(source, {}, openpaint::TransformKind::RotateRight, token)
                    .cancelled);
    }
    void replacementHistoryAndBudget() {
        openpaint::Document doc({6, 4});
        QVERIFY(doc.reset(fixture()));
        const auto original = doc.image();
        const auto saved = doc.revision();
        const auto rotated = original.transformed(QTransform().rotate(90));
        QVERIFY(!doc.replaceImage(rotated, saved + 1));
        QCOMPARE(doc.image(), original);
        QVERIFY(doc.replaceImage(rotated, saved));
        QVERIFY(doc.dirty());
        QVERIFY(doc.undo());
        QCOMPARE(doc.image(), original);
        QVERIFY(!doc.dirty());
        QVERIFY(doc.redo());
        QCOMPARE(doc.image(), rotated);
        QVERIFY(doc.undo());
        const auto crop = original.copy(1, 1, 3, 2);
        QVERIFY(doc.replaceImage(crop, doc.revision()));
        QVERIFY(!doc.canRedo());
        QVERIFY(doc.undo());
        QCOMPARE(doc.image(), original);
        openpaint::Document limited({6, 4}, 100);
        const auto base = limited.image();
        QVERIFY(
            !limited.replaceImage(base.transformed(QTransform().rotate(90)), limited.revision()));
        QCOMPARE(limited.image(), base);
        QVERIFY(!limited.canUndo());
        QVERIFY(!limited.dirty());
    }
    void controllerCropRotateAndSave() {
        TransformEditor editor;
        editor.reset(fixture());
        const auto original = editor.image();
        QSignalSpy replaced(&editor, &Editor::documentReplaced);
        editor.transformImage(0);
        QTRY_VERIFY(!editor.busy());
        QCOMPARE(editor.imageSize(), QSize(4, 6));
        QCOMPARE(replaced.size(), 1);
        editor.undo();
        QCOMPARE(editor.image(), original);
        QCOMPARE(replaced.size(), 2);
        editor.redo();
        QCOMPARE(editor.imageSize(), QSize(4, 6));
        editor.undo();
        const QRegion mask = QRegion(QRect(1, 0, 4, 4)) - QRect(2, 1, 1, 2);
        editor.select(mask);
        const auto crop = editor.selectedImage();
        editor.transformImage(5);
        QTRY_VERIFY(!editor.busy());
        QCOMPARE(editor.image(), crop);
        QVERIFY(!editor.hasSelection());
        QTemporaryDir dir;
        const auto url = QUrl::fromLocalFile(dir.filePath("crop.png"));
        editor.saveFile(url);
        QTRY_VERIFY(!editor.busy());
        QCOMPARE(openpaint::loadPng(url.toLocalFile()).image, crop);
        editor.undo();
        QCOMPARE(editor.image(), original);
        editor.redo();
        QVERIFY(!editor.dirty());
        editor.selectAll();
        editor.transformImage(5);
        QTRY_VERIFY(!editor.busy());
        QVERIFY(!editor.hasSelection());
        QCOMPARE(editor.image(), crop);
        QVERIFY(!editor.dirty());
    }
    void selectionPreviewAndCancellation() {
        TransformEditor editor;
        editor.reset(fixture());
        const auto original = editor.image();
        editor.selectRect(1, 0, 3, 2);
        const auto selected = editor.selection();
        editor.transformImage(0);
        QTRY_VERIFY(!editor.busy());
        QVERIFY(editor.floatingActive());
        QCOMPARE(editor.selectionRect().size(), QSize(2, 3));
        editor.cancel();
        QCOMPARE(editor.image(), original);
        QCOMPARE(editor.selection(), selected);
        editor.transformImage(3);
        QTRY_VERIFY(!editor.busy());
        const auto preview = editor.image();
        editor.finish();
        editor.undo();
        QCOMPARE(editor.image(), original);
        editor.redo();
        QCOMPARE(editor.image(), preview);
        editor.deselect();
        editor.transformImage(0);
        editor.cancelIo();
        QTRY_VERIFY(!editor.busy());
        QCOMPARE(editor.image(), preview);
        editor.transformImage(0);
        editor.reset(original); // A stale worker must not overwrite a newer revision.
        QTRY_VERIFY(!editor.busy());
        QCOMPARE(editor.image(), original);
        QVERIFY(!editor.error().isEmpty());
    }
};
QTEST_GUILESS_MAIN(TransformTest)
#include "tst_transform.moc"
