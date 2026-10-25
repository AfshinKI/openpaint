// SPDX-License-Identifier: GPL-3.0-or-later
#include "Appearance.h"
#include <QGuiApplication>
#include <QIcon>
#include <QStyleHints>
#include <QWindow>
#include <algorithm>
Appearance::Appearance(QObject* parent) : Appearance(QString{}, parent) {}
Appearance::Appearance(const QString& file, QObject* parent) : QObject(parent) {
    settings_ = file.isEmpty()
                    ? std::make_unique<QSettings>(QSettings::IniFormat, QSettings::UserScope,
                                                  "OpenPaint", "OpenPaint")
                    : std::make_unique<QSettings>(file, QSettings::IniFormat);
    smartGuides_ = settings_->value("view/smartGuides", false).toBool();
    themeMode_ = std::clamp(settings_->value("appearance/theme", 0).toInt(), 0, 2);
    glassEnabled_ = settings_->value("appearance/glass", true).toBool();
    bool ok = false;
    const int value = settings_->value("appearance/opacity", 40).toInt(&ok);
    surfaceOpacity_ = ok ? std::clamp(value, 40, 100) : 40;
    if (qGuiApp) {
        systemDark_ = QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;
        connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this,
                [this](Qt::ColorScheme scheme) {
                    systemDark_ = scheme == Qt::ColorScheme::Dark;
                    updateApplicationIcon();
                    emit changed();
                });
        updateApplicationIcon();
    }
}
QUrl Appearance::applicationIcon() const {
    const bool dark = themeMode_ == 2 || (themeMode_ == 0 && systemDark_);
    return QUrl(dark ? "qrc:/qt/qml/OpenPaint/icons/app-dark.svg"
                     : "qrc:/qt/qml/OpenPaint/icons/app.svg");
}
void Appearance::updateApplicationIcon() {
    if (!qGuiApp)
        return;
    const QIcon icon(":" + applicationIcon().path());
    QGuiApplication::setWindowIcon(icon);
    for (auto* window : QGuiApplication::topLevelWindows())
        window->setIcon(icon);
}
void Appearance::save() {
    settings_->setValue("appearance/theme", themeMode_);
    settings_->setValue("appearance/glass", glassEnabled_);
    settings_->setValue("appearance/opacity", surfaceOpacity_);
    settings_->sync();
    updateApplicationIcon();
    emit changed();
}
void Appearance::setSmartGuides(bool enabled) {
    if (enabled == smartGuides_)
        return;
    smartGuides_ = enabled;
    settings_->setValue("view/smartGuides", enabled);
    settings_->sync();
    emit changed();
}
void Appearance::setGlassEnabled(bool enabled) {
    if (enabled == glassEnabled_)
        return;
    glassEnabled_ = enabled;
    save();
}
void Appearance::setSurfaceOpacity(int opacity) {
    opacity = std::clamp(opacity, 40, 100);
    if (opacity == surfaceOpacity_)
        return;
    surfaceOpacity_ = opacity;
    save();
}
void Appearance::restoreDefaults() {
    glassEnabled_ = true;
    surfaceOpacity_ = 40;
    save();
}

void Appearance::setThemeMode(int mode) {
    mode = std::clamp(mode, 0, 2);
    if (mode == themeMode_)
        return;
    themeMode_ = mode;
    save();
}

void Appearance::setDesktopBlurAvailable(bool available) {
    if (desktopBlurAvailable_ == available)
        return;
    desktopBlurAvailable_ = available;
    emit changed();
}

bool Appearance::welcomePending() const {
    return settings_->value("onboarding/welcomeRevision", 0).toInt() < 1;
}
void Appearance::dismissWelcome() {
    settings_->setValue("onboarding/welcomeRevision", 1);
    settings_->sync();
    emit changed();
}
