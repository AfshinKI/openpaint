// SPDX-License-Identifier: GPL-3.0-or-later
#include "platform/Appearance.h"
#include "platform/MacGlass.h"
#import <AppKit/AppKit.h>
#include <QQuickWindow>
#include <QTemporaryDir>
#include <QtTest>
class MacGlassTest : public QObject {
    Q_OBJECT
  private slots:
    void nativeLifecycle() {
        QTemporaryDir directory;
        Appearance appearance(directory.filePath("settings.ini"));
        QQuickWindow window;
        window.setColor(Qt::transparent);
        window.resize(640, 480);
        bool reported = false;
        bool available = false;
        installMacGlass(&window, &appearance, [&](bool value) {
            reported = true;
            available = value;
        });
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_VERIFY(reported);
        QCOMPARE(window.property("nativeGlassAvailable").toBool(), available);
        QTRY_VERIFY(window.property("nativeGlassAttached").toBool());
        const bool expected =
            ![NSWorkspace sharedWorkspace].accessibilityDisplayShouldReduceTransparency;
        QTRY_COMPARE(available, expected);
        NSView* content = (__bridge NSView*)(void*)window.winId();
        QCOMPARE(content.superview, content.window.contentView);
        QVERIFY(content.superview.subviews.count >= 2);
        appearance.setGlassEnabled(false);
        window.resize(700, 500);
        appearance.setGlassEnabled(true);
        appearance.setThemeMode(2);
        QTest::qWait(100);
        QCOMPARE(window.property("nativeGlassAvailable").toBool(), expected);
    }
};
QTEST_MAIN(MacGlassTest)
#include "tst_macglass.moc"
