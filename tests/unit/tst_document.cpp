#include "document/Document.h"
#include "tools/Fill.h"
#include "tools/Shape.h"
#include <QtTest>
using openpaint::Document;
class DocumentTest : public QObject {
    Q_OBJECT
  private slots:
    void selectionClipsStrokesAndPreviews() {
        const QRegion mask(QRect(20, 20, 20, 20));
        for (int brush = -1; brush < 9; ++brush) {
            Document doc({80, 80});
            const auto base = doc.image();
            QVERIFY(doc.setSelection(mask));
            QVERIFY(!doc.dirty());
            QVERIFY(doc.beginStroke({0, 30}, Qt::black, 30, false, brush));
            QVERIFY(!doc.setSelection({}));
            doc.extendStroke({80, 30});
            QVERIFY(doc.commitStroke());
            QVERIFY(doc.image() != base);
            for (int y = 0; y < 80; ++y)
                for (int x = 0; x < 80; ++x)
                    if (!mask.contains(QPoint(x, y)))
                        QCOMPARE(doc.image().pixel(x, y), base.pixel(x, y));
            QVERIFY(doc.undo());
            QCOMPARE(doc.image(), base);
            QVERIFY(doc.redo());
        }
        Document doc({80, 80});
        const auto base = doc.image();
        QVERIFY(doc.setSelection(mask));
        QVERIFY(doc.beginPreview());
        QImage patch(60, 60, base.format());
        patch.fill(Qt::red);
        QVERIFY(doc.updatePreview(patch, QRect(0, 0, 60, 60)));
        QCOMPARE(doc.image().pixelColor(10, 30), QColor(Qt::white));
        QCOMPARE(doc.image().pixelColor(30, 30), QColor(Qt::red));
        QVERIFY(doc.updatePreview(patch, QRect(20, 20, 60, 60)));
        QCOMPARE(doc.image().pixelColor(60, 30), QColor(Qt::white));
        doc.cancelStroke();
        QCOMPARE(doc.image(), base);
        QVERIFY(doc.deleteSelection());
        const auto deleted = doc.image();
        QCOMPARE(deleted.pixelColor(30, 30).alpha(), 0);
        QCOMPARE(deleted.pixelColor(10, 30), QColor(Qt::white));
        QVERIFY(!doc.deleteSelection());
        QVERIFY(doc.undo());
        QCOMPARE(doc.image(), base);
        QVERIFY(doc.redo());
        QCOMPARE(doc.image(), deleted);
        QVERIFY(!doc.setSelection(QRect(-1, 0, 10, 10)));
        QCOMPARE(doc.selection(), mask);
        QVERIFY(doc.reset(base));
        QVERIFY(doc.selection().isEmpty());
    }
    void fillRespectsDisconnectedSelection() {
        Document doc({60, 30});
        const QRegion mask = QRegion(QRect(5, 5, 10, 10)) + QRect(30, 5, 10, 10);
        const auto token = std::make_shared<std::atomic_bool>(false);
        const auto filled = openpaint::fillRegion(doc.image(), {8, 8}, Qt::red, 255, token, mask);
        QVERIFY(!filled.failed);
        QCOMPARE(filled.rect, QRect(5, 5, 10, 10));
        QCOMPARE(filled.image.pixelColor(8, 8), QColor(Qt::red));
        QCOMPARE(filled.image.pixelColor(33, 8), QColor(Qt::white));
        QVERIFY(
            openpaint::fillRegion(doc.image(), {0, 0}, Qt::red, 255, token, mask).image.isNull());
    }
    void expandedShapeGeometry() {
        using namespace openpaint;
        Document doc({160, 160});
        QList<QImage> images;
        for (int kind = 0; kind < ShapeCount; ++kind) {
            const auto shape =
                renderShape(doc.image(), {20, 20}, {130, 130}, kind, Qt::black, Qt::red, 2, true,
                            true, 30, -30, {{20, 20}, {130, 45}, {80, 80}, {130, 130}, {20, 130}});
            QVERIFY(!shape.image.isNull());
            QVERIFY(doc.image().rect().contains(shape.rect));
            QImage full = doc.image();
            QVERIFY(doc.beginPreview());
            QVERIFY(doc.updatePreview(shape.image, shape.rect));
            full = doc.image();
            QVERIFY(!images.contains(full));
            images.append(full);
            doc.cancelStroke();
        }
        const auto a = renderShape(doc.image(), {20, 40}, {130, 120}, CurveShape, Qt::black,
                                   Qt::white, 2, true, false, 10, 20);
        const auto b = renderShape(doc.image(), {20, 40}, {130, 120}, CurveShape, Qt::black,
                                   Qt::white, 2, true, false, 50, -30);
        QVERIFY(a.image != b.image);
        QVERIFY(renderShape(doc.image(), {0, 0}, {50, 50}, ShapeCount, Qt::black, Qt::white, 2,
                            true, true)
                    .image.isNull());
        QList<QPointF> excessive(4097, {10, 10});
        QVERIFY(renderShape(doc.image(), {0, 0}, {50, 50}, PolygonShape, Qt::black, Qt::white, 2,
                            true, true, 30, -30, excessive)
                    .image.isNull());
    }

