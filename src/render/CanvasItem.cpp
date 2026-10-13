// SPDX-License-Identifier: GPL-3.0-or-later
#include "CanvasItem.h"
#include <QCursor>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>

CanvasItem::CanvasItem(QQuickItem* parent) : QQuickPaintedItem(parent) {
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);
    setAcceptHoverEvents(true);
    setFlag(ItemIsFocusScope);
    setClip(true);
    setCursor(Qt::CrossCursor);
}
void CanvasItem::setEditor(Editor* editor) {
    if (editor_ == editor)
        return;
    finishGesture();
    if (editor_)
        editor_->finish();
    if (editor_)
        disconnect(editor_, nullptr, this, nullptr);
    editor_ = editor;
    if (editor_) {
        connect(editor_, &Editor::pixelsChanged, this, [this](QRect rect) {
            if (rect.isEmpty())
                update();
            else
                update(QRectF(fromDocument(rect.topLeft()), QSizeF(rect.size()) * zoom_)
                           .adjusted(-2, -2, 2, 2)
                           .toAlignedRect());
        });
        connect(editor_, &Editor::documentReplaced, this, &CanvasItem::fit);
        connect(editor_, &Editor::settingsChanged, this, [this] {
            finishGesture();
            update();
        });
    }
    emit editorChanged();
    fit();
}
void CanvasItem::fit() {
    if (!editor_ || width() <= 48 || height() <= 48)
        return;
    finishGesture();
    zoom_ = std::clamp(std::min((width() - 48) / editor_->imageSize().width(),
                                (height() - 48) / editor_->imageSize().height()),
                       0.01, 1.0);
    origin_ = {(width() - editor_->imageSize().width() * zoom_) / 2,
               (height() - editor_->imageSize().height() * zoom_) / 2};
    fitted_ = true;
    emit viewportChanged();
    update();
}
void CanvasItem::zoomBy(qreal factor) {
    zoomAt(factor, {width() / 2, height() / 2});
}
void CanvasItem::zoomAt(qreal factor, QPointF anchor) {
    if (!std::isfinite(factor) || factor <= 0 || !std::isfinite(anchor.x()) ||
        !std::isfinite(anchor.y()))
        return;
    finishGesture();
    const auto documentPoint = toDocument(anchor);
    zoom_ = std::clamp(zoom_ * factor, 0.01, 32.0);
    origin_ = anchor - documentPoint * zoom_;
    fitted_ = false;
    emit viewportChanged();
    update();
}
void CanvasItem::geometryChange(const QRectF& now, const QRectF& before) {
    QQuickPaintedItem::geometryChange(now, before);
    finishGesture();
    if (fitted_)
        fit();
}
void CanvasItem::paint(QPainter* painter) {
    if (!editor_)
        return;
    // Qt calls paint during scene-graph synchronization while the GUI thread is blocked.
    // No worker mutates the document; do not retain a snapshot between pointer events.
    const auto& image = editor_->displayImage();
    const QRectF target(origin_, QSizeF(image.size()) * zoom_);
    QImage checks(16, 16, QImage::Format_RGB32);
    checks.fill(QColor("#ffffff"));
    QPainter tile(&checks);
    tile.fillRect(0, 0, 8, 8, QColor("#d9d9d9"));
    tile.fillRect(8, 8, 8, 8, QColor("#d9d9d9"));
    tile.end();
    painter->fillRect(target, QBrush(checks));
    painter->setRenderHint(QPainter::SmoothPixmapTransform, zoom_ < 1);
    painter->drawImage(target, image);
    painter->setPen(QColor("#9299a3"));
    painter->drawRect(target.adjusted(-1, -1, 1, 1));
    if (editor_->hasSelection()) {
        const auto offset = editor_->floatingActive() ? editor_->floatingPosition() : QPoint();
        const auto localRegion = editor_->selection().translated(-offset);
        if (selectionCache_ != localRegion) {
            selectionCache_ = localRegion;
            QPainterPath path;
            path.addRegion(localRegion);
            selectionBoundary_ = path.simplified();
        }
        QTransform transform;
        transform.translate(origin_.x(), origin_.y());
        transform.scale(zoom_, zoom_);
        transform.translate(offset.x(), offset.y());
        const auto boundary = transform.map(selectionBoundary_);
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(Qt::white, 1));
        painter->drawPath(boundary);
        painter->setPen(QPen(Qt::black, 1, Qt::DashLine));
        painter->drawPath(boundary);
    }
    if (!editor_->selectionDraft().isEmpty()) {
        QPolygonF path;
        for (const auto point : editor_->selectionDraft())
            path.append(fromDocument(point));
        painter->setPen(QPen(Qt::white, 1));
        painter->drawPolyline(path);
        painter->setPen(QPen(Qt::black, 1, Qt::DashLine));
        painter->drawPolyline(path);
    }
    const auto samplePoint = toDocument(hoverPoint_);
    if (hovering_ && !space_ && !panning_ && !editor_->busy() &&
        editor_->tool() == Editor::Tool::ColorPicker &&
        QRectF(image.rect()).contains(samplePoint)) {
        const auto tip = hoverPoint_;
        const QRectF swatch(tip + QPointF(12, 16), QSizeF(28, 28));
        painter->fillRect(swatch, QBrush(checks));
        painter->fillRect(swatch, image.pixelColor(int(std::floor(samplePoint.x())),
                                                   int(std::floor(samplePoint.y()))));
        painter->setPen(QPen(Qt::black, 1));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(swatch);
        painter->setPen(QPen(Qt::white, 1));
        painter->drawRect(swatch.adjusted(-1, -1, 1, 1));
    }
}
void CanvasItem::mousePressEvent(QMouseEvent* event) {
    // A chord cannot steal an existing gesture or change its drawing color.
    if (gestureButton_ != Qt::NoButton) {
        event->accept();
        return;
    }
    if (!(acceptedMouseButtons() & event->button()) || !std::isfinite(event->position().x()) ||
        !std::isfinite(event->position().y())) {
        event->ignore();
        return;
    }
    if (editor_ && editor_->textReady() && event->button() == Qt::LeftButton && !space_ &&
        !QRectF(editor_->textRect()).contains(toDocument(event->position()))) {
        editor_->commitPreview();
        forceActiveFocus();
        event->accept();
        return; // Applying consumes the click; it must not create another text box.
    }
    forceActiveFocus();
    // Sampling emits settingsChanged synchronously. Do it before establishing the
    // gesture so the settings handler cannot release ownership of this press.
    const bool picking = editor_ && editor_->tool() == Editor::Tool::ColorPicker && !space_ &&
                         event->button() != Qt::MiddleButton;
    if (picking)
        editor_->begin(toDocument(event->position()), event->button() == Qt::RightButton);
    gestureButton_ = event->button();
    last_ = event->position();
    panning_ = space_ || gestureButton_ == Qt::MiddleButton;
    drawing_ = !panning_ && !picking && editor_;
    if (drawing_)
        editor_->begin(toDocument(last_), gestureButton_ == Qt::RightButton);
    event->accept();
}
void CanvasItem::updateGesture(QPointF point, bool constrain) {
    if (!std::isfinite(point.x()) || !std::isfinite(point.y()))
        return;
    if (panning_) {
        origin_ += point - last_;
        fitted_ = false;
        emit viewportChanged();
        update();
    } else if (drawing_ && editor_) {
        editor_->extend(toDocument(point), constrain);
    }
    last_ = point;
    hoverPoint_ = point;
    cursor_ = toDocument(point);
    emit cursorPositionChanged();
    if (editor_ && editor_->tool() == Editor::Tool::ColorPicker)
        update();
}
void CanvasItem::mouseMoveEvent(QMouseEvent* event) {
    if (gestureButton_ != Qt::NoButton && !(event->buttons() & gestureButton_)) {
        // A missing release must not connect the stroke to a later hover position.
        finishGesture();
        return;
    }
    updateGesture(event->position(), event->modifiers() & Qt::ShiftModifier);
}
void CanvasItem::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() != gestureButton_)
        return;
    updateGesture(event->position(), event->modifiers() & Qt::ShiftModifier);
    finishGesture();
}
void CanvasItem::finishGesture() {
    if (editor_)
        editor_->endGesture();
    drawing_ = panning_ = false;
    gestureButton_ = Qt::NoButton;
}
void CanvasItem::mouseUngrabEvent() {
    finishGesture();
}
void CanvasItem::hoverMoveEvent(QHoverEvent* event) {
    hovering_ = true;
    hoverPoint_ = event->position();
    cursor_ = toDocument(event->position());
    emit cursorPositionChanged();
    if (editor_ && editor_->tool() == Editor::Tool::ColorPicker)
        update();
}
void CanvasItem::hoverLeaveEvent(QHoverEvent* event) {
    hovering_ = false;
    if (editor_ && editor_->tool() == Editor::Tool::ColorPicker)
        update();
    QQuickPaintedItem::hoverLeaveEvent(event);
}
void CanvasItem::wheelEvent(QWheelEvent* event) {
    if (event->modifiers() & Qt::ControlModifier) {
        zoomAt(std::pow(1.2, event->angleDelta().y() / 120.0), event->position());
    } else {
        finishGesture();
        origin_ += event->pixelDelta().isNull() ? QPointF(event->angleDelta()) / 3
                                                : QPointF(event->pixelDelta());
        fitted_ = false;
        emit viewportChanged();
        update();
    }
    event->accept();
}
void CanvasItem::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Space) {
        space_ = true;
        setCursor(Qt::OpenHandCursor);
        event->accept();
    } else if (editor_ && editor_->hasSelection() &&
               (!editor_->previewActive() || editor_->floatingActive()) &&
               (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right ||
                event->key() == Qt::Key_Up || event->key() == Qt::Key_Down)) {
        const int step = event->modifiers() & Qt::ShiftModifier ? 10 : 1;
        editor_->nudgeSelection(event->key() == Qt::Key_Left    ? -step
                                : event->key() == Qt::Key_Right ? step
                                                                : 0,
                                event->key() == Qt::Key_Up     ? -step
                                : event->key() == Qt::Key_Down ? step
                                                               : 0);
        event->accept();
    } else if (event->key() == Qt::Key_Backspace && editor_ && editor_->previewActive()) {
        editor_->removeShapePoint();
        event->accept();
    } else if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) && editor_ &&
               editor_->previewActive() &&
               (!editor_->textActive() || (event->modifiers() & Qt::ControlModifier))) {
        editor_->finish();
        drawing_ = panning_ = false;
        gestureButton_ = Qt::NoButton;
        event->accept();
    } else if (event->key() == Qt::Key_Escape && editor_) {
        editor_->cancel();
        drawing_ = panning_ = false;
        gestureButton_ = Qt::NoButton;
        event->accept();
    } else {
        QQuickPaintedItem::keyPressEvent(event);
    }
}
void CanvasItem::keyReleaseEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
        space_ = false;
        setCursor(Qt::CrossCursor);
    } else {
        QQuickPaintedItem::keyReleaseEvent(event);
    }
}
void CanvasItem::focusOutEvent(QFocusEvent* event) {
    finishGesture();
    space_ = false;
    setCursor(Qt::CrossCursor);
    QQuickPaintedItem::focusOutEvent(event);
}
