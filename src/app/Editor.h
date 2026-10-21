// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "document/Document.h"
#include "io/PngIo.h"
#include "tools/ColorSelect.h"
#include "tools/Fill.h"
#include "tools/Transform.h"
#include <QFutureWatcher>
#include <QObject>
#include <QPointer>
#include <QPolygon>
#include <QQuickTextDocument>
#include <QUrl>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

class Editor : public QObject {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QVariantList layers READ layers NOTIFY changed)
    Q_PROPERTY(int activeLayer READ activeLayer NOTIFY changed)
    Q_PROPERTY(bool canEditLayers READ canEditLayers NOTIFY changed)
    Q_PROPERTY(bool needsProject READ needsProject NOTIFY changed)
    Q_PROPERTY(bool sampleMerged MEMBER sampleMerged NOTIFY settingsChanged)
    Q_PROPERTY(bool exporting READ exporting NOTIFY changed)
    Q_PROPERTY(bool transforming READ transforming NOTIFY changed)
    Q_PROPERTY(bool colorSelection MEMBER colorSelection NOTIFY settingsChanged)
    Q_PROPERTY(bool freeFormSelection MEMBER freeFormSelection NOTIFY settingsChanged)
    Q_PROPERTY(bool transparentSelection MEMBER transparentSelection NOTIFY settingsChanged)
    Q_PROPERTY(bool clipboardBusy READ clipboardBusy NOTIFY changed)
    Q_PROPERTY(bool floatingActive READ floatingActive NOTIFY changed)
    Q_PROPERTY(QPoint floatingPosition READ floatingPosition NOTIFY changed)
    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY changed)
    Q_PROPERTY(QRect selectionRect READ selectionRect NOTIFY changed)
    Q_PROPERTY(bool canSelect READ canSelect NOTIFY changed)
    Q_PROPERTY(bool textActive READ textActive NOTIFY changed)
    Q_PROPERTY(bool textReady READ textReady NOTIFY changed)
    Q_PROPERTY(QRect textRect READ textRect NOTIFY changed)
    Q_PROPERTY(QString textDraft READ textDraft NOTIFY textDraftChanged)
    Q_PROPERTY(QColor textColor READ textColor NOTIFY changed)
    Q_PROPERTY(QColor textBackgroundColor READ textBackgroundColor NOTIFY changed)
    Q_PROPERTY(QString textFamily MEMBER textFamily WRITE setTextFamily NOTIFY settingsChanged)
    Q_PROPERTY(int textSize MEMBER textSize WRITE setTextSize NOTIFY settingsChanged)
    Q_PROPERTY(int textAlignment MEMBER textAlignment NOTIFY settingsChanged)
    Q_PROPERTY(bool textBold MEMBER textBold WRITE setTextBold NOTIFY settingsChanged)
    Q_PROPERTY(bool textItalic MEMBER textItalic WRITE setTextItalic NOTIFY settingsChanged)
    Q_PROPERTY(
        bool textUnderline MEMBER textUnderline WRITE setTextUnderline NOTIFY settingsChanged)
    Q_PROPERTY(
        bool textStrikeout MEMBER textStrikeout WRITE setTextStrikeout NOTIFY settingsChanged)
    Q_PROPERTY(bool textBackground MEMBER textBackground NOTIFY settingsChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY changed)
    Q_PROPERTY(bool interactionActive READ interactionActive NOTIFY changed)
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
    Q_PROPERTY(bool loading READ loading NOTIFY changed)
    Q_PROPERTY(QString fileName READ fileName NOTIFY changed)
    Q_PROPERTY(QUrl fileUrl READ fileUrl NOTIFY changed)
    Q_PROPERTY(QString notice READ notice NOTIFY noticeChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
    Q_PROPERTY(bool dirty READ dirty NOTIFY changed)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY changed)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY changed)
    Q_PROPERTY(QSize imageSize READ imageSize NOTIFY changed)
    Q_PROPERTY(bool editingSecondary READ editingSecondary WRITE setEditingSecondary NOTIFY
                   settingsChanged)
    Q_PROPERTY(
        QColor selectedColor READ selectedColor WRITE setSelectedColor NOTIFY settingsChanged)
    Q_PROPERTY(QRectF resizeBounds READ resizeBounds NOTIFY changed)
    Q_PROPERTY(QColor primary MEMBER primary NOTIFY settingsChanged)
    Q_PROPERTY(QColor secondary MEMBER secondary NOTIFY settingsChanged)
    Q_PROPERTY(int brushSize MEMBER brushSize NOTIFY settingsChanged)
    Q_PROPERTY(Tool tool READ tool WRITE setTool NOTIFY settingsChanged)
    Q_PROPERTY(bool transparentEraser MEMBER transparentEraser NOTIFY settingsChanged)
  public:
    QVariantList layers() const;
    QImage layerImage(int index) const {
        return index >= 0 && index < int(document.layers().size())
                   ? document.layers()[size_t(index)].pixels
                   : QImage();
    }
    int activeLayer() const {
        return document.activeLayer();
    }
    bool canEditLayers() const {
        return !busy() && !interactionActive() && !selecting_;
    }
    bool needsProject() const {
        return document.needsProject() ||
               fileUrl_.toLocalFile().endsWith(".ora", Qt::CaseInsensitive);
    }
    bool sampleMerged = false;
    Q_INVOKABLE bool enableLayers();
    Q_INVOKABLE void reorderLayer(const QString& id, int target);
    Q_INVOKABLE void toggleLayerVisibility(int index);
    Q_INVOKABLE void selectLayer(int index);
    Q_INVOKABLE void addLayer(bool duplicate = false);
    Q_INVOKABLE void deleteLayer();
    Q_INVOKABLE void moveLayer(int index);
    Q_INVOKABLE void renameLayer(const QString& name);
    Q_INVOKABLE void setLayerVisible(bool visible);
    Q_INVOKABLE void setLayerOpacity(int opacity);
    Q_INVOKABLE void mergeLayerDown();
    enum class Tool { Pencil, Eraser, ColorPicker, Fill, Brush, Shape, Text, Select };
    Q_ENUM(Tool)
    Tool tool() const {
        return tool_;
    }
    void setTool(Tool tool);
    bool editingSecondary() const {
        return editingSecondary_;
    }
    void setEditingSecondary(bool active);
    QColor selectedColor() const {
        return editingSecondary_ ? secondary : primary;
    }
    void setSelectedColor(QColor color);
    explicit Editor(QObject* parent = nullptr);
    bool exporting() const {
        return exporting_;
    }
    Q_INVOKABLE void exportFile(const QUrl& url, int format, int quality, bool lossless,
                                bool flatten, QColor background);
    bool transforming() const {
        return transforming_;
    }
    Q_INVOKABLE void transformImage(int kind);
    QRectF resizeBounds() const;
    Q_INVOKABLE void resizeCanvas(int x, int y, int width, int height);
    Q_INVOKABLE void resizeActiveBounds(int x, int y, int width, int height);
    Q_INVOKABLE void resizeImage(int width, int height, qreal skewX, qreal skewY, bool smooth);
    bool clipboardBusy() const {
        return clipboardBusy_;
    }
    bool interactionActive() const {
        return document.active() || previewActive();
    }
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
        return document.canUndo() || previewActive();
    }
    bool canRedo() const {
        return document.canRedo() && !previewActive();
    }
    QSize imageSize() const {
        return document.image().size();
    }
    const QImage& image() const {
        return document.image();
    }
    QImage displayImage() const {
        return document.composite(textActive_ && !hasSelection() && !needsProject());
    }
    bool hasSelection() const {
        return !selection().isEmpty();
    }
    QRect selectionRect() const {
        return selection().boundingRect();
    }
    QRegion selection() const {
        return floatingActive_ ? floatingRegion_.translated(floatingPosition_)
                               : document.selection();
    }
    bool floatingActive() const {
        return floatingActive_;
    }
    QPoint floatingPosition() const {
        return floatingPosition_;
    }
    QImage selectedImage(bool merged = false) const;
    Q_INVOKABLE bool copySelection();
    Q_INVOKABLE bool copyVisibleLayers();
    Q_INVOKABLE void cutSelection();
    Q_INVOKABLE void pasteClipboard();
    bool pasteImage(const QImage& image);
    Q_INVOKABLE void moveFloating(int x, int y);
    void nudgeSelection(int dx, int dy);
    bool canSelect() const {
        return !loading() && !previewActive();
    }
    Q_INVOKABLE void selectRect(int x, int y, int width, int height);
    Q_INVOKABLE void selectAll();
    Q_INVOKABLE void invertSelection();
    const QPolygon& selectionDraft() const {
        return selectionPoints_;
    }
    Q_INVOKABLE void deselect();
    Q_INVOKABLE void deleteSelection();
    bool textActive() const {
        return textActive_;
    }
    bool textReady() const {
        return textActive_ && !textDrawing_;
    }
    QRect textRect() const {
        return textRect_;
    }
    QString textDraft() const {
        return textDraft_;
    }
    QColor textColor() const {
        return textSecondary_ ? secondary : primary;
    }
    QColor textBackgroundColor() const {
        return textSecondary_ ? primary : secondary;
    }
    Q_INVOKABLE void attachTextDocument(QQuickTextDocument* document);
    Q_INVOKABLE void formatTextRange(const QFont& font, int start, int end);
    Q_INVOKABLE void setInsertionFont(const QFont& change, const QFont& current, int position);
    Q_INVOKABLE void clearInsertionFont();
    Q_INVOKABLE int undoText(bool redo);
    Q_INVOKABLE void syncTextCursor(int position, int start, int end);
    Q_INVOKABLE void syncTextFont(const QFont& font);
    void setTextFamily(const QString& family);
    void setTextSize(int size);
    void setTextBold(bool enabled);
    void setTextItalic(bool enabled);
    void setTextUnderline(bool enabled);
    void setTextStrikeout(bool enabled);
    Q_INVOKABLE bool setTextDraft(const QString& text);
    Q_INVOKABLE void setTextBounds(int x, int y, int width, int height);
    void begin(QPointF point, bool secondaryButton);
    void extend(QPointF point, bool constrain = false);
    void removeShapePoint();
    void endGesture();
    bool previewActive() const {
        return shapeActive_ || textActive_ || floatingActive_;
    }
    QPointF shapeStart() const {
        return shapeStart_;
    }
    QPointF shapeEnd() const {
        return shapeEnd_;
    }
    Q_INVOKABLE void setShapeGeometry(qreal x1, qreal y1, qreal x2, qreal y2);
    Q_INVOKABLE void commitPreview();
    Q_INVOKABLE void finish();
    Q_INVOKABLE void cancel();
    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();
    QColor primary = Qt::black, secondary = Qt::white;
    int brushSize = 3, brushStyle = 0, opacity = 100;
    bool colorSelection = false;
    bool freeFormSelection = false, transparentSelection = false;
    bool transparentEraser = false, shapeOutline = true, shapeFill = false;
    int shapeKind = 0, curveBend1 = 30, curveBend2 = -30;
    QString textFamily = QStringLiteral("Sans Serif");
    int textSize = 20, textAlignment = 0;
    bool textBold = false, textItalic = false, textUnderline = false, textStrikeout = false,
         textBackground = false;
  signals:
    void changed();
    void textDraftChanged();
    void textFontRequested(const QFont& font);
    void textEditingReady();
    void errorChanged();
    void noticeChanged();
    void documentReplaced();
    void documentSessionChanged();
    void saved(bool success);
    void exported(bool success);
    void settingsChanged();
    void pixelsChanged(QRect rect);

  private:
    void layerChanged(bool success);
    bool layersInitialized_ = false;
    bool editingSecondary_ = false;
    void fail(const QString& message);
    void finishTransform();
    void startTransform(openpaint::TransformKind operation,
                        const openpaint::TransformOptions& options = {});
    QFutureWatcher<openpaint::TransformResult> transformWatcher_;
    quint64 transformRevision_ = 0;
    bool transforming_ = false, transformSelection_ = false, transformFromFloating_ = false;
    QPoint transformPosition_;
    void refreshShape();
    void refreshText();
    bool startCopy(bool cut, bool merged = false);
    void finishCopy();
    QFutureWatcher<QByteArray> copyWatcher_;
    bool cutAfterCopy_ = false, clipboardBusy_ = false;
    void startColorSelection(QPoint seed);
    void finishColorSelection();
    QFutureWatcher<openpaint::ColorSelectionResult> colorSelectionWatcher_;
    quint64 colorSelectionRevision_ = 0;
    void finishSelection();
    QPolygon selectionPoints_;
    void beginMove(QPoint point);
    void refreshFloating();
    bool floatingActive_ = false, floatingMove_ = false, floatingDragging_ = false;
    QImage floatingImage_, floatingComposite_;
    QRgb floatingColorKey_ = 0;
    bool floatingTransparent_ = false;
    QRegion floatingRegion_, floatingOriginalSelection_;
    QPoint floatingPosition_, floatingDragOffset_;
    bool selecting_ = false;
    QPoint selectionAnchor_;
    QRegion previousSelection_;
    bool textActive_ = false, textDrawing_ = false, textSecondary_ = false;
    QRect textRect_;
    QPointF textAnchor_;
    QString textDraft_;
    QPointer<QTextDocument> inlineDocument_;
    QMetaObject::Connection inlineDocumentConnection_;
    QPointer<QObject> inlineInput_;
    QFont pendingTextFont_;
    bool pendingTextFormat_ = false, applyingTextFormat_ = false;
    int pendingTextPosition_ = 0;
    QPointF shapeStart_, shapeEnd_;
    QList<QPointF> shapePoints_;
    bool shapeActive_ = false, shapeSecondary_ = false, shapeInputBlocked_ = false;
    Tool tool_ = Tool::Pencil;
    bool exporting_ = false;
    bool busy_ = false, loading_ = false, filling_ = false, pasting_ = false;
    QFutureWatcher<openpaint::FillResult> fillWatcher_;
    quint64 fillRevision_ = 0;
    QUrl fileUrl_;
    QString error_, notice_, sourceName_;
    QFutureWatcher<openpaint::ImageResult> watcher_;
    openpaint::Cancellation cancellation_;
    QUrl operationUrl_;
    quint64 savingRevision_ = 0;

  protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    openpaint::Document document;
};