    void shapePreviewReplacementAndHistory() {
        using namespace openpaint;
        for (int kind = 0; kind < 3; ++kind) {
            Document doc({100, 100});
            const auto original = doc.image();
            QVERIFY(doc.beginPreview());
            const auto first = renderShape(doc.previewBase(), {10, 10}, {50, 50}, kind, Qt::black,
                                           Qt::red, 3, true, true);
            QVERIFY(doc.updatePreview(first.image, first.rect));
            QVERIFY(doc.image() != original);
            const auto next = renderShape(doc.previewBase(), {60, 60}, {95, 95}, kind, Qt::black,
                                          Qt::red, 3, true, true);
            QVERIFY(doc.updatePreview(next.image, next.rect));
            QCOMPARE(doc.image().pixelColor(10, 10), QColor(Qt::white));
            const auto painted = doc.image();
            QVERIFY(doc.commitStroke());
            QVERIFY(doc.undo());
            QCOMPARE(doc.image(), original);
            QVERIFY(!doc.canUndo());
            QVERIFY(doc.redo());
            QCOMPARE(doc.image(), painted);
            QVERIFY(doc.beginPreview());
            QVERIFY(doc.updatePreview(first.image, first.rect));
            QVERIFY(!doc.updatePreview(first.image, QRect(-1, 0, 20, 20)));
            doc.cancelStroke();
            QCOMPARE(doc.image(), painted);
        }
        Document drawing({100, 100});
        QVERIFY(drawing.beginStroke({10, 10}, Qt::black, 3));
        QVERIFY(!drawing.updatePreview({}, {}));
        drawing.cancelStroke();
        Document tiny({100, 100}, 1);
        const auto original = tiny.image();
        QVERIFY(tiny.beginPreview());
        auto patch = renderShape(tiny.previewBase(), {-20, -20}, {60, 60}, 1, Qt::black, Qt::red, 3,
                                 false, true);
        QVERIFY(tiny.updatePreview(patch.image, patch.rect));
        QCOMPARE(tiny.image().pixelColor(0, 0), QColor(Qt::red));
        QVERIFY(!tiny.commitStroke());
        QCOMPARE(tiny.image(), original);
        QVERIFY(tiny.beginPreview());
        patch = renderShape(tiny.previewBase(), {10, 10}, {60, 60}, 2, Qt::black, Qt::red, 3, false,
                            false);
        QVERIFY(tiny.updatePreview(patch.image, patch.rect));
        QVERIFY(!tiny.commitStroke());
        QVERIFY(!tiny.dirty());
    }

