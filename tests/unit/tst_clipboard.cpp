// SPDX-License-Identifier: GPL-3.0-or-later
#include "app/Editor.h"
#include <QBuffer>
#include <QClipboard>
#include <QColorSpace>
#include <QGuiApplication>
#include <QMimeData>
#include <QPainter>
#include <QTemporaryDir>
#include <QtTest>
class ClipboardEditor : public Editor {
  public:
    void reset(const QImage& image) {
        document.reset(image);
    }
    void select(const QRegion& region) {
        document.setSelection(region);
    }
};
class ClipboardTest : public QObject {
    Q_OBJECT
  private slots:
    void cutUsesBackground() {
        for (const auto color :
             {QColor(Qt::yellow), QColor(Qt::transparent), QColor(30, 90, 150, 120)}) {
            ClipboardEditor editor;
            QImage source(20, 20, QImage::Format_ARGB32_Premultiplied);
            source.fill(Qt::red);
            source.setColorSpace(QColorSpace::SRgb);
            editor.reset(source);
            editor.secondary = color;
            editor.selectRect(2, 2, 5, 5);
            editor.cutSelection();
            QTRY_VERIFY(!editor.busy());
            QImage expected(1, 1, source.format());
            expected.fill(color);
            QCOMPARE(editor.image().pixel(3, 3), expected.pixel(0, 0));
            QCOMPARE(editor.image().pixelColor(0, 0), QColor(Qt::red));
            editor.undo();
            QCOMPARE(editor.image(), source);
        }
    }
    void cleanup() {
        QGuiApplication::clipboard()->clear();
    }
    void freeFormAndInversion() {
        ClipboardEditor editor;
        editor.setTool(Editor::Tool::Select);
        editor.selectRect(5, 5, 10, 10);
        editor.deselect();
        const auto prior = editor.selection();
        editor.freeFormSelection = true;
        editor.begin({100, 100}, false);
        editor.extend({200, 100});
        editor.extend({100, 200});
        QVERIFY(!editor.hasSelection()); // Mask is built once at release.
        QVERIFY(!editor.selectionDraft().isEmpty());
        editor.endGesture();
        const auto triangle = editor.selection();
        QVERIFY(triangle.contains(QPoint(120, 120)));
        QVERIFY(!triangle.contains(QPoint(180, 180)));
        editor.deselect();
        editor.begin({100, 100}, false);
        editor.extend({100, 200});
        editor.extend({200, 100});
        editor.endGesture();
        QCOMPARE(editor.selection(), triangle);
        QVERIFY(!editor.dirty());
        editor.invertSelection();
        QVERIFY(!editor.selection().contains(QPoint(120, 120)));
        QVERIFY(editor.selection().contains(QPoint(180, 180)));
        editor.invertSelection();
        QCOMPARE(editor.selection(), triangle);
        editor.deselect();
        editor.begin({400, 100}, false);
        editor.extend({410, 110});
        editor.endGesture();
        QCOMPARE(editor.selection(), prior); // Degenerate path restores empty selection.
        editor.begin({400, 100}, false);
        editor.extend({450, 100});
        editor.extend({450, 160});
        editor.cancel();
        QCOMPARE(editor.selection(), prior);
        editor.select(prior);
        editor.begin({400, 100}, false);
        for (int i = 0; i < 4097; ++i)
            editor.extend({400.0 + (i % 2), 100.0 + (i % 3)});
        editor.endGesture();
        QCOMPARE(editor.selection(), prior);
        QVERIFY(!editor.error().isEmpty());
        editor.begin({400, 100}, false);
        editor.extend({500, 200});
        editor.extend({500, 100});
        editor.extend({400, 200});
        editor.endGesture();
        QVERIFY(editor.hasSelection()); // Odd-even self-crossing path.
        editor.selectAll();
        editor.invertSelection();
        QVERIFY(!editor.hasSelection());
        QVERIFY(!editor.dirty());
    }
    void irregularMovePreservesHole() {
        ClipboardEditor editor;
        editor.setTool(Editor::Tool::Select);
        QImage base(100, 60, QImage::Format_ARGB32_Premultiplied);
        base.fill(Qt::green);
        editor.reset(base);
        const auto original = editor.image();
        const QRegion ring = QRegion(QRect(10, 10, 30, 30)) - QRect(20, 20, 10, 10);
        editor.select(ring);
        editor.begin({12, 12}, false);
        editor.extend({52, 12});
        editor.endGesture();
        QCOMPARE(editor.image().pixelColor(12, 12), editor.secondary);
        QCOMPARE(editor.image().pixelColor(25, 25), QColor(Qt::green));
        QCOMPARE(editor.image().pixelColor(52, 12), QColor(Qt::green));
        editor.finish();
        QCOMPARE(editor.selection(), ring.translated(40, 0));
        editor.undo();
        QCOMPARE(editor.image(), original);
    }
    void transparentKeyCanBeReversed() {
        ClipboardEditor editor;
        editor.setTool(Editor::Tool::Select);
        QImage source(3, 1, QImage::Format_ARGB32_Premultiplied);
        source.fill(Qt::white);
        source.setPixelColor(1, 0, QColor(254, 255, 255));
        source.setPixelColor(2, 0, QColor(255, 255, 255, 128));
        QImage base(20, 20, source.format());
        base.fill(Qt::blue);
        editor.reset(base);
        QVERIFY(editor.pasteImage(source));
        const auto opaque = editor.image();
        editor.setProperty("transparentSelection", true);
        QCOMPARE(editor.image().pixelColor(0, 0), QColor(Qt::blue));
        QCOMPARE(editor.image().pixelColor(1, 0), source.pixelColor(1, 0));
        QCOMPARE(editor.image().pixelColor(2, 0), opaque.pixelColor(2, 0));
        editor.setProperty("transparentSelection", false);
        QCOMPARE(editor.image(), opaque);
        editor.cancel();
        editor.reset(source);
        editor.selectAll();
        editor.transparentSelection = true;
        QVERIFY(editor.copySelection());
        QTRY_VERIFY(!editor.busy());
        const auto copied =
            openpaint::decodePng(QGuiApplication::clipboard()->mimeData()->data("image/png"));
        QVERIFY(copied.ok());
        QCOMPARE(copied.image.pixelColor(0, 0).alpha(), 0);
        QCOMPARE(copied.image.pixelColor(1, 0), source.pixelColor(1, 0));
        QCOMPARE(copied.image.pixelColor(2, 0), source.pixelColor(2, 0));
    }
    void selectedAlphaAndClipboardRoundTrip() {
        QImage source(80, 60, QImage::Format_ARGB32_Premultiplied);
        source.fill(Qt::transparent);
        source.setPixelColor(12, 12, QColor(255, 0, 0, 128));
        source.setPixelColor(22, 12, Qt::blue);
        source.setText(QStringLiteral("Comment"), QStringLiteral("synthetic metadata"));
        ClipboardEditor editor;
        editor.setTool(Editor::Tool::Select);
        editor.reset(source);
        const auto original = editor.image();
        editor.select(QRegion(QRect(10, 10, 5, 5)) + QRect(20, 10, 5, 5));
        QVERIFY(editor.copySelection());
        QTRY_VERIFY(!editor.busy());
        QVERIFY(!editor.dirty());
        const auto bytes = QGuiApplication::clipboard()->mimeData()->data("image/png");
        const auto result = openpaint::decodePng(bytes);
        QVERIFY(result.ok());
        QCOMPARE(result.image.size(), QSize(15, 5));
        QCOMPARE(result.image.pixelColor(2, 2), source.pixelColor(12, 12));
        QCOMPARE(result.image.pixelColor(7, 2).alpha(), 0);
        QVERIFY(result.image.textKeys().isEmpty());
        editor.cutSelection();
        QTRY_VERIFY(!editor.busy());
        QCOMPARE(editor.image().pixelColor(12, 12), editor.secondary);
        editor.undo();
        QCOMPARE(editor.image(), original);
        editor.pasteClipboard();
        QTRY_VERIFY(!editor.busy());
        QVERIFY(editor.floatingActive());
        editor.cancel();
        QCOMPARE(editor.image(), original);
        QCOMPARE(editor.selectionRect(), QRect(10, 10, 15, 5));
    }
    void moveOverlapCancelAndHistory() {
        QImage source(80, 60, QImage::Format_ARGB32_Premultiplied);
        source.fill(Qt::transparent);
        for (int y = 10; y < 30; ++y)
            for (int x = 10; x < 30; ++x)
                source.setPixelColor(x, y, QColor(x * 7, y * 7, 0, 160));
        ClipboardEditor editor;
        editor.setTool(Editor::Tool::Select);
        editor.reset(source);
        editor.secondary = Qt::transparent;
        const auto original = editor.image();
        editor.selectRect(10, 10, 20, 20);
        editor.begin({15, 15}, false);
        editor.extend({25, 15});
        editor.endGesture();
        QVERIFY(editor.floatingActive());
        QCOMPARE(editor.floatingPosition(), QPoint(20, 10));
        QCOMPARE(editor.image().pixelColor(15, 15).alpha(), 0);
        QCOMPARE(editor.image().pixelColor(25, 15), original.pixelColor(15, 15));
        const auto moved = editor.image();
        editor.cancel();
        QCOMPARE(editor.image(), original);
        QCOMPARE(editor.selectionRect(), QRect(10, 10, 20, 20));
        editor.begin({15, 15}, false);
        editor.extend({25, 15});
        editor.endGesture();
        editor.finish();
        QCOMPARE(editor.image(), moved);
        QCOMPARE(editor.selectionRect(), QRect(20, 10, 20, 20));
        editor.undo();
        QCOMPARE(editor.image(), original);
        editor.redo();
        QCOMPARE(editor.image(), moved);
    }
    void pasteClippingAndFileCommit() {
        ClipboardEditor editor;
        editor.setTool(Editor::Tool::Select);
        QImage base(80, 60, QImage::Format_ARGB32_Premultiplied);
        base.fill(Qt::blue);
        editor.reset(base);
        base = editor.image();
        QImage source(20, 20, base.format());
        source.fill(QColor(255, 0, 0, 128));
        QVERIFY(editor.pasteImage(source));
        editor.moveFloating(-10, -10);
        const auto partly = editor.image();
        QCOMPARE(partly.pixelColor(15, 15), QColor(Qt::blue));
        QVERIFY(partly.pixelColor(0, 0) != QColor(Qt::blue));
        editor.moveFloating(-20, 0);
        QCOMPARE(editor.image(), partly);
        editor.undo(); // Cancels only the preview.
        QCOMPARE(editor.image(), base);
        QVERIFY(!editor.hasSelection());
        QVERIFY(editor.pasteImage(source));
        editor.moveFloating(20, 20);
        const auto pasted = editor.image();
        QTemporaryDir dir;
        editor.saveFile(QUrl::fromLocalFile(dir.filePath("pasted.png")));
        QTRY_VERIFY(!editor.busy());
        QVERIFY(!editor.floatingActive());
        QCOMPARE(openpaint::loadPng(dir.filePath("pasted.png")).image, pasted);
        editor.undo();
        QCOMPARE(editor.image(), base);
        QVERIFY(!editor.canUndo());
        editor.redo();
        QCOMPARE(editor.image(), pasted);
    }
    void invalidClipboardAndCancellation() {
        Editor editor;
        const auto base = editor.image();
        auto* mime = new QMimeData;
        mime->setText(QStringLiteral("plain text"));
        QGuiApplication::clipboard()->setMimeData(mime);
        editor.pasteClipboard();
        QVERIFY(!editor.busy());
        QVERIFY(!editor.error().isEmpty());
        QCOMPARE(editor.image(), base);
        mime = new QMimeData;
        mime->setData("image/png", "broken PNG");
        QGuiApplication::clipboard()->setMimeData(mime);
        editor.pasteClipboard();
        QTRY_VERIFY(!editor.busy());
        QVERIFY(!editor.floatingActive());
        QCOMPARE(editor.image(), base);
        QImage tooWide(16385, 1, QImage::Format_ARGB32_Premultiplied);
        tooWide.fill(Qt::white);
        QByteArray oversized;
        QBuffer encoded(&oversized);
        encoded.open(QIODevice::WriteOnly);
        QVERIFY(openpaint::writePng(encoded, tooWide));
        QVERIFY(!openpaint::decodePng(oversized).ok());
        QCOMPARE(editor.image(), base);
        editor.selectAll();
        const auto clipboardBefore = QGuiApplication::clipboard()->mimeData()->data("image/png");
        editor.cutSelection();
        editor.cancelIo();
        QTRY_VERIFY(!editor.busy());
        QCOMPARE(editor.image(), base);
        QCOMPARE(QGuiApplication::clipboard()->mimeData()->data("image/png"), clipboardBefore);
        QVERIFY(editor.copySelection());
        QTRY_VERIFY(!editor.busy());
        editor.pasteClipboard();
        editor.cancelIo();
        QTRY_VERIFY(!editor.busy());
        QVERIFY(!editor.floatingActive());
        QCOMPARE(editor.image(), base);
    }
};
QTEST_MAIN(ClipboardTest)
#include "tst_clipboard.moc"
