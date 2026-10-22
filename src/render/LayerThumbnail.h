// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "app/Editor.h"
#include <QQuickPaintedItem>
class LayerThumbnail : public QQuickPaintedItem {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(Editor* editor READ editor WRITE setEditor NOTIFY changed)
    Q_PROPERTY(int layerIndex READ layerIndex WRITE setLayerIndex NOTIFY changed)
  public:
    explicit LayerThumbnail(QQuickItem* parent = nullptr) : QQuickPaintedItem(parent) {}
    Editor* editor() const {
        return editor_;
    }
    int layerIndex() const {
        return index_;
    }
    void setEditor(Editor* editor);
    void setLayerIndex(int index) {
        if (index_ != index) {
            index_ = index;
            update();
            emit changed();
        }
    }
    void paint(QPainter* painter) override;
  signals:
    void changed();

  private:
    QPointer<Editor> editor_;
    int index_ = 0;
};