    void brushStylesAndOpacity() {
        using namespace openpaint;
        QList<QImage> previews;
        for (int style = 0; style < 9; ++style) {
            const auto preview = Brush::preview(style, Qt::black);
            QVERIFY(!previews.contains(preview));
            QCOMPARE(preview, Brush::preview(style, Qt::black));
            previews.append(preview);
            Document one({128, 64}), split({128, 64});
            const auto original = one.image();
            QVERIFY(one.beginStroke({10, 32}, Qt::red, 18, false, style, 50));
            one.extendStroke({110, 32});
            QVERIFY(one.commitStroke());
            QVERIFY(split.beginStroke({10, 32}, Qt::red, 18, false, style, 50));
            for (int x = 11; x <= 110; ++x)
                split.extendStroke({qreal(x), 32});
            QVERIFY(split.commitStroke());
            QCOMPARE(one.image(), split.image());
            const auto painted = one.image();
            QVERIFY(one.undo());
            QCOMPARE(one.image(), original);
            QVERIFY(one.redo());
            QCOMPARE(one.image(), painted);
            QVERIFY(one.beginStroke({5, 5}, Qt::blue, 12, false, style, 100));
            one.extendStroke({-1000000, -1000000});
            one.extendStroke({1000000, 1000000});
            one.cancelStroke();
            QCOMPARE(one.image(), painted);
        }
        Document opacity({32, 32});
        const auto original = opacity.image();
        QVERIFY(opacity.beginStroke({16, 16}, Qt::black, 12, false, 0, 50));
        opacity.extendStroke({17, 16});
        opacity.extendStroke({16, 16});
        QVERIFY(opacity.commitStroke());
        QCOMPARE(opacity.image().pixelColor(16, 16), QColor(127, 127, 127));
        opacity.undo();
        QVERIFY(opacity.beginStroke({16, 16}, QColor(0, 0, 0, 128), 12, false, 0, 50));
        QVERIFY(opacity.commitStroke());
        QCOMPARE(opacity.image().pixelColor(16, 16), QColor(191, 191, 191));
        opacity.undo();
        QVERIFY(opacity.beginStroke({16, 16}, Qt::red, 12, false, 0, 0));
        QVERIFY(!opacity.commitStroke());
        QCOMPARE(opacity.image(), original);
        QVERIFY(opacity.canRedo());
        QVERIFY(!opacity.beginStroke({16, 16}, Qt::red, 12, false, 9, 100));
    }

    void fillRecolorsAntialiasedBackground() {
        for (bool transparent : {false, true}) {
            Document doc({64, 64});
            QImage base = doc.image();
            base.fill(transparent ? Qt::transparent : Qt::white);
            QVERIFY(doc.reset(base));
            QVERIFY(doc.beginStroke({10.25, 12.75}, Qt::black, 3));
            doc.extendStroke({49.25, 43.75});
            QVERIFY(doc.commitStroke());
            const auto line = doc.image();
            const auto token = std::make_shared<std::atomic_bool>(false);
            const auto red = openpaint::fillRegion(line, {0, 0}, Qt::red, 0, token, {}, true);
            QVERIFY(!red.failed && !red.image.isNull());
            for (int y = 0; y < 64; ++y)
                for (int x = 0; x < 64; ++x) {
                    const int background =
                        transparent ? 255 - qAlpha(line.pixel(x, y)) : qRed(line.pixel(x, y));
                    QVERIFY(std::abs(qRed(red.image.pixel(x, y)) - background) <= 2);
                    QCOMPARE(qGreen(red.image.pixel(x, y)), 0);
                    QCOMPARE(qBlue(red.image.pixel(x, y)), 0);
                    QCOMPARE(qAlpha(red.image.pixel(x, y)), 255);
                }
            const auto blue =
                openpaint::fillRegion(red.image, {0, 0}, Qt::blue, 0, token, {}, true);
            QVERIFY(!blue.image.isNull());
            for (int y = 0; y < 64; ++y)
                for (int x = 0; x < 64; ++x)
                    QVERIFY(std::abs(qBlue(blue.image.pixel(x, y)) - qRed(red.image.pixel(x, y))) <=
                            2);
            QVERIFY(doc.applyPatch(red.image, red.rect, doc.revision()));
            doc.undo();
            QCOMPARE(doc.image(), line);
            doc.redo();
            QCOMPARE(doc.image(), red.image);
        }
        QImage boundary(6, 3, QImage::Format_ARGB32_Premultiplied);
        boundary.fill(Qt::white);
        for (int y = 0; y < 3; ++y) {
            boundary.setPixelColor(1, y, QColor(180, 180, 180));
            boundary.setPixelColor(2, y, Qt::black);
        }
        boundary.setPixelColor(1, 1, Qt::cyan);
        const auto token = std::make_shared<std::atomic_bool>(false);
        const auto filled = openpaint::fillRegion(boundary, {0, 0}, Qt::red, 0, token, {}, true);
        QCOMPARE(filled.image.pixelColor(1, 0), QColor(180, 0, 0));
        QCOMPARE(filled.image.pixelColor(1, 1), QColor(Qt::cyan));
        QCOMPARE(filled.image.pixelColor(2, 0), QColor(Qt::black));
        QCOMPARE(filled.image.pixelColor(3, 0), QColor(Qt::white));
        const auto selected = openpaint::fillRegion(boundary, {0, 0}, Qt::red, 0, token,
                                                    QRegion(QRect(0, 0, 1, 3)), true);
        QCOMPARE(selected.image.pixel(1, 0), boundary.pixel(1, 0));
    }
    void fillSmoothSameColorFringe() {
        using namespace openpaint;
        Document doc({64, 64});
        QVERIFY(doc.beginStroke({10.25, 12.75}, Qt::black, 3));
        doc.extendStroke({49.25, 43.75});
        QVERIFY(doc.commitStroke());
        const auto line = doc.image();
        const auto token = std::make_shared<std::atomic_bool>(false);
        const auto exact = fillRegion(line, {0, 0}, Qt::black, 0, token);
        bool fringe = false;
        for (int y = 0; y < 64; ++y)
            for (int x = 0; x < 64; ++x)
                fringe |= exact.image.pixelColor(x, y) != QColor(Qt::black);
        QVERIFY(fringe);
        const auto smooth = fillRegion(line, {0, 0}, Qt::black, 0, token, {}, true);
        for (int y = 0; y < 64; ++y)
            for (int x = 0; x < 64; ++x)
                QCOMPARE(smooth.image.pixelColor(x, y), QColor(Qt::black));
        QVERIFY(doc.applyPatch(smooth.image, smooth.rect, doc.revision()));
        QVERIFY(doc.undo());
        QCOMPARE(doc.image(), line);
        QVERIFY(doc.redo());
        QCOMPARE(doc.image(), smooth.image);

        QImage boundary(6, 3, QImage::Format_ARGB32_Premultiplied);
        boundary.fill(Qt::white);
        for (int y = 0; y < 3; ++y) {
            boundary.setPixelColor(1, y, QColor(180, 180, 180));
            boundary.setPixelColor(2, y, Qt::black);
        }
        boundary.setPixelColor(1, 1, Qt::red);
        const auto bounded = fillRegion(boundary, {0, 0}, Qt::black, 0, token, {}, true);
        QCOMPARE(bounded.image.pixelColor(1, 0), QColor(Qt::black));
        QCOMPARE(bounded.image.pixelColor(1, 1), QColor(Qt::red));
        QCOMPARE(bounded.image.pixelColor(3, 0), QColor(Qt::white));
        const auto selected =
            fillRegion(boundary, {0, 0}, Qt::black, 0, token, QRegion(QRect(0, 0, 1, 3)), true);
        QCOMPARE(selected.image.pixelColor(1, 0), boundary.pixelColor(1, 0));
        token->store(true);
        QVERIFY(fillRegion(boundary, {0, 0}, Qt::black, 0, token, {}, true).image.isNull());
    }

