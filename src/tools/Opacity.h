// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QColor>
namespace openpaint {
// Interpolate premultiplied pixels so operation opacity preserves transparency.
inline QRgb operationOpacity(QRgb before, QRgb after, int opacity) {
    const auto mix = [opacity](int a, int b) {
        return (a * (100 - opacity) + b * opacity + 50) / 100;
    };
    return qRgba(mix(qRed(before), qRed(after)), mix(qGreen(before), qGreen(after)),
                 mix(qBlue(before), qBlue(after)), mix(qAlpha(before), qAlpha(after)));
}
} // namespace openpaint
