// SPDX-License-Identifier: GPL-3.0-or-later
#include "app/Editor.h"
#include <QTemporaryDir>
#include <QtTest>
class EditorTest : public QObject {
    Q_OBJECT
  private slots:
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
