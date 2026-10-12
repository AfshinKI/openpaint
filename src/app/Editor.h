// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "document/Document.h"
#include <QObject>
#include <QtQml/qqmlregistration.h>

class Editor : public QObject {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool dirty READ dirty NOTIFY changed)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY changed)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY changed)
    Q_PROPERTY(QSize imageSize READ imageSize NOTIFY changed)
    Q_PROPERTY(QColor primary MEMBER primary NOTIFY settingsChanged)
    Q_PROPERTY(QColor secondary MEMBER secondary NOTIFY settingsChanged)
    Q_PROPERTY(int brushSize MEMBER brushSize NOTIFY settingsChanged)
    Q_PROPERTY(bool eraser MEMBER eraser NOTIFY settingsChanged)
    Q_PROPERTY(bool transparentEraser MEMBER transparentEraser NOTIFY settingsChanged)
  public:
    explicit Editor(QObject* parent = nullptr) : QObject(parent) {}
    bool dirty() const {
        return document.dirty() || document.active();
    }
    bool canUndo() const {
        return document.canUndo();
    }
    bool canRedo() const {
        return document.canRedo();
    }
    QSize imageSize() const {
        return document.image().size();
    }
    const QImage& image() const {
        return document.image();
    }
    void begin(QPointF point, bool secondaryButton);
    void extend(QPointF point);
    Q_INVOKABLE void finish();
    Q_INVOKABLE void cancel();
    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();
    QColor primary = Qt::black, secondary = Qt::white;
    int brushSize = 3;
    bool eraser = false, transparentEraser = false;
  signals:
    void changed();
    void settingsChanged();
    void pixelsChanged(QRect rect);

  protected:
    openpaint::Document document;
};
