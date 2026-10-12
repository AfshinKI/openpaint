// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QColor>
#include <QImage>
#include <QPointF>
#include <QRect>
namespace openpaint {
struct ShapePatch {
    QImage image;
    QRect rect;
};
ShapePatch renderShape(const QImage& base, QPointF start, QPointF end, int kind,
                       QColor outlineColor, QColor fillColor, int width, bool outline, bool fill);
} // namespace openpaint
