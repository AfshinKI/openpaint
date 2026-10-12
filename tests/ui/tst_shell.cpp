// SPDX-License-Identifier: GPL-3.0-or-later
#include <QDir>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlError>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QSignalSpy>
#include <QTest>
#include <memory>

class ShellTest : public QObject {
    Q_OBJECT

  private slots:
    void init() {
        warnings.clear();
        engine = std::make_unique<QQmlApplicationEngine>();
        connect(engine.get(), &QQmlApplicationEngine::warnings, this,
                [this](const QList<QQmlError>& errors) {
                    for (const auto& error : errors)
                        warnings.append(error.toString());
                });
        engine->loadFromModule("OpenPaint", "Main");
        QCOMPARE(engine->rootObjects().size(), 1);
        window = qobject_cast<QQuickWindow*>(engine->rootObjects().first());
        QVERIFY(window);
        QVERIFY(QTest::qWaitForWindowExposed(window));
    }

    void cleanup() {
        engine.reset();
        window = nullptr;
        QVERIFY2(warnings.isEmpty(), qPrintable(warnings.join('\n')));
    }

    void launchAndResize() {
        QVERIFY(window->isVisible());
        QVERIFY(window->title().contains(QStringLiteral("OpenPaint")));
        QVERIFY(window->findChild<QObject*>(QStringLiteral("previewNotice")));
        window->resize(1024, 640);
        QTRY_COMPARE(window->size(), QSize(1024, 640));
        QVERIFY(!window->grabWindow().isNull());
    }

    void themeActions() {
        auto* light = window->findChild<QObject*>(QStringLiteral("lightThemeAction"));
        auto* dark = window->findChild<QObject*>(QStringLiteral("darkThemeAction"));
        auto* system = window->findChild<QObject*>(QStringLiteral("systemThemeAction"));
        auto* button = window->findChild<QObject*>(QStringLiteral("aboutButton"));
        QVERIFY(light && dark && system);
        QVERIFY(button);
        auto* label = button->property("contentItem").value<QObject*>();
        QVERIFY(label);
        QVERIFY(QMetaObject::invokeMethod(dark, "trigger", Q_ARG(QObject*, nullptr)));
        QTRY_COMPARE(window->color(), QColor("#111418"));
        QCOMPARE(label->property("color").value<QColor>(), QColor("#10211D"));
        QVERIFY(dark->property("checked").toBool());
        QVERIFY(!light->property("checked").toBool());
        QVERIFY(capture(QStringLiteral("shell-dark-1280.png")));
        window->resize(1024, 640);
        QVERIFY(capture(QStringLiteral("shell-dark-1024.png")));
        QVERIFY(QMetaObject::invokeMethod(light, "trigger", Q_ARG(QObject*, nullptr)));
        QTRY_COMPARE(window->color(), QColor("#E2E5EA"));
        QCOMPARE(label->property("color").value<QColor>(), QColor("#FFFFFF"));
        QVERIFY(light->property("checked").toBool());
        QVERIFY(!dark->property("checked").toBool());
        QVERIFY(capture(QStringLiteral("shell-light-1024.png")));
        window->resize(1280, 800);
        QVERIFY(capture(QStringLiteral("shell-light-1280.png")));
        QVERIFY(QMetaObject::invokeMethod(system, "trigger", Q_ARG(QObject*, nullptr)));
        QVERIFY(system->property("checked").toBool());
        QVERIFY(!light->property("checked").toBool());
    }

    void keyboardAboutAndEscape() {
        auto* dialog = window->findChild<QObject*>(QStringLiteral("aboutDialog"));
        QVERIFY(dialog);
        window->requestActivate();
        QVERIFY(QTest::qWaitForWindowActive(window));
        QTest::keyClick(window, Qt::Key_F1);
        QTRY_VERIFY(dialog->property("visible").toBool());
        QTest::keyClick(window, Qt::Key_Escape);
        QTRY_VERIFY(!dialog->property("visible").toBool());
        QVERIFY(window->isVisible());
    }

    void quitClosesWindow() {
        auto* quit = window->findChild<QObject*>(QStringLiteral("quitAction"));
        QVERIFY(quit);
        QVERIFY(QMetaObject::invokeMethod(quit, "trigger", Q_ARG(QObject*, nullptr)));
        QTRY_VERIFY(!window->isVisible());
    }

  private:
    // Optional synthetic evidence: capture only this test window, never the desktop.
    bool capture(const QString& name) {
        const auto directory = qEnvironmentVariable("OPENPAINT_CAPTURE_DIR");
        if (directory.isEmpty())
            return true;
        if (!QDir().mkpath(directory))
            return false;
        QSignalSpy frame(window, &QQuickWindow::frameSwapped);
        window->requestUpdate();
        if (!frame.wait(1000))
            return false;
        return window->grabWindow().save(QDir(directory).filePath(name));
    }

    std::unique_ptr<QQmlApplicationEngine> engine;
    QQuickWindow* window = nullptr;
    QStringList warnings;
};

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle(QStringLiteral("Basic"));
    ShellTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "tst_shell.moc"
