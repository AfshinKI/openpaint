// SPDX-License-Identifier: GPL-3.0-or-later
#include "ColorSpectrum.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <algorithm>

ColorSpectrum::ColorSpectrum(QQuickItem* parent) : QQuickPaintedItem(parent) {
    setAcceptedMouseButtons(Qt::LeftButton);
    setAntialiasing(true);
}
void ColorSpectrum::setSelection(ColorSelection* selection) {
    if (selection_)
        disconnect(selection_, nullptr, this, nullptr);
    selection_ = selection;
    if (selection_)
        connect(selection_, &ColorSelection::changed, this, [this] { update(); });
    emit selectionChanged();
    update();
}
void ColorSpectrum::paint(QPainter* painter) {
    if (!selection_ || width() <= 0 || height() <= 0)
        return;
    QPainterPath outline;
    outline.addRoundedRect(boundingRect(), 4, 4);
    painter->setClipPath(outline);
    QLinearGradient hue(0, 0, width(), 0);
    for (int i = 0; i <= 6; ++i)
        hue.setColorAt(i / 6.0, QColor::fromHsvF((i % 6) / 6.0, 1, 1));
    painter->fillRect(boundingRect(), hue);
    QLinearGradient saturation(0, 0, 0, height());
    saturation.setColorAt(0, QColor(255, 255, 255, 0));
    saturation.setColorAt(1, Qt::white);
    painter->fillRect(boundingRect(), saturation);
    const QPointF point(selection_->hue() / 360 * width(),
                        (1 - selection_->saturation() / 100) * height());
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(Qt::black, 3));
    painter->drawEllipse(point, 5, 5);
    painter->setPen(QPen(Qt::white, 1.5));
    painter->drawEllipse(point, 5, 5);
}
void ColorSpectrum::selectAt(QPointF point) {
    if (!selection_ || !dragging_ || width() <= 0 || height() <= 0)
        return;
    selection_->setHsv(std::clamp(point.x() / width() * 360, 0.0, 359.99),
                       (1 - point.y() / height()) * 100, selection_->value());
}
void ColorSpectrum::mousePressEvent(QMouseEvent* event) {
    dragging_ = true;
    selectAt(event->position());
    event->accept();
}
void ColorSpectrum::mouseMoveEvent(QMouseEvent* event) {
    selectAt(event->position());
}
void ColorSpectrum::mouseReleaseEvent(QMouseEvent* event) {
    selectAt(event->position());
    dragging_ = false;
}
void ColorSpectrum::mouseUngrabEvent() {
    dragging_ = false;
}
