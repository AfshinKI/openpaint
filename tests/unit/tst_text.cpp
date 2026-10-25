// SPDX-License-Identifier: GPL-3.0-or-later
#include "app/Editor.h"
#include "tools/Text.h"
#include <QTemporaryDir>
#include <QtTest>
class TextTest : public QObject {
    Q_OBJECT
  private slots:
    void smartGuideTextMoves() {
        Editor editor;
        const auto base = editor.image();
        editor.setSmartGuides(true);
        editor.setTool(Editor::Tool::Text);
        editor.begin({10, 10}, false);
        editor.extend({130, 70});
        editor.endGesture();
        editor.setTextDraft(QStringLiteral("Guide test"));
        const auto size = editor.resizeBounds().size();
        editor.moveActiveBounds(2, 2, 6);
        QCOMPARE(editor.resizeBounds().topLeft(), QPointF(0, 0));
        QCOMPARE(editor.resizeBounds().size(), size);
        QCOMPARE(editor.guidePosition(), QPointF(0, 0));
        editor.cancel();
        QCOMPARE(editor.image(), base);
        QCOMPARE(editor.guidePosition(), QPointF(-1, -1));
    }
    void selectedTextClipsRaster() {
        Editor editor;
        editor.setTool(Editor::Tool::Text);
        editor.selectRect(20, 20, 40, 40);
        const auto base = editor.image();
        editor.begin({0, 0}, false);
        editor.endGesture();
        editor.textBackground = true;
        editor.secondary = Qt::red;
        QVERIFY(editor.setTextDraft(QStringLiteral("Selected text")));
        QCOMPARE(editor.displayImage(), editor.image());
        QCOMPARE(editor.image().pixelColor(10, 30), QColor(Qt::white));
        QVERIFY(editor.image() != base);
        for (int y = 0; y < base.height(); ++y)
            for (int x = 0; x < base.width(); ++x)
                if (!editor.selectionRect().contains(x, y))
                    QCOMPARE(editor.image().pixel(x, y), base.pixel(x, y));
        editor.commitPreview();
        editor.undo();
        QCOMPARE(editor.image(), base);
    }
    void boundedFormattedPlainText() {
        QImage base(180, 100, QImage::Format_ARGB32_Premultiplied);
        base.fill(Qt::white);
        QFont font(QStringLiteral("Sans Serif"));
        font.setPixelSize(20);
        openpaint::TextStyle style{font, Qt::black, Qt::red, false, 0};
        const QRect rect(10, 10, 160, 80);
        const auto normal =
            openpaint::renderText(base, rect, QString::fromUtf8("OpenPaint\nΩ"), style);
        QCOMPARE(normal.rect, rect);
        QVERIFY(normal.image != base.copy(rect));
        style.font.setBold(true);
        QVERIFY(openpaint::renderText(base, rect, QString::fromUtf8("OpenPaint\nΩ"), style).image !=
                normal.image);
        style.opaque = true;
        auto opaque = openpaint::renderText(base, rect, QStringLiteral("Text"), style);
        QCOMPARE(opaque.image.pixelColor(159, 79), QColor(Qt::red));
        style.alignment = 2;
        QVERIFY(openpaint::renderText(base, rect, QStringLiteral("Text"), style).image !=
                opaque.image);
        QVERIFY(openpaint::renderText(base, rect, QStringLiteral("<b>literal</b>"), style).image !=
                opaque.image);
        QVERIFY(openpaint::renderText(base, QRect(-1, 0, 10, 10), QStringLiteral("Text"), style)
                    .image.isNull());
        QVERIFY(openpaint::renderText(base, rect, QString(10001, 'x'), style).image.isNull());
        style.font.setPixelSize(513);
        QVERIFY(openpaint::renderText(base, rect, QStringLiteral("Text"), style).image.isNull());
    }
    void draftHistoryAndSave() {
        Editor editor;
        const auto original = editor.image();
        editor.setTool(Editor::Tool::Text);
        editor.begin({20, 20}, false);
        editor.extend({220, 140});
        editor.endGesture();
        QCOMPARE(editor.textRect(), QRect(20, 20, 200, 120));
        QVERIFY(editor.textReady());
        QVERIFY(editor.setTextDraft(QStringLiteral("OpenPaint\nplain text")));
        const auto first = editor.image();
        QVERIFY(first != original);
        QCOMPARE(editor.displayImage(), original);
        editor.setTool(Editor::Tool::Pencil);
        QCOMPARE(editor.tool(), Editor::Tool::Text);
        const auto rect = editor.textRect();
        editor.setTextBounds(-1, 0, 10, 10);
        QCOMPARE(editor.textRect(), rect);
        QVERIFY(!editor.setTextDraft(QString(10001, 'x')));
        QCOMPARE(editor.image(), first);
        editor.cancel();
        QCOMPARE(editor.image(), original);
        QVERIFY(!editor.dirty());
        editor.begin({20, 20}, true);
        editor.endGesture();
        editor.secondary = Qt::blue;
        QVERIFY(editor.setTextDraft(QStringLiteral("Saved text")));
        editor.setProperty("textBold", true);
        editor.setProperty("textUnderline", true);
        editor.setProperty("textAlignment", 1);
        const auto rendered = editor.image();
        QTemporaryDir dir;
        editor.saveFile(QUrl::fromLocalFile(dir.filePath("text.png")));
        QTRY_VERIFY(!editor.busy());
        QVERIFY(!editor.textActive());
        QVERIFY(!editor.dirty());
        QCOMPARE(openpaint::loadPng(dir.filePath("text.png")).image, rendered);
        editor.undo();
        QCOMPARE(editor.image(), original);
        editor.redo();
        QCOMPARE(editor.image(), rendered);
        editor.begin({1151.75, 647.75}, false);
        editor.endGesture();
        QCOMPARE(editor.textRect(), QRect(1151, 647, 1, 1));
        editor.cancel();
    }
};
QTEST_MAIN(TextTest)
#include "tst_text.moc"
