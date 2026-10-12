#include "document/Document.h"
#include "tools/Fill.h"
#include <QtTest>
using openpaint::Document;
class DocumentTest : public QObject {
    Q_OBJECT
  private slots:
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
