// SPDX-License-Identifier: GPL-3.0-or-later
#include "AppInfo.h"
#include "BuildConfig.h"
#include <QDesktopServices>
#include <QKeySequence>

QString AppInfo::version() {
    return QStringLiteral(OPENPAINT_VERSION);
}
QString AppInfo::channel() const {
    const auto channel = QStringLiteral(OPENPAINT_CHANNEL);
    return channel == "stable"       ? tr("Stable")
           : channel == "prerelease" ? tr("Prerelease")
                                     : tr("Development");
}
QString AppInfo::installation() const {
    const auto kind = QStringLiteral(OPENPAINT_INSTALL_KIND);
    if (kind == "macos")
        return tr("macOS application");
    if (kind == "portable")
        return tr("Portable archive");
    if (kind == "appimage")
        return tr("AppImage");
    if (kind == "flatpak")
        return tr("Flatpak test bundle");
    if (kind == "system")
        return tr("Distribution package");
    return tr("Source build");
}
QString AppInfo::instructions() const {
    const auto kind = QStringLiteral(OPENPAINT_INSTALL_KIND);
    if (kind == "macos")
        return tr("Download the newer Mac disk image from GitHub Releases. Save your work, "
                  "quit OpenPaint, then replace OpenPaint in Applications.");
    if (kind == "portable")
        return tr("Download and extract a newer portable archive from GitHub Releases. Save "
                  "your work and close OpenPaint before replacing the application folder.");
    if (kind == "appimage")
        return tr("Download a newer AppImage from GitHub Releases. Save your work and close "
                  "OpenPaint before replacing the file.");
    if (kind == "flatpak")
        return tr("This test bundle has no OpenPaint update remote. Download and install a newer "
                  "bundle manually. Flatpak manages the installed files.");
    if (kind == "system")
        return tr("Use your distribution's software manager. OpenPaint does not replace "
                  "package-managed files.");
    return tr("Build a newer release from source using the build guide, or follow your "
              "distribution's package instructions.");
}
QUrl AppInfo::releaseUrl() const {
    return QUrl(QStringLiteral("https://github.com/AfshinKI/openpaint/releases"));
}
bool AppInfo::openReleases() const {
    return QDesktopServices::openUrl(releaseUrl());
}

QString AppInfo::shortcutText(const QVariant& shortcut) const {
    if (shortcut.metaType() == QMetaType::fromType<QString>())
        return QKeySequence::fromString(shortcut.toString(), QKeySequence::PortableText)
            .toString(QKeySequence::NativeText);
    bool ok = false;
    const int key = shortcut.toInt(&ok);
    if (!ok || key < QKeySequence::UnknownKey || key > QKeySequence::Cancel)
        return {};
    return QKeySequence(static_cast<QKeySequence::StandardKey>(key))
        .toString(QKeySequence::NativeText);
}

bool AppInfo::openContributing() const {
    return QDesktopServices::openUrl(
        QUrl("https://github.com/AfshinKI/openpaint/blob/main/CONTRIBUTING.md"));
}

bool AppInfo::openWebsite() const {
    return QDesktopServices::openUrl(QUrl("https://afshinki.github.io/openpaint/"));
}

bool AppInfo::openContact() const {
    return QDesktopServices::openUrl(QUrl("https://github.com/AfshinKI/openpaint/issues"));
}
