// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QQuickPaintedItem>
#include <QtQml/qqmlregistration.h>
class BrushPreview : public QQuickPaintedItem {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(int style MEMBER style NOTIFY changed)
    Q_PROPERTY(QColor color MEMBER color NOTIFY changed)
  public:
    explicit BrushPreview(QQuickItem* parent = nullptr);
    void paint(QPainter* painter) override;
    int style = 0;
    QColor color = Qt::black;
  signals:
    void changed();
};
