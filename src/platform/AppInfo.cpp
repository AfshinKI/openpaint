// SPDX-License-Identifier: GPL-3.0-or-later
#include "AppInfo.h"
#include "BuildConfig.h"
#include <QDesktopServices>

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
