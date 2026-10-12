// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "ColorSelection.h"
#include <QPointer>
#include <QQuickPaintedItem>

class ColorWheel : public QQuickPaintedItem {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(ColorSelection* selection READ selection WRITE setSelection NOTIFY selectionChanged)
  public:
    explicit ColorWheel(QQuickItem* parent = nullptr);
    ColorSelection* selection() const {
        return selection_;
    }
    void setSelection(ColorSelection* selection);
    void paint(QPainter* painter) override;
    QRectF svRect() const;

  protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseUngrabEvent() override;
  signals:
    void selectionChanged();

  private:
    void selectAt(QPointF point);
    QPointer<ColorSelection> selection_;
    enum class Drag { None, Hue, SV } drag_ = Drag::None;
};
