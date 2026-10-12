// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "document/Document.h"
#include "io/PngIo.h"
#include "tools/Fill.h"
#include <QFutureWatcher>
#include <QObject>
#include <QUrl>
#include <QtQml/qqmlregistration.h>

class Editor : public QObject {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool busy READ busy NOTIFY changed)
    Q_PROPERTY(bool filling READ filling NOTIFY changed)
    Q_PROPERTY(bool previewActive READ previewActive NOTIFY changed)
    Q_PROPERTY(QPointF shapeStart READ shapeStart NOTIFY changed)
    Q_PROPERTY(QPointF shapeEnd READ shapeEnd NOTIFY changed)
    Q_PROPERTY(int shapeKind MEMBER shapeKind NOTIFY settingsChanged)
    Q_PROPERTY(bool shapeOutline MEMBER shapeOutline NOTIFY settingsChanged)
    Q_PROPERTY(int curveBend1 MEMBER curveBend1 NOTIFY settingsChanged)
    Q_PROPERTY(int curveBend2 MEMBER curveBend2 NOTIFY settingsChanged)
    Q_PROPERTY(bool shapeFill MEMBER shapeFill NOTIFY settingsChanged)
    Q_PROPERTY(int brushStyle MEMBER brushStyle NOTIFY settingsChanged)
    Q_PROPERTY(int opacity MEMBER opacity NOTIFY settingsChanged)
    Q_PROPERTY(int tolerance MEMBER tolerance NOTIFY settingsChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY changed)
    Q_PROPERTY(QString fileName READ fileName NOTIFY changed)
    Q_PROPERTY(QUrl fileUrl READ fileUrl NOTIFY changed)
    Q_PROPERTY(QString notice READ notice NOTIFY noticeChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
    Q_PROPERTY(bool dirty READ dirty NOTIFY changed)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY changed)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY changed)
    Q_PROPERTY(QSize imageSize READ imageSize NOTIFY changed)
    Q_PROPERTY(QColor primary MEMBER primary NOTIFY settingsChanged)
    Q_PROPERTY(QColor secondary MEMBER secondary NOTIFY settingsChanged)
    Q_PROPERTY(int brushSize MEMBER brushSize NOTIFY settingsChanged)
    Q_PROPERTY(Tool tool READ tool WRITE setTool NOTIFY settingsChanged)
    Q_PROPERTY(bool transparentEraser MEMBER transparentEraser NOTIFY settingsChanged)
  public:
    enum class Tool { Pencil, Eraser, ColorPicker, Fill, Brush, Shape };
    Q_ENUM(Tool)
    Tool tool() const {
        return tool_;
    }
    void setTool(Tool tool);
    explicit Editor(QObject* parent = nullptr);
    bool busy() const {
        return busy_;
    }
    bool filling() const {
        return filling_;
    }
    bool loading() const {
        return loading_ || filling_;
    }
    QString fileName() const;
    QUrl fileUrl() const {
        return fileUrl_;
    }
    QString notice() const {
        return notice_;
    }
    QString error() const {
        return error_;
    }
    Q_INVOKABLE void newDocument();
    Q_INVOKABLE void openFile(const QUrl& url);
    Q_INVOKABLE void saveFile(const QUrl& url);
    Q_INVOKABLE void cancelIo();
    bool dirty() const {
        return document.dirty() || document.active();
    }
    bool canUndo() const {
        return document.canUndo() || shapeActive_;
    }
    bool canRedo() const {
        return document.canRedo() && !shapeActive_;
    }
    QSize imageSize() const {
        return document.image().size();
    }
    const QImage& image() const {
        return document.image();
    }
    void begin(QPointF point, bool secondaryButton);
    void extend(QPointF point, bool constrain = false);
    void removeShapePoint();
    void endGesture();
    bool previewActive() const {
        return shapeActive_;
    }
    QPointF shapeStart() const {
        return shapeStart_;
    }
    QPointF shapeEnd() const {
        return shapeEnd_;
    }
    Q_INVOKABLE void setShapeGeometry(qreal x1, qreal y1, qreal x2, qreal y2);
    Q_INVOKABLE void finish();
    Q_INVOKABLE void cancel();
    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();
    QColor primary = Qt::black, secondary = Qt::white;
    int brushSize = 3, tolerance = 0, brushStyle = 0, opacity = 100;
    bool transparentEraser = false, shapeOutline = true, shapeFill = false;
    int shapeKind = 0, curveBend1 = 30, curveBend2 = -30;
  signals:
    void changed();
    void errorChanged();
    void noticeChanged();
    void documentReplaced();
    void saved(bool success);
    void settingsChanged();
    void pixelsChanged(QRect rect);

  private:
    void fail(const QString& message);
    void refreshShape();
    QPointF shapeStart_, shapeEnd_;
    QList<QPointF> shapePoints_;
    bool shapeActive_ = false, shapeSecondary_ = false, shapeInputBlocked_ = false;
    Tool tool_ = Tool::Pencil;
    bool busy_ = false, loading_ = false, filling_ = false;
    QFutureWatcher<openpaint::FillResult> fillWatcher_;
    quint64 fillRevision_ = 0;
    QUrl fileUrl_;
    QString error_, notice_;
    QFutureWatcher<openpaint::ImageResult> watcher_;
    openpaint::Cancellation cancellation_;
    QUrl operationUrl_;
    quint64 savingRevision_ = 0;

  protected:
    openpaint::Document document;
};
