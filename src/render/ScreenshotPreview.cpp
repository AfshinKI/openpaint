// SPDX-License-Identifier: GPL-3.0-or-later
#include "ScreenshotPreview.h"
#include <QPainter>
#include <QPainterPath>
#include <algorithm>

ScreenshotPreview::ScreenshotPreview(QQuickItem* parent) : QQuickPaintedItem(parent) {
    setAntialiasing(true);
    loadImage();
}
void ScreenshotPreview::loadImage() {
    image_.load(glass_ ? ":/qt/qml/OpenPaint/previews/glass.png"
                       : ":/qt/qml/OpenPaint/previews/opaque.png");
    update();
}
void ScreenshotPreview::setGlass(bool glass) {
    if (glass_ == glass)
        return;
    glass_ = glass;
    loadImage();
    emit changed();
}
void ScreenshotPreview::setCorner(bool corner) {
    if (corner_ == corner)
        return;
    corner_ = corner;
    update();
    emit changed();
}
void ScreenshotPreview::setAngled(bool angled) {
    if (angled_ == angled)
        return;
    angled_ = angled;
    update();
    emit changed();
}
void ScreenshotPreview::paint(QPainter* painter) {
    const QRectF card = boundingRect().adjusted(9, 5, -9, -13);
    if (card.width() <= 0 || card.height() <= 0 || image_.isNull())
        return;
    painter->save();
    if (angled_) {
        const qreal w = width(), h = height();
        QTransform perspective;
        const QPolygonF target{QPointF(15, h * .17), QPointF(w - 30, 14), QPointF(w - 12, h * .81),
                               QPointF(35, h - 18)};
        if (QTransform::quadToQuad(
                QPolygonF{card.topLeft(), card.topRight(), card.bottomRight(), card.bottomLeft()},
                target, perspective))
            painter->setTransform(perspective, true);
    }
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->setPen(Qt::NoPen);
    // Several soft low-alpha silhouettes form a bounded shadow without shaders.
    for (int spread = 7; spread >= 1; --spread) {
        painter->setBrush(QColor(0, 0, 0, 4));
        painter->drawRoundedRect(card.adjusted(-spread, -spread, spread, spread).translated(0, 4),
                                 18 + spread, 18 + spread);
    }
    QPainterPath mask;
    mask.addRoundedRect(card, 18, 18);
    painter->save();
    painter->setClipPath(mask);
    QRectF source = corner_ ? QRectF(0, glass_ ? 37 : 0, 620, 330) : image_.rect();
    const double scale = std::max(card.width() / source.width(), card.height() / source.height());
    const QSizeF cropped(card.width() / scale, card.height() / scale);
    if (!corner_)
        source.moveLeft(source.left() + (source.width() - cropped.width()) / 2);
    source.setSize(cropped);
    painter->drawImage(card, image_, source);
    painter->restore();
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QColor(128, 128, 128, 45));
    painter->drawRoundedRect(card, 18, 18);
    painter->restore();
}
