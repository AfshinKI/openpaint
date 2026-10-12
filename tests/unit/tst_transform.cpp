// SPDX-License-Identifier: GPL-3.0-or-later
#include "app/Editor.h"
#include "tools/Transform.h"
#include <QColorSpace>
#include <QTemporaryDir>
#include <QtTest>
#include <limits>
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
    void resizeSkewBoundsAndFiltering() {
        using namespace openpaint;
        const auto source = fixture();
        const auto token = std::make_shared<std::atomic_bool>(false);
        const QRegion mask(QRect(1, 1, 2, 2));
        auto nearest =
            transformImage(source, mask, TransformKind::ResizeSkew, token, {{12, 8}, 0, 0, false});
        QVERIFY(nearest.error.isEmpty());
        QCOMPARE(nearest.image.size(), QSize(12, 8));
        QCOMPARE(nearest.mask, QRegion(QRect(2, 2, 4, 4)));
        for (int y = 0; y < 8; ++y)
            for (int x = 0; x < 12; ++x)
                QCOMPARE(nearest.image.pixel(x, y), source.pixel(x / 2, y / 2));
        auto smooth =
            transformImage(source, mask, TransformKind::ResizeSkew, token, {{12, 8}, 0, 0, true});
        QVERIFY(smooth.error.isEmpty());
        QVERIFY(smooth.image != nearest.image);
        auto skewed =
            transformImage(source, mask, TransformKind::ResizeSkew, token, {{6, 4}, 45, 0, false});
        QVERIFY(skewed.error.isEmpty());
        QCOMPARE(skewed.image.size(), QSize(10, 4));
        QCOMPARE(skewed.image.pixelColor(9, 0).alpha(), 0);
        QVERIFY(skewed.image.rect().contains(skewed.mask.boundingRect()));
        for (const auto options :
             {TransformOptions{{0, 4}, 0, 0, true}, TransformOptions{{16384, 16384}, 0, 0, true},
              TransformOptions{{16384, 1}, 89, 0, true}, TransformOptions{{6, 4}, 45, 45, true},
              TransformOptions{{6, 4}, std::numeric_limits<double>::quiet_NaN(), 0, true}}) {
            const auto invalid =
                transformImage(source, mask, TransformKind::ResizeSkew, token, options);
            QVERIFY(!invalid.error.isEmpty());
            QVERIFY(invalid.image.isNull());
        }
    }
    void smoothSelectionRetainsEdgeCoverage() {
        using namespace openpaint;
        QImage source(6, 4, QImage::Format_ARGB32_Premultiplied);
        source.fill(Qt::transparent);
        source.setPixelColor(0, 0, Qt::red);
        source.setPixelColor(5, 3, QColor(0, 0, 255, 100));
        const QRegion mask = QRegion(QRect(0, 0, 1, 1)) + QRect(5, 3, 1, 1);
        const auto token = std::make_shared<std::atomic_bool>(false);
        for (const auto options :
             {TransformOptions{{24, 16}, 15, 0, true}, TransformOptions{{1, 1}, 0, 0, true}}) {
            const auto result =
                transformImage(source, mask, TransformKind::ResizeSkew, token, options);
            QVERIFY(result.error.isEmpty());
            QVERIFY(!result.mask.isEmpty());
            for (int y = 0; y < result.image.height(); ++y)
                for (int x = 0; x < result.image.width(); ++x)
                    if (qAlpha(result.image.pixel(x, y)))
                        QVERIFY(result.mask.contains(QPoint(x, y)));
        }
    }
    void resizeControllerHistory() {
        TransformEditor editor;
        editor.reset(fixture());
        const auto original = editor.image();
        editor.resizeImage(12, 8, 0, 0, false);
        QTRY_VERIFY(!editor.busy());
        QCOMPARE(editor.imageSize(), QSize(12, 8));
        const auto resized = editor.image();
        editor.undo();
        QCOMPARE(editor.image(), original);
        editor.redo();
        QCOMPARE(editor.image(), resized);
        editor.selectRect(2, 2, 4, 4);
        editor.resizeImage(6, 3, 0, 0, true);
        QTRY_VERIFY(!editor.busy());
        QVERIFY(editor.floatingActive());
        QCOMPARE(editor.selectionRect().size(), QSize(6, 3));
        editor.cancel();
        QCOMPARE(editor.image(), resized);
        editor.deselect();
        editor.resizeImage(0, 20, 0, 0, true);
        QTRY_VERIFY(!editor.busy());
        QCOMPARE(editor.image(), resized);
        editor.resizeImage(20, 20, 10, 0, true);
        editor.cancelIo();
        QTRY_VERIFY(!editor.busy());
        QCOMPARE(editor.image(), resized);
    }
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
