// SPDX-License-Identifier: GPL-3.0-or-later
#include "Shape.h"
#include <QPainter>
#include <cmath>
namespace openpaint {
ShapePatch renderShape(const QImage& base, QPointF start, QPointF end, int kind,
                       QColor outlineColor, QColor fillColor, int width, bool outline, bool fill) {
    if (base.isNull() || kind < 0 || kind > 2 || width < 1 || width > 128 ||
        !outlineColor.isValid() || !fillColor.isValid())
        return {};
    for (qreal value : {start.x(), start.y(), end.x(), end.y()})
        if (!std::isfinite(value) || std::abs(value) > 1000000)
            return {};
    const auto bounds = QRectF(start, end).normalized();
    const qreal margin = width / 2.0 + 2;
    const auto rect =
        bounds.adjusted(-margin, -margin, margin, margin).toAlignedRect().intersected(base.rect());
    if (rect.isEmpty())
        return {};
    QImage image = base.copy(rect);
    QPainter painter(&image);
    painter.translate(-rect.topLeft());
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(outline ? QPen(outlineColor, width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)
                           : QPen(Qt::NoPen));
    painter.setBrush(fill ? QBrush(fillColor) : Qt::NoBrush);
    if (kind == 0) {
        if (outline)
            painter.drawLine(start, end);
    } else if (kind == 1)
        painter.drawRect(bounds);
    else
        painter.drawEllipse(bounds);
    painter.end();
    return {image, rect};
}
} // namespace openpaint
