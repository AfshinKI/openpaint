// SPDX-License-Identifier: GPL-3.0-or-later
#include "color/ColorPalette.h"
#include "color/ColorSelection.h"
#include <QFile>
#include <QTemporaryDir>
#include <QtTest>
#include <limits>

class ColorsTest : public QObject {
    Q_OBJECT
  private slots:
    void conversionAndValidation() {
        ColorSelection state;
        QVERIFY(state.setHex("#1a80ff"));
        QCOMPARE(state.hex(), QString("#1A80FF"));
        QCOMPARE(state.red(), 26);
        QCOMPARE(state.green(), 128);
        QCOMPARE(state.blue(), 255);
        const auto before = state.color();
        QVERIFY(!state.setHex("#oops"));
        QCOMPARE(state.color(), before);
        state.setHsv(120, 100, 100);
        QCOMPARE(state.color(), QColor(Qt::green));
        state.setHsv(210, 0, 50);
        QCOMPARE(state.hue(), 210.0);
        state.setColor(QColor("#555555"));
        QCOMPARE(state.hue(), 210.0);
        state.setHsv(state.hue(), 100, 100);
        QVERIFY(state.blue() == 255 && state.red() == 0);
        state.setHsv(std::numeric_limits<double>::quiet_NaN(), 100, 100);
        QCOMPARE(state.hue(), 210.0);
        state.setRgb(-5, 300, 12);
        QCOMPARE(state.color(), QColor(0, 255, 12));
    }
    void palettePersistsAndIsBounded() {
        QTemporaryDir dir;
        const auto path = dir.filePath("palette.json");
        ColorPalette palette(nullptr, path);
        QVERIFY(palette.colors().isEmpty());
        for (int i = 0; i < 24; ++i)
            QVERIFY(palette.add(QColor(i * 10, 20, 200)));
        QVERIFY(!palette.add(Qt::red));
        QVERIFY(palette.replace(2, Qt::yellow));
        ColorPalette restored(nullptr, path);
        QCOMPARE(restored.colors(), palette.colors());
        QCOMPARE(restored.colors().at(2), QString("#ffff00"));
        QVERIFY(!palette.replace(-1, Qt::red));
        QVERIFY(!palette.remove(24));
        QVERIFY(palette.remove(2));
        QCOMPARE(palette.colors().size(), 23);
        QVERIFY(palette.clear());
        ColorPalette empty(nullptr, path);
        QVERIFY(empty.colors().isEmpty());
    }
    void legacyTenColorPaletteLoads() {
        QTemporaryDir dir;
        const auto path = dir.filePath("palette.json");
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("{\"version\":1,\"colors\":[\"#000000\",\"#111111\",\"#222222\",\"#333333\",\"#"
                   "444444\",\"#555555\",\"#666666\",\"#777777\",\"#888888\",\"#999999\"]}");
        file.close();
        ColorPalette palette(nullptr, path);
        QCOMPARE(palette.colors().size(), 10);
        QCOMPARE(palette.colors().last(), QString("#999999"));
        QVERIFY(palette.add(Qt::red));
        ColorPalette reopened(nullptr, path);
        QCOMPARE(reopened.colors().size(), 11);
    }
    void toolbarSlotsPreserveGapsAndFailures() {
        QTemporaryDir dir;
        const auto path = dir.filePath("palette.json");
        ColorPalette palette(nullptr, path);
        QVERIFY(palette.setToolbarColor(7, Qt::red));
        QVERIFY(palette.toolbarColors().at(6).isEmpty());
        QVERIFY(palette.colors().isEmpty());
        ColorPalette restored(nullptr, path);
        QCOMPARE(restored.toolbarColors(), palette.toolbarColors());
        QVERIFY(!palette.setToolbarColor(10, Qt::blue));
        QVERIFY(QFile::remove(path + ".toolbar"));
        QVERIFY(QDir().mkdir(path + ".toolbar"));
        QVERIFY(!palette.setToolbarColor(7, Qt::blue));
        QCOMPARE(palette.toolbarColors().at(7), QString("#ff0000"));
        QVERIFY(!palette.error().isEmpty());
    }
    void failedWritesAndCorruptSettings() {
        QTemporaryDir dir;
        const auto path = dir.filePath("palette.json");
        ColorPalette palette(nullptr, path);
        QVERIFY(palette.add(Qt::red));
        QFile file(path);
        QVERIFY(file.remove());
        QVERIFY(QDir().mkdir(path)); // A directory cannot be replaced by QSaveFile.
        QVERIFY(!palette.add(Qt::blue));
        QCOMPARE(palette.colors(), QStringList{"#ff0000"});
        QVERIFY(!palette.error().isEmpty());
        QVERIFY(QDir().rmdir(path));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("{\"version\":1,\"colors\":[\"#ff0000\",\"invalid\"]}");
        file.close();
        ColorPalette corrupt(nullptr, path);
        QVERIFY(corrupt.colors().isEmpty());
        QVERIFY(!corrupt.error().isEmpty());
        QVERIFY(corrupt.add(Qt::green));
        QVERIFY(corrupt.error().isEmpty());
        ColorPalette recovered(nullptr, path);
        QCOMPARE(recovered.colors(), QStringList{"#00ff00"});
    }
};
QTEST_GUILESS_MAIN(ColorsTest)
#include "tst_colors.moc"
