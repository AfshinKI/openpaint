// SPDX-License-Identifier: GPL-3.0-or-later
#include "BrushPreview.h"
#include "tools/Brush.h"
#include <QPainter>
BrushPreview::BrushPreview(QQuickItem* parent) : QQuickPaintedItem(parent) {
    connect(this, &BrushPreview::changed, this, [this] { update(); });
}
void BrushPreview::paint(QPainter* painter) {
    painter->drawImage(boundingRect(), openpaint::Brush::preview(style, color));
}