    void fillConnectivityToleranceAndHistory() {
        using namespace openpaint;
        Document doc({5, 3});
        QImage source = doc.image();
        source.setPixelColor(1, 0, QColor(250, 250, 250));
        for (int y = 0; y < 3; ++y)
            source.setPixelColor(2, y, Qt::black);
        QVERIFY(doc.reset(source));
        auto token = std::make_shared<std::atomic_bool>(false);
        const auto exact = fillRegion(source, {0, 0}, Qt::red, 0, token);
        QCOMPARE(exact.image.pixelColor(1, 0), QColor(250, 250, 250));
        QCOMPARE(exact.image.pixelColor(1, 1), QColor(Qt::red));
        QCOMPARE(exact.image.pixelColor(3, 1), QColor(Qt::white));
        const auto tolerant = fillRegion(source, {0, 0}, Qt::red, 5, token);
        QCOMPARE(tolerant.image.pixelColor(1, 0), QColor(Qt::red));
        QVERIFY(doc.applyPatch(tolerant.image, tolerant.rect, doc.revision()));
        const auto revision = doc.revision();
        QVERIFY(doc.undo());
        QCOMPARE(doc.image(), source);
        QVERIFY(!doc.dirty());
        QVERIFY(!doc.applyPatch(tolerant.image, tolerant.rect, revision));
        QVERIFY(doc.canRedo());
        QVERIFY(doc.redo());
        QCOMPARE(doc.image(), tolerant.image);
        auto all = fillRegion(source, {0, 0}, QColor(0, 0, 255, 128), 255, token);
        QCOMPARE(all.rect, source.rect());
        QCOMPARE(all.image.pixelColor(4, 2), QColor(0, 0, 255, 128));
        Document tiny({5, 3}, 1);
        QVERIFY(!tiny.applyPatch(all.image, all.rect, tiny.revision()));
        QVERIFY(!tiny.dirty());
        token->store(true);
        QVERIFY(fillRegion(source, {0, 0}, Qt::red, 255, token).image.isNull());
        token->store(false);
        QVERIFY(fillRegion(source, {-1, 0}, Qt::red, 0, token).failed);
        QVERIFY(fillRegion(source, {0, 0}, Qt::red, 256, token).failed);
        QVERIFY(fillRegion(source, {0, 0}, Qt::white, 0, token).rect.isEmpty());
        QImage diagonal(2, 2, QImage::Format_ARGB32_Premultiplied);
        diagonal.fill(Qt::black);
        diagonal.setPixelColor(0, 0, Qt::white);
        diagonal.setPixelColor(1, 1, Qt::white);
        const auto island = fillRegion(diagonal, {0, 0}, Qt::red, 0, token);
        QCOMPARE(island.image.pixelColor(1, 1), QColor(Qt::white));
        source.fill(Qt::transparent);
        source.setPixelColor(1, 0, QColor(0, 0, 0, 10));
        all = fillRegion(source, {0, 0}, Qt::red, 9, token);
        QCOMPARE(all.image.pixelColor(1, 0), QColor(0, 0, 0, 10));
        all = fillRegion(source, {0, 0}, Qt::red, 10, token);
        QCOMPARE(all.image.pixelColor(1, 0), QColor(Qt::red));
    }

