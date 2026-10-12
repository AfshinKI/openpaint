// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QQuickPaintedItem>
#include <QtQml/qqmlregistration.h>
class ShapePreview : public QQuickPaintedItem {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(int kind MEMBER kind NOTIFY changed)
    Q_PROPERTY(QColor color MEMBER color NOTIFY changed)
  public:
    explicit ShapePreview(QQuickItem* parent = nullptr);
    void paint(QPainter* painter) override;
    int kind = 0;
    QColor color = Qt::black;
  signals:
    void changed();
};
