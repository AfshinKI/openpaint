#include "document/Document.h"
#include <QtTest>
using openpaint::Document;
class DocumentTest : public QObject {
    Q_OBJECT
  private slots:
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
