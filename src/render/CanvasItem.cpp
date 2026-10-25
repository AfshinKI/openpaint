// SPDX-License-Identifier: GPL-3.0-or-later
#include "CanvasItem.h"
#include "tools/Shape.h"
#include <QCursor>
#include <QImageReader>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QQuickWindow>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>

CanvasItem::CanvasItem(QQuickItem* parent) : QQuickPaintedItem(parent) {
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);
    setAcceptHoverEvents(true);
    setFlag(ItemIsFocusScope);
    setClip(true);
    setCursor(Qt::CrossCursor);
    connect(this, &QQuickItem::windowChanged, this, [this] { updateToolCursor(); });
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
        connect(editor_, &Editor::guidesChanged, this, [this] { update(); });
        connect(editor_, &Editor::documentReplaced, this, &CanvasItem::fit);
        connect(editor_, &Editor::settingsChanged, this, [this] {
            finishGesture();
            updateToolCursor();
            update();
        });
    }
    updateToolCursor();
    emit editorChanged();
    fit();
}
void CanvasItem::updateToolCursor() {
    if (space_ || panning_) {
        setCursor(panning_ ? Qt::ClosedHandCursor : Qt::OpenHandCursor);
        return;
    }
    QString symbol;
    const QPoint hotSpot(5, 5);
    if (editor_) {
        switch (editor_->tool()) {
        case Editor::Tool::Pencil:
            symbol = "pencil";
            break;
        case Editor::Tool::Eraser:
            symbol = editor_->transparentEraser ? "eraser" : "eraser-paper";
            break;
        case Editor::Tool::ColorPicker:
            symbol = "eyedropper";
            break;
        case Editor::Tool::Zoom:
            symbol = "zoom-region";
            break;
        case Editor::Tool::Text:
            symbol = "text";
            break;
        case Editor::Tool::Select:
            symbol = editor_->colorSelection      ? "wand"
                     : editor_->freeFormSelection ? "lasso"
                                                  : "selection";
            break;
        case Editor::Tool::Fill:
            symbol = editor_->fillAllMatching ? "fill-all" : "fill";
            break;
        case Editor::Tool::Shape:
            symbol = "shape-" + QString::number(editor_->shapeKind);
            break;
        case Editor::Tool::Brush:
            symbol = "brush";
            break;
        default:
            break;
        }
    }
    if (symbol.isEmpty()) {
        setCursor(Qt::CrossCursor);
        return;
    }
    const qreal scale = window() ? window()->devicePixelRatio() : 1;
    const QString key = symbol + QString::number(scale);
    // Native cursors are created and cached only on the GUI thread.
    static QHash<QString, QCursor> cursors;
    if (!cursors.contains(key)) {
        QImage image;
        if (symbol.startsWith("shape-")) {
            QImage base(28, 28, QImage::Format_ARGB32_Premultiplied);
            base.fill(Qt::transparent);
            const int kind = editor_->shapeKind;
            const QList<QPointF> vertices =
                kind == openpaint::SplineShape
                    ? QList<QPointF>{{4, 20}, {10, 6}, {18, 22}, {24, 8}}
                    : QList<QPointF>{{4, 4}, {24, 8}, {15, 15}, {24, 24}, {4, 24}};
            const auto patch = openpaint::renderShape(base, {4, 4}, {24, 24}, kind, Qt::black,
                                                      Qt::black, 2, true, false, 30, -30, vertices);
            {
                QPainter painter(&base);
                painter.drawImage(patch.rect.topLeft(), patch.image);
            }
            image = base.scaled(qRound(20 * scale), qRound(20 * scale), Qt::IgnoreAspectRatio,
                                Qt::SmoothTransformation);
        } else {
            QImageReader reader(":/qt/qml/OpenPaint/icons/" + symbol + ".svg");
            reader.setScaledSize(QSize(qRound(20 * scale), qRound(20 * scale)));
            image = reader.read();
        }
        if (image.isNull()) {
            setCursor(Qt::CrossCursor);
            return;
        }
        image.setDevicePixelRatio(scale);
        QPixmap glyph = QPixmap::fromImage(image);
        QPixmap halo = glyph;
        {
            QPainter painter(&halo);
            painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
            painter.fillRect(halo.rect(), Qt::white);
        }
        QPixmap bitmap(qRound(32 * scale), qRound(32 * scale));
        bitmap.setDevicePixelRatio(scale);
        bitmap.fill(Qt::transparent);
        {
            QPainter painter(&bitmap);
            for (int x = 9; x <= 11; ++x)
                for (int y = 9; y <= 11; ++y)
                    painter.drawPixmap(x, y, halo);
            painter.drawPixmap(10, 10, glyph);
            {
                // The cross marks the exact action point; the glyph stays out of its way.
                painter.setPen(QPen(Qt::white, 3));
                painter.drawLine(hotSpot + QPoint(-3, 0), hotSpot + QPoint(3, 0));
                painter.drawLine(hotSpot + QPoint(0, -3), hotSpot + QPoint(0, 3));
                painter.setPen(QPen(Qt::black, 1));
                painter.drawLine(hotSpot + QPoint(-3, 0), hotSpot + QPoint(3, 0));
                painter.drawLine(hotSpot + QPoint(0, -3), hotSpot + QPoint(0, 3));
            }
        }
        cursors.insert(key, QCursor(bitmap, hotSpot.x(), hotSpot.y()));
    }
    setCursor(cursors.value(key));
}
void CanvasItem::setFitLeftInset(qreal inset) {
    if (!std::isfinite(inset) || inset < 0 || inset == fitLeftInset_)
        return;
    fitLeftInset_ = inset;
    emit fitInsetsChanged();
}
void CanvasItem::fit() {
    if (!editor_ || width() <= 48 || height() <= 48)
        return;
    finishGesture();
    const qreal inset = std::min(fitLeftInset_, width() - 49);
    zoom_ = std::clamp(std::min((width() - 48 - inset) / editor_->imageSize().width(),
                                (height() - 48) / editor_->imageSize().height()),
                       0.01, 1.0);
    origin_ = {inset + (width() - inset - editor_->imageSize().width() * zoom_) / 2,
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
    if (zooming_) {
        const QRectF box(fromDocument(zoomStart_), fromDocument(zoomEnd_));
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(Qt::white, 1));
        painter->drawRect(box.normalized());
        painter->setPen(QPen(Qt::black, 1, Qt::DashLine));
        painter->drawRect(box.normalized());
    }
    const auto guides = editor_->guidePosition();
    painter->setPen(QPen(QColor("#e94ed7"), 1));
    if (guides.x() >= 0) {
        const qreal x = fromDocument({guides.x(), 0}).x();
        painter->drawLine(QPointF(x, target.top()), QPointF(x, target.bottom()));
    }
    if (guides.y() >= 0) {
        const qreal y = fromDocument({0, guides.y()}).y();
        painter->drawLine(QPointF(target.left(), y), QPointF(target.right(), y));
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
                         event->button() == Qt::LeftButton;
    if (picking)
        editor_->begin(toDocument(event->position()), event->button() == Qt::RightButton);
    gestureButton_ = event->button();
    last_ = event->position();
    panning_ = space_ || gestureButton_ == Qt::MiddleButton || gestureButton_ == Qt::RightButton;
    updateToolCursor();
    if (!panning_ && editor_ && editor_->tool() == Editor::Tool::Zoom) {
        zooming_ = !editor_->busy() && QRectF(editor_->image().rect()).contains(toDocument(last_));
        zoomStart_ = zoomEnd_ = toDocument(last_);
        update();
        event->accept();
        return;
    }
    drawing_ = !panning_ && !picking && editor_;
    if (drawing_)
        editor_->begin(toDocument(last_), gestureButton_ == Qt::RightButton);
    event->accept();
}
void CanvasItem::updateGesture(QPointF point, bool constrain, bool bypassSnap) {
    if (!std::isfinite(point.x()) || !std::isfinite(point.y()))
        return;
    if (panning_) {
        origin_ += point - last_;
        fitted_ = false;
        emit viewportChanged();
        update();
    } else if (zooming_ && editor_) {
        const auto doc = toDocument(point);
        zoomEnd_ = {std::clamp(doc.x(), 0.0, double(editor_->image().width())),
                    std::clamp(doc.y(), 0.0, double(editor_->image().height()))};
        update();
    } else if (drawing_ && editor_) {
        editor_->extend(toDocument(point), constrain, bypassSnap ? 0 : 6 / zoom_);
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
    updateGesture(event->position(), event->modifiers() & Qt::ShiftModifier,
                  event->modifiers() & Qt::ControlModifier);
}
void CanvasItem::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() != gestureButton_)
        return;
    updateGesture(event->position(), event->modifiers() & Qt::ShiftModifier,
                  event->modifiers() & Qt::ControlModifier);
    const bool applyZoom = zooming_;
    const QRectF region = QRectF(zoomStart_, zoomEnd_).normalized();
    const bool largeEnough = region.width() * zoom_ >= 4 && region.height() * zoom_ >= 4;
    finishGesture();
    if (applyZoom && largeEnough)
        zoomToRegion(region);
}
void CanvasItem::finishGesture() {
    if (zooming_) {
        zooming_ = false;
        update();
    }
    if (editor_)
        editor_->endGesture();
    drawing_ = panning_ = false;
    gestureButton_ = Qt::NoButton;
    updateToolCursor();
}
void CanvasItem::mouseUngrabEvent() {
    finishGesture();
}
void CanvasItem::hoverMoveEvent(QHoverEvent* event) {
    updateToolCursor();
    hovering_ = true;
    hoverPoint_ = event->position();
    cursor_ = toDocument(event->position());
    emit cursorPositionChanged();
    if (editor_ && editor_->tool() == Editor::Tool::ColorPicker)
        update();
}
void CanvasItem::hoverLeaveEvent(QHoverEvent* event) {
    hovering_ = false;
    emit cursorPositionChanged();
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
    if (event->key() == Qt::Key_Escape && zooming_) {
        finishGesture();
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Space) {
        space_ = true;
        updateToolCursor();
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
        updateToolCursor();
        event->accept();
    } else if (event->key() == Qt::Key_Escape && editor_) {
        editor_->cancel();
        drawing_ = panning_ = false;
        gestureButton_ = Qt::NoButton;
        updateToolCursor();
        event->accept();
    } else {
        QQuickPaintedItem::keyPressEvent(event);
    }
}
void CanvasItem::keyReleaseEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
        space_ = false;
        updateToolCursor();
    } else {
        QQuickPaintedItem::keyReleaseEvent(event);
    }
}
void CanvasItem::focusOutEvent(QFocusEvent* event) {
    finishGesture();
    space_ = false;
    updateToolCursor();
    QQuickPaintedItem::focusOutEvent(event);
}

void CanvasItem::zoomToRegion(QRectF region) {
    if (!editor_ || width() <= 48 || height() <= 48)
        return;
    region = region.intersected(QRectF(editor_->image().rect()));
    if (region.isEmpty())
        return;
    const qreal inset = std::min(fitLeftInset_, std::max(0.0, width() - 49));
    zoom_ = std::clamp(
        std::min((width() - inset - 48) / region.width(), (height() - 48) / region.height()), 0.01,
        32.0);
    origin_ = QPointF(inset + (width() - inset) / 2, height() / 2) - region.center() * zoom_;
    fitted_ = false;
    emit viewportChanged();
    update();
}
