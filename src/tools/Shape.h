// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QColor>
#include <QImage>
#include <QPointF>
#include <QRect>
namespace openpaint {
inline constexpr int ShapeCount = 20, CurveShape = 14, PolygonShape = 15, SplineShape = 19;
inline bool isPointShape(int kind) {
    return kind == PolygonShape || kind == SplineShape;
}
struct ShapePatch {
    QImage image;
    QRect rect;
};
ShapePatch renderShape(const QImage& base, QPointF start, QPointF end, int kind,
                       QColor outlineColor, QColor fillColor, int width, bool outline, bool fill,
                       qreal bend1 = 30, qreal bend2 = -30, const QList<QPointF>& vertices = {});
} // namespace openpaint
