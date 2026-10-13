// SPDX-License-Identifier: GPL-3.0-or-later
#include "platform/AppInfo.h"
#include <QDesktopServices>
#include <QtTest>

class AppInfoTest : public QObject {
    Q_OBJECT
    QUrl received;
  public slots:
    void capture(const QUrl& url) {
        received = url;
    }
  private slots:
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
        QDesktopServices::unsetUrlHandler("https");
    }
};
QTEST_MAIN(AppInfoTest)
#include "tst_appinfo.moc"
