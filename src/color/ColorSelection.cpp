// SPDX-License-Identifier: GPL-3.0-or-later
#include "ColorSelection.h"
#include <QRegularExpression>
#include <algorithm>
#include <cmath>
void ColorSelection::setColor(QColor color) {
    if (!color.isValid())
        return;
    color.setAlpha(255);
    color_ = color.toRgb();
    if (color_.hsvHueF() >= 0)
        hue_ = color_.hsvHueF() * 360;
    saturation_ = color_.hsvSaturationF() * 100;
    value_ = color_.valueF() * 100;
    emit changed();
}
bool ColorSelection::setHex(const QString& text) {
    static const QRegularExpression valid("^#?[0-9a-fA-F]{6}$");
    if (!valid.match(text).hasMatch())
        return false;
    setColor(QColor(text.startsWith('#') ? text : '#' + text));
    return true;
}
void ColorSelection::setHsv(qreal hue, qreal saturation, qreal value) {
    if (!std::isfinite(hue) || !std::isfinite(saturation) || !std::isfinite(value))
        return;
    hue_ = std::fmod(std::fmod(hue, 360) + 360, 360);
    saturation_ = std::clamp(saturation, 0.0, 100.0);
    value_ = std::clamp(value, 0.0, 100.0);
    color_ = QColor::fromHsvF(hue_ / 360, saturation_ / 100, value_ / 100).toRgb();
    emit changed();
}
void ColorSelection::setRgb(int red, int green, int blue) {
    setColor(QColor(std::clamp(red, 0, 255), std::clamp(green, 0, 255), std::clamp(blue, 0, 255)));
}
