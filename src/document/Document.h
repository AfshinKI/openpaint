#pragma once

#include "tools/Brush.h"
#include <QColor>
#include <QImage>
#include <QPointF>
#include <QRect>
#include <QRegion>
#include <vector>

namespace openpaint {
// GUI-thread owned. Consumers receive implicitly shared, immutable snapshots.
struct Layer {
    quint64 id = 0;
    QString name;
    QImage pixels;
    bool visible = true;
    qreal opacity = 1.0;
};
using Layers = std::vector<Layer>;

class Document {
  public:
    explicit Document(QSize size = {1152, 648}, qsizetype budget = 256 * 1024 * 1024);
    static bool validSize(QSize size);
    const QImage& image() const {
        return raster();
    }
    const Layers& layers() const {
        return layers_;
    }
    int activeLayer() const {
        return activeLayer_;
    }
    bool selectLayer(int index);
    bool addLayer(bool duplicate = false);
    bool deleteLayer();
    bool moveLayer(int index);
    bool renameLayer(const QString& name);
    bool setLayerVisible(bool visible);
    bool setLayerOpacity(qreal opacity);
    bool mergeDown();
    bool replaceLayers(Layers layers, quint64 sourceRevision);
    bool resetLayers(Layers layers);
    static bool validLayers(const Layers& layers);
    bool needsProject() const;
    QImage composite(bool previewBase = false) const;
    static QImage composite(const Layers& layers);
    quint64 revision() const {
        return revision_;
    }
    bool dirty() const {
        return revision_ != savedRevision_;
    }
    bool canUndo() const {
        return cursor_ > 0;
    }
    bool canRedo() const {
        return cursor_ < history_.size();
    }
    bool active() const {
        return active_;
    }
    qsizetype historyBytes() const {
        return historyBytes_;
    }
    void markSaved(quint64 revision) {
        savedRevision_ = revision;
    }
    bool reset(const QImage& image);
    const QRegion& selection() const {
        return selection_;
    }
    bool setSelection(const QRegion& region);
    bool deleteSelection(QColor background = Qt::transparent);
    bool beginStroke(QPointF point, QColor color, qreal width, bool clearAlpha = false,
                     int brushStyle = -1, int opacity = 100);
    QRect extendStroke(QPointF point);
    bool commitStroke();
    void cancelStroke();
    bool beginPreview();
    bool updatePreview(const QImage& patch, QRect rect);
    const QImage& previewBase() const {
        return active_ ? original_ : raster();
    }
    bool applyPatch(const QImage& candidate, QRect rect, quint64 sourceRevision);
    bool replaceImage(const QImage& candidate, quint64 sourceRevision);
    bool undo();
    bool redo();

  private:
    struct Patch {
        QRect rect;
        QImage before, after;
        quint64 from, to;
        bool replacement = false;
        quint64 layerId = 0;
        Layers layersBefore = {}, layersAfter = {};
        int activeBefore = 0, activeAfter = 0;
        qsizetype bytes() const {
            qsizetype result = before.sizeInBytes() + after.sizeInBytes();
            for (const auto& layer : layersBefore)
                result += layer.pixels.sizeInBytes();
            for (const auto& layer : layersAfter)
                result += layer.pixels.sizeInBytes();
            return result;
        }
    };
    void append(Patch patch);
    void apply(const Patch& patch, bool forward);
    QImage& raster() {
        compositeValid_ = false;
        return layers_[activeLayer_].pixels;
    }
    const QImage& raster() const {
        return layers_[activeLayer_].pixels;
    }
    bool changeLayers(Layers layers, int active);
    Layers layers_;
    int activeLayer_ = 0;
    quint64 nextLayerId_ = 1;
    QImage original_;
    mutable QImage compositeCache_;
    mutable bool compositeValid_ = false, compositePreview_ = false;
    QRegion selection_;
    Brush brush_;
    bool brushed_ = false, previewMode_ = false;
    std::vector<Patch> history_;
    size_t cursor_ = 0;
    qsizetype budget_, historyBytes_ = 0;
    quint64 revision_ = 0, savedRevision_ = 0, nextRevision_ = 1;
    bool active_ = false, clearAlpha_ = false;
    QColor color_;
    qreal width_ = 1;
    QPointF previous_;
    QRect dirtyRect_;
};
} // namespace openpaint
