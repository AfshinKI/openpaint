// SPDX-License-Identifier: GPL-3.0-or-later
#include "ColorWheel.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <cmath>
#include <numbers>
ColorWheel::ColorWheel(QQuickItem* parent) : QQuickPaintedItem(parent) {
    setAcceptedMouseButtons(Qt::LeftButton);
    setAntialiasing(true);
}
void ColorWheel::setSelection(ColorSelection* selection) {
    if (selection_)
        disconnect(selection_, nullptr, this, nullptr);
    selection_ = selection;
    if (selection_)
        connect(selection_, &ColorSelection::changed, this, [this] { update(); });
    emit selectionChanged();
    update();
}
QRectF ColorWheel::svRect() const {
    const auto side = std::min(width(), height()) * 0.52;
    return {(width() - side) / 2, (height() - side) / 2, side, side};
}
void ColorWheel::paint(QPainter* painter) {
    if (!selection_)
        return;
    const QPointF center(width() / 2, height() / 2);
    const qreal radius = std::min(width(), height()) / 2 - 6;
    if (radius <= 24)
        return;
    QConicalGradient hue(center, 90);
    for (int i = 0; i <= 12; ++i)
        hue.setColorAt(i / 12.0, QColor::fromHsvF((12 - i) % 12 / 12.0, 1, 1));
    QPainterPath ring;
    ring.addEllipse(center, radius, radius);
    ring.addEllipse(center, radius - 24, radius - 24);
    painter->fillPath(ring, hue);
    const auto square = svRect();
    QLinearGradient saturation(square.topLeft(), square.topRight());
    saturation.setColorAt(0, Qt::white);
    saturation.setColorAt(1, QColor::fromHsvF(selection_->hue() / 360, 1, 1));
    painter->fillRect(square, saturation);
    QLinearGradient value(square.topLeft(), square.bottomLeft());
    value.setColorAt(0, QColor(0, 0, 0, 0));
    value.setColorAt(1, Qt::black);
    painter->fillRect(square, value);
    const qreal angle = (selection_->hue() - 90) * std::numbers::pi / 180;
    const QPointF huePoint = center + QPointF(std::cos(angle), std::sin(angle)) * (radius - 12);
    const QPointF svPoint(square.left() + square.width() * selection_->saturation() / 100,
                          square.bottom() - square.height() * selection_->value() / 100);
    for (const auto& point : {huePoint, svPoint}) {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(Qt::black, 3));
        painter->drawEllipse(point, 6, 6);
        painter->setPen(QPen(Qt::white, 1.5));
        painter->drawEllipse(point, 6, 6);
    }
}
void ColorWheel::selectAt(QPointF point) {
    if (!selection_)
        return;
    if (drag_ == Drag::Hue) {
        const auto hue =
            std::atan2(point.y() - height() / 2, point.x() - width() / 2) * 180 / std::numbers::pi +
            90;
        selection_->setHsv(hue, selection_->saturation(), selection_->value());
    } else if (drag_ == Drag::SV) {
        const auto square = svRect();
        if (square.width() > 0)
            selection_->setHsv(selection_->hue(),
                               (point.x() - square.left()) / square.width() * 100,
                               (square.bottom() - point.y()) / square.height() * 100);
    }
}
void ColorWheel::mousePressEvent(QMouseEvent* event) {
    const auto point = event->position();
    const qreal radius = std::min(width(), height()) / 2 - 6;
    const qreal distance = QLineF(point, {width() / 2, height() / 2}).length();
    if (svRect().contains(point))
        drag_ = Drag::SV;
    else if (distance >= radius - 24 && distance <= radius)
        drag_ = Drag::Hue;
    else {
        event->ignore();
        return;
    }
    selectAt(point);
    event->accept();
}
void ColorWheel::mouseMoveEvent(QMouseEvent* event) {
    selectAt(event->position());
}
void ColorWheel::mouseReleaseEvent(QMouseEvent* event) {
    selectAt(event->position());
    drag_ = Drag::None;
}
void ColorWheel::mouseUngrabEvent() {
    drag_ = Drag::None;
}
