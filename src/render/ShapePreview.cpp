// SPDX-License-Identifier: GPL-3.0-or-later
#include "ShapePreview.h"
#include "tools/Shape.h"
#include <QPainter>
ShapePreview::ShapePreview(QQuickItem* parent) : QQuickPaintedItem(parent) {
    connect(this, &ShapePreview::changed, this, [this] { update(); });
}
void ShapePreview::paint(QPainter* painter) {
    QImage base(28, 28, QImage::Format_ARGB32_Premultiplied);
    base.fill(Qt::transparent);
    const QList<QPointF> vertices =
        kind == openpaint::SplineShape
            ? QList<QPointF>{{4, 20}, {10, 6}, {18, 22}, {24, 8}}
            : QList<QPointF>{{4, 4}, {24, 8}, {15, 15}, {24, 24}, {4, 24}};
    const auto patch = openpaint::renderShape(base, {4, 4}, {24, 24}, kind, color, color, 2, true,
                                              false, 30, -30, vertices);
    painter->scale(width() / 28, height() / 28);
    painter->drawImage(patch.rect.topLeft(), patch.image);
}
