// SPDX-License-Identifier: GPL-3.0-or-later
#include "color/ColorPalette.h"
#include "color/ColorSelection.h"
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
    void sessionPalettesFillIndependently() {
        ColorPalette palette;
        QVERIFY(palette.colors().isEmpty());
        QCOMPARE(palette.toolbarColors(), QStringList(10, QString()));
        for (int i = 0; i < 24; ++i)
            QVERIFY(palette.remember(QColor(i, 20, 200)));
        QCOMPARE(palette.colors().size(), 24);
        QCOMPARE(palette.toolbarColors().last(), QColor(9, 20, 200).name());
        const auto toolbar = palette.toolbarColors();
        const auto library = palette.colors();
        QVERIFY(palette.remember(Qt::red));
        QCOMPARE(palette.toolbarColors(), toolbar);
        QCOMPARE(palette.colors(), library);
        // Applying a known color must not consume another slot.
        QVERIFY(palette.remember(QColor(0, 20, 200)));
        QCOMPARE(palette.colors(), library);
        QVERIFY(palette.remember(Qt::red, 7));
        QCOMPARE(palette.toolbarColors().at(7), QString("#ff0000"));
        QCOMPARE(palette.colors(), library);
        QVERIFY(!palette.remember(Qt::red, 10));
        ColorPalette nextLaunch;
        QVERIFY(nextLaunch.colors().isEmpty());
        QCOMPARE(nextLaunch.toolbarColors(), QStringList(10, QString()));
    }
    void fullLibraryStillAllowsToolbarAndManagement() {
        ColorPalette palette;
        for (int i = 0; i < 24; ++i)
            QVERIFY(palette.add(QColor(i, 20, 200)));
        const auto library = palette.colors();
        QVERIFY(palette.remember(Qt::red));
        QCOMPARE(palette.toolbarColors().first(), QString("#ff0000"));
        QCOMPARE(palette.colors(), library);
        QVERIFY(!palette.add(Qt::blue));
        QVERIFY(palette.remove(7));
        QVERIFY(palette.remember(Qt::green));
        QCOMPARE(palette.colors().last(), QString("#00ff00"));
        QCOMPARE(palette.toolbarColors().at(1), QString("#00ff00"));
        QVERIFY(palette.replace(0, Qt::yellow));
        QVERIFY(!palette.replace(-1, Qt::blue));
        QVERIFY(!palette.remove(24));
        QVERIFY(palette.clear());
        QVERIFY(palette.colors().isEmpty());
        QCOMPARE(palette.toolbarColors().first(), QString("#ff0000"));
    }
};
QTEST_GUILESS_MAIN(ColorsTest)
#include "tst_colors.moc"
