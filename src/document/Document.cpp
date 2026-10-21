#include "Document.h"
#include <QColorSpace>
#include <QPainter>
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace openpaint {
bool Document::validSize(QSize size) {
    // Working copies, pending saves, and history need space too: M1 caps at 32 MP.
    return size.width() > 0 && size.height() > 0 && size.width() <= 16384 &&
           size.height() <= 16384 && qint64(size.width()) * size.height() <= 32000000;
}
Document::Document(QSize size, qsizetype budget) : budget_(std::max(qsizetype(0), budget)) {
    if (!validSize(size))
        throw std::invalid_argument("Invalid document dimensions");
    layers_.push_back({nextLayerId_++, QStringLiteral("Background"), {}});
    raster() = QImage(size, QImage::Format_ARGB32_Premultiplied);
    raster().fill(Qt::white);
    raster().setColorSpace(QColorSpace::SRgb);
}
bool Document::reset(const QImage& image) {
    if (image.isNull() || !validSize(image.size()))
        return false;
    layers_ = {{nextLayerId_++, QStringLiteral("Background"),
                image.convertToFormat(QImage::Format_ARGB32_Premultiplied)}};
    activeLayer_ = 0;
    raster().setColorSpace(QColorSpace::SRgb);
    original_ = {};
    selection_ = {};
    brush_.clear();
    active_ = false;
    previewMode_ = false;
    history_.clear();
    cursor_ = 0;
    historyBytes_ = 0;
    revision_ = nextRevision_++;
    savedRevision_ = revision_;
    return true;
}
bool Document::beginStroke(QPointF point, QColor color, qreal width, bool clearAlpha,
                           int brushStyle, int opacity) {
    if (!layers_[activeLayer_].visible || active_ || !std::isfinite(point.x()) ||
        !std::isfinite(point.y()) || std::abs(point.x()) > 1000000 ||
        std::abs(point.y()) > 1000000 || !std::isfinite(width) || width <= 0 || width > 256 ||
        !color.isValid())
        return false;
    if (brushStyle < -1 || brushStyle > 8 || opacity < 0 || opacity > 100)
        return false;
    previewMode_ = false;
    brushed_ = brushStyle >= 0;
    if (brushed_ &&
        !brush_.begin(raster().size(), point, color, width, brushStyle, opacity, selection_))
        return false;
    original_ = raster();
    active_ = true;
    color_ = color;
    width_ = width;
    clearAlpha_ = clearAlpha;
    previous_ = point;
    dirtyRect_ = {};
    extendStroke(point);
    return true;
}
QRect Document::extendStroke(QPointF point) {
    if (!active_ || !std::isfinite(point.x()) || !std::isfinite(point.y()) ||
        std::abs(point.x()) > 1000000 || std::abs(point.y()) > 1000000)
        return {};
    if (brushed_) {
        const auto rect = brush_.extend(raster(), original_, point);
        dirtyRect_ = dirtyRect_.united(rect);
        previous_ = point;
        return rect;
    }
    const qreal margin = width_ / 2 + 2;
    QRect rect = QRectF(previous_, point)
                     .normalized()
                     .adjusted(-margin, -margin, margin, margin)
                     .toAlignedRect()
                     .intersected(raster().rect());
    if (!rect.isEmpty()) {
        QPainter painter(&raster());
        painter.setRenderHint(QPainter::Antialiasing);
        if (!selection_.isEmpty())
            painter.setClipRegion(selection_);
        if (clearAlpha_)
            painter.setCompositionMode(QPainter::CompositionMode_Clear);
        painter.setPen(QPen(color_, width_, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        if (point == previous_)
            painter.drawPoint(point);
        else
            painter.drawLine(previous_, point);
        dirtyRect_ = dirtyRect_.united(rect);
    }
    previous_ = point;
    return rect;
}
bool Document::commitStroke() {
    if (!active_)
        return false;
    active_ = false;
    previewMode_ = false;
    brush_.clear();
    if (dirtyRect_.isEmpty() || original_.copy(dirtyRect_) == raster().copy(dirtyRect_)) {
        original_ = {};
        return false;
    }
    if (qint64(dirtyRect_.width()) * dirtyRect_.height() * 8 > budget_) {
        raster() = original_;
        original_ = {};
        return false;
    }
    Patch patch{dirtyRect_, original_.copy(dirtyRect_), raster().copy(dirtyRect_), revision_,
                nextRevision_++};
    original_ = {};
    append(std::move(patch));
    return true;
}
void Document::append(Patch patch) {
    if (patch.layersBefore.empty())
        patch.layerId = layers_[activeLayer_].id;
    while (history_.size() > cursor_) {
        historyBytes_ -= history_.back().bytes();
        history_.pop_back();
    }
    revision_ = patch.to;
    historyBytes_ += patch.bytes();
    history_.push_back(std::move(patch));
    cursor_ = history_.size();
    while (!history_.empty() && historyBytes_ > budget_) {
        historyBytes_ -= history_.front().bytes();
        history_.erase(history_.begin());
        --cursor_;
    }
}
bool Document::replaceImage(const QImage& candidate, quint64 sourceRevision) {
    if (layers_.size() != 1 || active_ || sourceRevision != revision_ || candidate.isNull() ||
        !validSize(candidate.size()) || candidate.format() != raster().format() ||
        candidate.colorSpace() != raster().colorSpace() ||
        candidate.sizeInBytes() + raster().sizeInBytes() > budget_ || candidate == raster())
        return false;
    Patch patch{{}, raster(), candidate, revision_, nextRevision_++, true};
    raster() = candidate;
    selection_ = {};
    append(std::move(patch));
    return true;
}
bool Document::beginPreview() {
    if (active_ || !layers_[activeLayer_].visible)
        return false;
    original_ = raster();
    active_ = true;
    brushed_ = false;
    previewMode_ = true;
    dirtyRect_ = {};
    return true;
}
bool Document::updatePreview(const QImage& patch, QRect rect) {
    if (!active_ || !previewMode_ ||
        (!rect.isEmpty() && (!raster().rect().contains(rect) || patch.size() != rect.size() ||
                             patch.format() != raster().format())))
        return false;
    QPainter painter(&raster());
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    if (!dirtyRect_.isEmpty())
        painter.drawImage(dirtyRect_, original_, dirtyRect_);
    if (!selection_.isEmpty())
        painter.setClipRegion(selection_);
    if (!rect.isEmpty())
        painter.drawImage(rect.topLeft(), patch);
    dirtyRect_ = dirtyRect_.united(rect);
    return true;
}
bool Document::applyPatch(const QImage& candidate, QRect rect, quint64 sourceRevision) {
    if (active_ || sourceRevision != revision_ || candidate.size() != raster().size() ||
        candidate.format() != raster().format() || rect.isEmpty() ||
        !raster().rect().contains(rect) || qint64(rect.width()) * rect.height() * 8 > budget_)
        return false;
    original_ = raster();
    active_ = true;
    dirtyRect_ = rect;
    {
        QPainter painter(&raster());
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.drawImage(rect, candidate, rect);
    }
    return commitStroke();
}
bool Document::setSelection(const QRegion& region) {
    if (active_ || (!region.isEmpty() && !raster().rect().contains(region.boundingRect())))
        return false;
    selection_ = region;
    return true;
}
bool Document::deleteSelection(QColor background) {
    if (selection_.isEmpty() || !beginPreview())
        return false;
    const auto rect = selection_.boundingRect();
    QImage patch(rect.size(), raster().format());
    patch.fill(background);
    updatePreview(patch, rect);
    return commitStroke();
}
void Document::cancelStroke() {
    brush_.clear();
    if (active_)
        raster() = original_;
    original_ = {};
    active_ = false;
    previewMode_ = false;
}
void Document::apply(const Patch& patch, bool forward) {
    if (!patch.layersBefore.empty()) {
        compositeValid_ = false;
        layers_ = forward ? patch.layersAfter : patch.layersBefore;
        activeLayer_ = forward ? patch.activeAfter : patch.activeBefore;
        selection_ = {};
        revision_ = forward ? patch.to : patch.from;
        return;
    }
    const auto target = std::find_if(layers_.begin(), layers_.end(),
                                     [&](const Layer& l) { return l.id == patch.layerId; });
    if (target == layers_.end())
        return;
    activeLayer_ = int(target - layers_.begin());
    if (patch.replacement) {
        raster() = forward ? patch.after : patch.before;
        selection_ = {};
    } else {
        QPainter painter(&raster());
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.drawImage(patch.rect.topLeft(), forward ? patch.after : patch.before);
    }
    revision_ = forward ? patch.to : patch.from;
}
bool Document::undo() {
    if (active_ || !canUndo())
        return false;
    apply(history_[--cursor_], false);
    return true;
}
bool Document::redo() {
    if (active_ || !canRedo())
        return false;
    apply(history_[cursor_++], true);
    return true;
}
bool Document::validLayers(const Layers& layers) {
    if (layers.empty() || layers.size() > 32 || !validSize(layers.front().pixels.size()))
        return false;
    const auto size = layers.front().pixels.size();
    // Bound resident rasters to 128 MiB, leaving room for workers/history/composite.
    if (qint64(size.width()) * size.height() * 4 * qint64(layers.size()) > 128 * 1024 * 1024)
        return false;
    for (const auto& l : layers) {
        if (l.pixels.isNull() || l.pixels.size() != size ||
            l.pixels.format() != QImage::Format_ARGB32_Premultiplied || !std::isfinite(l.opacity) ||
            l.opacity < 0 || l.opacity > 1 || l.name.size() > 128)
            return false;
    }
    return true;
}
bool Document::resetLayers(Layers layers) {
    if (!validLayers(layers))
        return false;
    if (!reset(layers.front().pixels))
        return false;
    for (auto& l : layers) {
        l.id = nextLayerId_++;
        l.pixels.setColorSpace(QColorSpace::SRgb);
    }
    compositeValid_ = false;
    layers_ = std::move(layers);
    activeLayer_ = int(layers_.size()) - 1;
    return true;
}
bool Document::selectLayer(int index) {
    if (active_ || index < 0 || index >= int(layers_.size()) || index == activeLayer_)
        return false;
    compositeValid_ = false;
    activeLayer_ = index;
    selection_ = {};
    return true;
}
bool Document::changeLayers(Layers layers, int active) {
    if (active_ || !validLayers(layers) || active < 0 || active >= int(layers.size()))
        return false;
    Patch patch;
    patch.from = revision_;
    patch.to = nextRevision_++;
    patch.layersBefore = layers_;
    patch.layersAfter = layers;
    patch.activeBefore = activeLayer_;
    patch.activeAfter = active;
    if (patch.bytes() > budget_)
        return false;
    compositeValid_ = false;
    layers_ = std::move(layers);
    activeLayer_ = active;
    selection_ = {};
    append(std::move(patch));
    return true;
}
bool Document::replaceLayers(Layers layers, quint64 sourceRevision) {
    return sourceRevision == revision_ && changeLayers(std::move(layers), activeLayer_);
}
bool Document::addLayer(bool duplicate) {
    if (active_ || layers_.size() >= 32 ||
        qint64(raster().sizeInBytes()) * qint64(layers_.size() * 2 + 1) > budget_)
        return false;
    auto result = layers_;
    Layer layer{nextLayerId_++, QStringLiteral("Layer %1").arg(nextLayerId_ - 1), raster()};
    if (duplicate) {
        layer = layers_[activeLayer_];
        layer.id = nextLayerId_ - 1;
        layer.name = (layer.name + QStringLiteral(" copy")).left(128);
    } else
        layer.pixels.fill(Qt::transparent);
    result.insert(result.begin() + activeLayer_ + 1, layer);
    return changeLayers(std::move(result), activeLayer_ + 1);
}
bool Document::deleteLayer() {
    if (layers_.size() <= 1 || activeLayer_ == 0)
        return false;
    auto result = layers_;
    result.erase(result.begin() + activeLayer_);
    return changeLayers(std::move(result), std::min(activeLayer_, int(layers_.size()) - 2));
}
bool Document::moveLayer(int index) {
    if (activeLayer_ == 0 || index < 1 || index >= int(layers_.size()) || index == activeLayer_)
        return false;
    auto result = layers_;
    auto layer = result[activeLayer_];
    result.erase(result.begin() + activeLayer_);
    result.insert(result.begin() + index, layer);
    return changeLayers(std::move(result), index);
}
bool Document::renameLayer(const QString& name) {
    const auto clean = name.trimmed();
    if (clean.isEmpty() || clean.size() > 128 || clean == layers_[activeLayer_].name)
        return false;
    auto result = layers_;
    result[activeLayer_].name = clean;
    return changeLayers(std::move(result), activeLayer_);
}
bool Document::setLayerVisible(bool visible) {
    return setLayerVisibility(activeLayer_, visible);
}
bool Document::setLayerVisibility(int index, bool visible) {
    if (index < 0 || index >= int(layers_.size()) || layers_[index].visible == visible)
        return false;
    auto result = layers_;
    result[index].visible = visible;
    return changeLayers(std::move(result), activeLayer_);
}
bool Document::setLayerOpacity(qreal opacity) {
    if (!std::isfinite(opacity) || opacity < 0 || opacity > 1 ||
        layers_[activeLayer_].opacity == opacity)
        return false;
    auto result = layers_;
    result[activeLayer_].opacity = opacity;
    return changeLayers(std::move(result), activeLayer_);
}
QImage Document::composite(const Layers& layers) {
    if (layers.empty())
        return {};
    QImage result(layers.front().pixels.size(), QImage::Format_ARGB32_Premultiplied);
    result.setColorSpace(QColorSpace::SRgb);
    result.fill(Qt::transparent);
    QPainter painter(&result);
    for (const auto& l : layers)
        if (l.visible) {
            painter.setOpacity(l.opacity);
            painter.drawImage(0, 0, l.pixels);
        }
    return result;
}
QImage Document::composite(bool previewBase) const {
    if (layers_.size() == 1 && layers_[0].visible && layers_[0].opacity == 1.0)
        return previewBase && active_ ? original_ : raster();
    if (!compositeValid_ || compositePreview_ != previewBase) {
        auto layers = layers_;
        if (previewBase && active_)
            layers[activeLayer_].pixels = original_;
        compositeCache_ = composite(layers);
        compositeValid_ = true;
        compositePreview_ = previewBase;
    }
    return compositeCache_;
}
bool Document::needsProject() const {
    return layers_.size() > 1 || !layers_[0].visible || layers_[0].opacity != 1.0 ||
           layers_[0].name != QStringLiteral("Background");
}
bool Document::mergeDown() {
    if (activeLayer_ == 0 || !layers_[activeLayer_].visible || !layers_[activeLayer_ - 1].visible)
        return false;
    auto result = layers_;
    result[activeLayer_ - 1].pixels =
        composite(Layers{result[activeLayer_ - 1], result[activeLayer_]});
    result[activeLayer_ - 1].opacity = 1;
    result.erase(result.begin() + activeLayer_);
    return changeLayers(std::move(result), activeLayer_ - 1);
}
} // namespace openpaint
