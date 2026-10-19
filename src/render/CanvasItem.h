// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "app/Editor.h"
#include <QPainterPath>
#include <QPointer>
#include <QQuickPaintedItem>

class CanvasItem : public QQuickPaintedItem {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(Editor* editor READ editor WRITE setEditor NOTIFY editorChanged)
    Q_PROPERTY(QPointF origin READ origin NOTIFY viewportChanged)
    Q_PROPERTY(qreal zoom READ zoom NOTIFY viewportChanged)
    Q_PROPERTY(QPointF cursorPosition READ cursorPosition NOTIFY cursorPositionChanged)
  public:
    explicit CanvasItem(QQuickItem* parent = nullptr);
    Editor* editor() const {
        return editor_;
    }
    void setEditor(Editor* editor);
    QPointF origin() const {
        return origin_;
    }
    qreal zoom() const {
        return zoom_;
    }
    QPointF cursorPosition() const {
        return cursor_;
    }
    QPointF toDocument(QPointF point) const {
        return (point - origin_) / zoom_;
    }
    QPointF fromDocument(QPointF point) const {
        return point * zoom_ + origin_;
    }
    Q_INVOKABLE void fit();
    Q_INVOKABLE void zoomBy(qreal factor);
    void zoomAt(qreal factor, QPointF anchor);
    void paint(QPainter* painter) override;

  protected:
    void geometryChange(const QRectF& now, const QRectF& before) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseUngrabEvent() override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
  signals:
    void editorChanged();
    void viewportChanged();
    void cursorPositionChanged();

  private:
    void updateToolCursor();
    void finishGesture();
    void updateGesture(QPointF point, bool constrain = false);
    QPointer<Editor> editor_;
    QRegion selectionCache_;
    QPainterPath selectionBoundary_;
    Qt::MouseButton gestureButton_ = Qt::NoButton;
    qreal zoom_ = 1;
    QPointF origin_, last_, cursor_, hoverPoint_;
    bool hovering_ = false;
    bool space_ = false, panning_ = false, drawing_ = false, fitted_ = true;
};
