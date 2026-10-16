// SPDX-License-Identifier: GPL-3.0-or-later
#include "Appearance.h"
#include <algorithm>
Appearance::Appearance(QObject* parent) : Appearance(QString{}, parent) {}
Appearance::Appearance(const QString& file, QObject* parent) : QObject(parent) {
    settings_ = file.isEmpty()
                    ? std::make_unique<QSettings>(QSettings::IniFormat, QSettings::UserScope,
                                                  "OpenPaint", "OpenPaint")
                    : std::make_unique<QSettings>(file, QSettings::IniFormat);
    glassEnabled_ = settings_->value("appearance/glass", true).toBool();
    bool ok = false;
    const int value = settings_->value("appearance/opacity", 85).toInt(&ok);
    surfaceOpacity_ = ok ? std::clamp(value, 40, 100) : 85;
}
void Appearance::save() {
    settings_->setValue("appearance/glass", glassEnabled_);
    settings_->setValue("appearance/opacity", surfaceOpacity_);
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
    surfaceOpacity_ = 85;
    save();
}