    void history() {
        Document doc({64, 64});
        const QImage original = doc.image();
        QVERIFY(doc.beginStroke({2, 2}, Qt::black, 5));
        doc.extendStroke({50, 50});
        QVERIFY(doc.commitStroke());
        const QImage drawn = doc.image();
        QVERIFY(drawn != original);
        QVERIFY(doc.dirty());
        doc.markSaved(doc.revision());
        QVERIFY(!doc.dirty());
        QVERIFY(doc.undo());
        QCOMPARE(doc.image(), original);
        QVERIFY(doc.dirty());
        QVERIFY(doc.redo());
        QCOMPARE(doc.image(), drawn);
        QVERIFY(!doc.dirty());
        QVERIFY(!doc.redo());
        QVERIFY(doc.undo());
        doc.beginStroke({20, 20}, Qt::red, 7);
        doc.commitStroke();
        QVERIFY(!doc.canRedo());
        QVERIFY(doc.dirty());
    }
    void cancelAndErase() {
        Document doc({64, 64});
        const auto original = doc.image();
        doc.beginStroke({20, 20}, Qt::black, 12, true);
        QCOMPARE(doc.image().pixelColor(20, 20).alpha(), 0);
        doc.cancelStroke();
        QCOMPARE(doc.image(), original);
        QVERIFY(!doc.dirty());
        doc.beginStroke({20, 20}, Qt::black, 12, true);
        doc.commitStroke();
        QVERIFY(doc.undo());
        QCOMPARE(doc.image(), original);
        QVERIFY(doc.redo());
        QCOMPARE(doc.image().pixelColor(20, 20).alpha(), 0);
    }
    void trimmingAndLimits() {
        Document doc({64, 64}, 1);
        doc.beginStroke({20, 20}, Qt::blue, 10);
        QVERIFY(!doc.commitStroke());
        QVERIFY(!doc.canUndo());
        QVERIFY(!doc.dirty());
        QCOMPARE(doc.historyBytes(), 0);
        QVERIFY(!Document::validSize({0, 10}));
        QVERIFY(!Document::validSize({INT_MAX, INT_MAX}));
        QVERIFY(!Document::validSize({16384, 16384}));
        QVERIFY(!doc.reset(QImage()));
        QVERIFY(!doc.dirty());
        Document bounded({64, 64}, 800);
        for (int i = 0; i < 10; ++i) {
            bounded.beginStroke({qreal(i * 4 + 2), 2}, Qt::black, 1);
            QVERIFY(bounded.commitStroke());
        }
        QVERIFY(bounded.historyBytes() <= 800);
        int count = 0;
        while (bounded.undo())
            ++count;
        QVERIFY(count > 0 && count < 10);
        QVERIFY(bounded.dirty());
    }
};
QTEST_GUILESS_MAIN(DocumentTest)
#include "tst_document.moc"
