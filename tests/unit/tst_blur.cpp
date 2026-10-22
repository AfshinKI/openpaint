// SPDX-License-Identifier: GPL-3.0-or-later
#include "platform/Appearance.h"
#include "platform/BlurSupport.h"
#include <QClipboard>
#include <QGuiApplication>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

class BlurTest : public QObject {
    Q_OBJECT
    QStringList calls;
    bool installed = false;
    bool active = false;
    QString blur = "true";
    QString all = "false";
    QString whitelist = "['OpenPaint']";
    QString blacklist = "@as []";
    QString shell = "GNOME Shell 46.0";
    BlurSupport::Runner runner() {
        return [this](const QString& program, const QStringList& args) -> QString {
            calls << program;
            if (program == "gnome-shell")
                return shell;
            if (program == "gnome-extensions")
                return (args.contains("--active") ? active : installed) ? "blur-my-shell@aunetx"
                                                                        : "";
            if (program == "gsettings") {
                const auto key = args.last();
                if (key == "blur")
                    return blur;
                if (key == "enable-all")
                    return all;
                if (key == "whitelist")
                    return whitelist;
                if (key == "blacklist")
                    return blacklist;
            }
            return {};
        };
    }
    BlurSupport::Result evaluate() {
        return BlurSupport::evaluate("ubuntu:GNOME", "x11", "xcb", "ubuntu", "24.04", false, {},
                                     runner());
    }
  private slots:
    void init() {
        calls.clear();
        installed = active = false;
        blur = "true";
        all = "false";
        whitelist = "['OpenPaint']";
        blacklist = "@as []";
        shell = "GNOME Shell 46.0";
    }
    void unsupportedIsSilentAndDoesNotProbe() {
        auto result =
            BlurSupport::evaluate("KDE", "x11", "xcb", "ubuntu", "24.04", false, {}, runner());
        QVERIFY(!result.ready && result.command.isEmpty());
        result = BlurSupport::evaluate("GNOME", "wayland", "wayland", "ubuntu", "24.04", false, {},
                                       runner());
        QVERIFY(!result.ready && result.command.isEmpty());
        result =
            BlurSupport::evaluate("GNOME", "x11", "xcb", "ubuntu", "24.04", true, {}, runner());
        QVERIFY(!result.ready && result.command.isEmpty());
        QVERIFY(calls.isEmpty());
    }
    void supportedSessionOffersOneSetupBlock() {
        const auto result = evaluate();
        QVERIFY(!result.ready);
        QVERIFY(result.command.startsWith("/usr/bin/python3 <<'OPENPAINT_GLASS'"));
        QVERIFY(result.command.contains("extensions.gnome.org/download-extension/"));
        QVERIFY(result.command.contains("hashlib.sha256"));
        QVERIFY(!result.command.contains("sudo apt install"));
        QVERIFY(!calls.contains("apt-cache"));
        installed = true;
        QCOMPARE(evaluate().command, result.command);
        shell = "GNOME Shell 47.0";
        QVERIFY(evaluate().command.isEmpty());
    }
    void refreshAdvancesSetupToReady() {
        BlurSupport service([this] { return evaluate(); });
        QSignalSpy finished(&service, &BlurSupport::probeFinished);
        service.refresh();
        QTRY_COMPARE(finished.count(), 1);
        QVERIFY(service.setupAvailable());
        const auto setup = service.command();
        installed = true;
        service.refresh();
        QTRY_COMPARE(finished.count(), 2);
        QCOMPARE(service.command(), setup);
        installed = active = true;
        service.refresh();
        QTRY_COMPARE(finished.count(), 3);
        QVERIFY(service.ready());
        QVERIFY(!service.setupAvailable());
    }
    void activeExtensionNeedsApplicationSettings() {
        installed = true;
        QVERIFY(!evaluate().ready);
        QVERIFY(evaluate().command.startsWith("/usr/bin/python3"));
        active = true;
        QVERIFY(evaluate().ready);
        whitelist = "['OtherApp']";
        QVERIFY(!evaluate().ready);
        all = "true";
        QVERIFY(evaluate().ready);
        blacklist = "['OpenPaint']";
        QVERIFY(!evaluate().ready);
        blacklist = "['*']";
        QVERIFY(!evaluate().ready);
        blacklist.clear(); // Failed settings lookup is not support evidence.
        QVERIFY(!evaluate().ready);
        blacklist = "[]";
        blur = "false";
        QVERIFY(!evaluate().ready);
    }
    void normalExclusionsDoNotBlockAllowedApplication() {
        installed = active = true;
        blacklist = "['Plank', 'com.desktop.ding', 'Conky']";
        QVERIFY(evaluate().ready);
        blacklist = "['OpenPaint']";
        QVERIFY(evaluate().ready); // Exclusions are ignored in inclusion mode.
        all = "true";
        QVERIFY(!evaluate().ready);
        blacklist = "['Plank', 'com.desktop.ding', 'Conky']";
        QVERIFY(evaluate().ready);
        blacklist = "['open*']";
        QVERIFY(!evaluate().ready);
        blacklist = "['OpenPain?']";
        QVERIFY(!evaluate().ready);
        blacklist = "not a list";
        QVERIFY(!evaluate().ready);
        blacklist.clear();
        QVERIFY(!evaluate().ready);
        all = "false";
        whitelist = "['OPEN*']";
        QVERIFY(evaluate().ready);
        whitelist = "['OtherApp']";
        QVERIFY(!evaluate().ready);
        whitelist = "broken 'OpenPaint'";
        QVERIFY(!evaluate().ready);
    }
    void dismissalPersistsWithoutChangingAppearance() {
        QTemporaryDir directory;
        const auto path = directory.filePath("settings.ini");
        const auto probe = [] {
            return BlurSupport::Result{false, "test-command", "instructions"};
        };
        Appearance appearance(path);
        appearance.setSurfaceOpacity(62);
        QVERIFY(!appearance.effectiveGlassEnabled());
        BlurSupport service(probe);
        QSignalSpy finished(&service, &BlurSupport::probeFinished);
        service.refresh();
        QTRY_COMPARE(finished.size(), 1);
        QVERIFY(appearance.welcomePending());
        QVERIFY(service.copyCommand());
        QCOMPARE(QGuiApplication::clipboard()->text(), QString("test-command"));
        appearance.dismissWelcome();
        QVERIFY(!appearance.welcomePending());
        BlurSupport restored(probe);
        QSignalSpy restoredFinished(&restored, &BlurSupport::probeFinished);
        restored.refresh();
        QTRY_COMPARE(restoredFinished.size(), 1);
        QVERIFY(restored.setupAvailable());
        Appearance reopened(path);
        QVERIFY(!reopened.welcomePending());
        appearance.setDesktopBlurAvailable(true);
        QVERIFY(appearance.effectiveGlassEnabled());
        appearance.setDesktopBlurAvailable(false);
        QVERIFY(!appearance.effectiveGlassEnabled());
        Appearance saved(path);
        QVERIFY(saved.glassEnabled());
        QCOMPARE(saved.surfaceOpacity(), 62);
        service.setProbe([] { return BlurSupport::Result{}; });
        service.refresh();
        QTRY_COMPARE(finished.size(), 2);
        QVERIFY(!service.copyCommand());
        QCOMPARE(QGuiApplication::clipboard()->text(), QString("test-command"));
    }
};
QTEST_MAIN(BlurTest)
#include "tst_blur.moc"
