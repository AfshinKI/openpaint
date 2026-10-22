// SPDX-License-Identifier: GPL-3.0-or-later
#include "BlurSupport.h"
#include <QClipboard>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QGuiApplication>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QSysInfo>
#include <QtConcurrentRun>
#include <optional>

namespace {
const QString extension = QStringLiteral("blur-my-shell@aunetx");
QString runLocal(const QString& program, const QStringList& arguments) {
    const auto executable = QStandardPaths::findExecutable(program);
    if (executable.isEmpty())
        return {};
    QProcess process;
    auto environment = QProcessEnvironment::systemEnvironment();
    environment.insert("LC_ALL", "C");
    process.setProcessEnvironment(environment);
    process.setStandardErrorFile(QProcess::nullDevice());
    process.start(executable, arguments, QIODevice::ReadOnly);
    if (!process.waitForStarted(500))
        return {};
    QByteArray output;
    QElapsedTimer elapsed;
    elapsed.start();
    while (process.state() != QProcess::NotRunning && elapsed.elapsed() < 1500) {
        process.waitForReadyRead(50);
        output += process.readAllStandardOutput();
        if (output.size() > 65536)
            break;
    }
    output += process.readAllStandardOutput();
    if (process.state() != QProcess::NotRunning || output.size() > 65536) {
        process.kill();
        process.waitForFinished(500);
        return {};
    }
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0)
        return {};
    return QString::fromUtf8(output).trimmed();
}
std::optional<bool> matchesApplication(QString value) {
    // gsettings prints a GVariant string array. Unknown/escaped syntax is not
    // support evidence; never interpret a failed lookup as an empty exclusion.
    if (value.startsWith("@as "))
        value = value.mid(4).trimmed();
    const QString token = R"pattern('([^'\\]*)'|"([^"\\]*)")pattern";
    const QRegularExpression array("^\\[\\s*(?:(?:" + token + ")(?:\\s*,\\s*(?:" + token +
                                   "))*)?\\s*\\]$");
    if (!array.match(value).hasMatch())
        return std::nullopt;
    auto entries = QRegularExpression(token).globalMatch(value);
    while (entries.hasNext()) {
        const auto entry = entries.next();
        const auto pattern = entry.captured(1).isNull() ? entry.captured(2) : entry.captured(1);
        auto expression = QRegularExpression::escape(pattern);
        expression.replace("\\*", ".*").replace("\\?", ".");
        const QRegularExpression matcher("^" + expression + "$",
                                         QRegularExpression::CaseInsensitiveOption);
        // Qt's X11 WM_CLASS is OpenPaint (the application name).
        if (matcher.match("OpenPaint").hasMatch())
            return true;
    }
    return false;
}
} // namespace

BlurSupport::BlurSupport(QObject* parent) : BlurSupport(probeDesktop, parent) {}
BlurSupport::BlurSupport(Probe probe, QObject* parent) : QObject(parent), probe_(std::move(probe)) {
    connect(&watcher_, &QFutureWatcher<Result>::finished, this, [this] {
        result_ = watcher_.result();
        busy_ = false;
        emit changed();
        emit probeFinished();
    });
}
void BlurSupport::setProbe(Probe probe) {
    if (!busy())
        probe_ = std::move(probe);
}
void BlurSupport::refresh() {
    if (busy())
        return;
    busy_ = true;
    watcher_.setFuture(QtConcurrent::run(probe_));
    emit changed();
}
bool BlurSupport::copyCommand() {
    if (!setupAvailable() || !qGuiApp || !QGuiApplication::clipboard())
        return false;
    auto* clipboard = QGuiApplication::clipboard();
    clipboard->setText(command());
    return clipboard->text() == command();
}
BlurSupport::Result BlurSupport::probeDesktop() {
    QString schemas;
    for (const auto& root :
         QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation)) {
        const auto directory = root + "/gnome-shell/extensions/" + extension + "/schemas";
        if (QFile::exists(directory + "/gschemas.compiled")) {
            schemas = directory;
            break;
        }
    }
    return evaluate(
        qEnvironmentVariable("XDG_CURRENT_DESKTOP"), qEnvironmentVariable("XDG_SESSION_TYPE"),
        QGuiApplication::platformName(), QSysInfo::productType(), QSysInfo::productVersion(),
        QFile::exists("/.flatpak-info") || qEnvironmentVariableIsSet("SNAP"), schemas, runLocal);
}
BlurSupport::Result BlurSupport::evaluate(const QString& desktop, const QString& session,
                                          const QString& platform, const QString& osId,
                                          const QString& osVersion, bool sandboxed,
                                          const QString& schemaDirectory, const Runner& run) {
    Result result;
    // Only the independently documented GNOME 46 / Ubuntu 24.04 / X11 recipe.
    // No host escape or guessed instructions in sandboxes/other desktops.
    if (sandboxed || !desktop.split(':').contains("GNOME", Qt::CaseInsensitive) ||
        session != "x11" || platform != "xcb" || osId != "ubuntu" || osVersion != "24.04")
        return result;
    if (!run("gnome-shell", {"--version"}).startsWith("GNOME Shell 46."))
        return result;
    const auto installed = run("gnome-extensions", {"list"}).split('\n').contains(extension);
    const auto active =
        installed && run("gnome-extensions", {"list", "--active"}).split('\n').contains(extension);
    if (active) {
        auto get = [&](const QString& key) {
            QStringList args;
            if (!schemaDirectory.isEmpty())
                args << "--schemadir" << schemaDirectory;
            args << "get" << "org.gnome.shell.extensions.blur-my-shell.applications" << key;
            return run("gsettings", args);
        };
        const auto blur = get("blur");
        const auto all = get("enable-all");
        const auto whitelist = get("whitelist");
        const auto blacklist = get("blacklist");
        const auto included = matchesApplication(whitelist);
        const auto excluded = matchesApplication(blacklist);
        // Blur My Shell uses either inclusion mode or exclusion mode, not both.
        result.ready = blur == "true" && ((all == "false" && included.value_or(false)) ||
                                          (all == "true" && excluded.has_value() && !*excluded));
    }
    {
        QFile setup(":/qt/qml/OpenPaint/setup/enable-glass.py");
        if (setup.open(QIODevice::ReadOnly)) {
            result.command = "/usr/bin/python3 <<'OPENPAINT_GLASS'\n" +
                             QString::fromUtf8(setup.readAll()) + "OPENPAINT_GLASS";
            result.instructions =
                tr("Copy setup and paste it into a terminal. It downloads Blur My Shell "
                   "from GNOME Extensions only if missing, enables it and configures "
                   "wallpaper-based application blur, instead of live windows behind OpenPaint. "
                   "No sudo or Extension Manager is needed. "
                   "Then click Check again; if the terminal asks, sign out and back in first.");
        }
    }
    return result;
}
