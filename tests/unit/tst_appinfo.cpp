// SPDX-License-Identifier: GPL-3.0-or-later
#include "platform/AppInfo.h"
#include "platform/Appearance.h"
#include <QDesktopServices>
#include <QTemporaryDir>
#include <QtTest>

class AppInfoTest : public QObject {
    Q_OBJECT
    QUrl received;
  public slots:
    void capture(const QUrl& url) {
        received = url;
    }
  private slots:
    void rulerPreferencePersists() {
        QTemporaryDir dir;
        const auto file = dir.filePath("appearance.ini");
        Appearance first(file);
        QVERIFY(first.rulersVisible());
        first.setRulersVisible(false);
        Appearance reopened(file);
        QVERIFY(!reopened.rulersVisible());
        reopened.restoreDefaults(); // Glass reset does not reset View choices.
        Appearance afterGlassReset(file);
        QVERIFY(!afterGlassReset.rulersVisible());
        afterGlassReset.setRulersVisible(true);
        Appearance enabledAgain(file);
        QVERIFY(enabledAgain.rulersVisible());
    }
    void appearancePersistenceAndBounds() {
        QTemporaryDir dir;
        const auto file = dir.filePath("appearance.ini");
        Appearance first(file);
        QVERIFY(first.glassEnabled());
        QCOMPARE(first.surfaceOpacity(), 40);
        first.setGlassEnabled(false);
        first.setSurfaceOpacity(62);
        first.setThemeMode(2);
        Appearance restored(file);
        QVERIFY(!restored.glassEnabled());
        QCOMPARE(restored.surfaceOpacity(), 62);
        QCOMPARE(restored.themeMode(), 2);
        restored.setSurfaceOpacity(-5);
        QCOMPARE(restored.surfaceOpacity(), 40);
        restored.setSurfaceOpacity(200);
        QCOMPARE(restored.surfaceOpacity(), 100);
        restored.restoreDefaults();
        Appearance defaults(file);
        QVERIFY(defaults.glassEnabled());
        QCOMPARE(defaults.surfaceOpacity(), 40);
    }
    void explicitBrowserRequest() {
        QDesktopServices::setUrlHandler("https", this, "capture");
        AppInfo info;
        QVERIFY(received.isEmpty());
        QVERIFY(!info.version().isEmpty());
        QVERIFY(!info.channel().isEmpty());
        QVERIFY(!info.installation().isEmpty());
        QVERIFY(!info.instructions().isEmpty());
        QCOMPARE(info.releaseUrl(), QUrl("https://github.com/AfshinKI/openpaint/releases"));
        QVERIFY(received.isEmpty());
        QVERIFY(info.openReleases());
        QCOMPARE(received, info.releaseUrl());
        QVERIFY(info.openContributing());
        QCOMPARE(received, QUrl("https://github.com/AfshinKI/openpaint/blob/main/CONTRIBUTING.md"));
        QVERIFY(info.openWebsite());
        QCOMPARE(received, QUrl("https://afshinki.github.io/openpaint/"));
        QVERIFY(info.openContact());
        QCOMPARE(received, QUrl("https://github.com/AfshinKI/openpaint/issues"));
        QDesktopServices::unsetUrlHandler("https");
    }
};
QTEST_MAIN(AppInfoTest)
#include "tst_appinfo.moc"
