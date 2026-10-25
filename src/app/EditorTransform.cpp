// SPDX-License-Identifier: GPL-3.0-or-later
#include "Editor.h"
#include <QtConcurrent>
#include <algorithm>
#include <cmath>
QRectF Editor::resizeBounds() const {
    if (busy() || selecting_ || textDrawing_)
        return {};
    if (textActive_)
        return textRect_;
    if (shapeActive_) {
        const auto box = QPolygonF(shapePoints_).boundingRect();
        return {box.topLeft(), QSizeF(std::max(1.0, box.width()), std::max(1.0, box.height()))};
    }
    if (floatingActive_)
        return {floatingPosition_, floatingImage_.size()};
    return hasSelection() ? QRectF(selectionRect()) : QRectF();
}
void Editor::setSmartGuides(bool enabled) {
    if (smartGuides_ == enabled)
        return;
    smartGuides_ = enabled;
    guidePosition_ = {-1, -1};
    emit guidesChanged();
}
void Editor::clearGuides() {
    if (guidePosition_ == QPointF(-1, -1))
        return;
    guidePosition_ = {-1, -1};
    emit guidesChanged();
}
void Editor::moveActiveBounds(int x, int y, qreal tolerance) {
    clearGuides();
    const auto before = resizeBounds();
    if (before.isEmpty() || std::abs(qint64(x)) > 1000000 || std::abs(qint64(y)) > 1000000)
        return;
    // Geometry and preview mutation stay on the editor's GUI thread. Tolerance is
    // supplied in image pixels by the view, so the capture distance stays constant on screen.
    QPointF guides(-1, -1);
    auto snap = [&](int position, qreal extent, int canvasExtent, qreal& guide) {
        qreal best = tolerance;
        int result = position;
        for (qreal target : {0.0, canvasExtent / 2.0, double(canvasExtent)}) {
            for (qreal edge : {0.0, extent / 2.0, extent}) {
                const qreal candidate = target - edge;
                const int pixel = qRound(candidate);
                const qreal distance = std::abs(candidate - position);
                if (std::abs(candidate - pixel) < 0.001 && distance <= best &&
                    (guide < 0 || distance < best)) {
                    best = distance;
                    result = pixel;
                    guide = target;
                }
            }
        }
        return result;
    };
    if (smartGuides_ && std::isfinite(tolerance) && tolerance > 0 && tolerance <= 600) {
        qreal gx = -1, gy = -1;
        x = snap(x, before.width(), imageSize().width(), gx);
        y = snap(y, before.height(), imageSize().height(), gy);
        guides = {gx, gy};
    }
    if (textActive_ || shapeActive_) {
        const int boundedX =
            std::clamp(x, 0, std::max(0, int(imageSize().width() - before.width())));
        const int boundedY =
            std::clamp(y, 0, std::max(0, int(imageSize().height() - before.height())));
        if (boundedX != x)
            guides.setX(-1);
        if (boundedY != y)
            guides.setY(-1);
        x = boundedX;
        y = boundedY;
        if (textActive_) {
            setTextBounds(x, y, textRect_.width(), textRect_.height());
        } else {
            const QPointF delta = QPointF(x, y) - before.topLeft();
            shapeStart_ += delta;
            shapeEnd_ += delta;
            for (auto& point : shapePoints_)
                point += delta;
            refreshShape();
        }
    } else {
        if (!floatingActive_) {
            beginMove(selectionRect().topLeft());
            floatingDragging_ = false;
        }
        if (!floatingActive_ || x <= -floatingImage_.width() || y <= -floatingImage_.height() ||
            x >= imageSize().width() || y >= imageSize().height())
            return;
        moveFloating(x, y);
    }
    guidePosition_ = guides;
    emit guidesChanged();
}
void Editor::resizeCanvas(int x, int y, int width, int height) {
    if (busy() || previewActive() || hasSelection() ||
        !openpaint::Document::validSize({width, height}) || std::abs(qint64(x)) > 16384 ||
        std::abs(qint64(y)) > 16384 || (x == 0 && y == 0 && imageSize() == QSize(width, height)))
        return;
    openpaint::TransformOptions options;
    options.size = {width, height};
    options.canvasOrigin = {x, y};
    options.background = secondary;
    startTransform(openpaint::TransformKind::CanvasResize, options);
}
void Editor::resizeActiveBounds(int x, int y, int width, int height) {
    const auto before = resizeBounds();
    if (before.isEmpty() || !openpaint::Document::validSize({width, height}) || x < 0 || y < 0 ||
        width > image().width() || height > image().height() || x > image().width() - width ||
        y > image().height() - height)
        return;
    const QRect target(x, y, width, height);
    if (before == QRectF(target))
        return;
    if (textActive_) {
        setTextBounds(x, y, width, height);
    } else if (shapeActive_) {
        const auto map = [&](QPointF p) {
            return QPointF(x + (p.x() - before.x()) * width / before.width(),
                           y + (p.y() - before.y()) * height / before.height());
        };
        shapeStart_ = map(shapeStart_);
        shapeEnd_ = map(shapeEnd_);
        for (auto& point : shapePoints_)
            point = map(point);
        refreshShape();
    } else {
        startTransform(openpaint::TransformKind::ResizeSkew, {{width, height}, 0, 0, true});
        if (transforming_)
            transformPosition_ = target.topLeft();
    }
}
void Editor::invertColors() {
    if (!canInvertColors())
        return;
    startTransform(openpaint::TransformKind::InvertColors);
}
void Editor::transformImage(int kind) {
    if (kind < 0 || kind > int(openpaint::TransformKind::Crop))
        return;
    startTransform(openpaint::TransformKind(kind));
}
void Editor::resizeImage(int width, int height, qreal skewX, qreal skewY, bool smooth) {
    startTransform(openpaint::TransformKind::ResizeSkew, {{width, height}, skewX, skewY, smooth});
}
void Editor::startTransform(openpaint::TransformKind operation,
                            const openpaint::TransformOptions& options) {
    const bool floatingResize =
        floatingActive_ && operation == openpaint::TransformKind::ResizeSkew;
    if (busy_ || (!canSelect() && !floatingResize))
        return;
    if (operation == openpaint::TransformKind::Crop && !hasSelection())
        return;
    transformColors_ = operation == openpaint::TransformKind::InvertColors;
    transformFromFloating_ = floatingResize;
    transformPosition_ = floatingResize ? floatingPosition_ : selectionRect().topLeft();
    if (!floatingResize)
        finish();
    const auto source = transformColors_ ? image()
                        : floatingResize ? floatingImage_
                        : hasSelection() ? selectedImage()
                                         : image();
    const auto mask = transformColors_ ? selection()
                      : floatingResize ? floatingRegion_
                      : hasSelection() ? selection().translated(-selectionRect().topLeft())
                                       : QRegion();
    transformSelection_ =
        !transformColors_ && hasSelection() && operation != openpaint::TransformKind::Crop;
    transformRevision_ = document.revision();
    cancellation_ = std::make_shared<std::atomic_bool>(false);
    busy_ = loading_ = transforming_ = true;
    emit changed();
    const auto layers = document.layers();
    const auto cropBounds = selectionRect();
    const bool wholeStack = !transformColors_ && !transformSelection_ && layers.size() > 1;
    transformWatcher_.setFuture(QtConcurrent::run(
        [source, mask, operation, options, layers, wholeStack, cropBounds, token = cancellation_] {
            if (!wholeStack)
                return openpaint::transformImage(source, mask, operation, token, options);
            openpaint::TransformResult result;
            auto settings = options;
            if (operation == openpaint::TransformKind::CanvasResize)
                settings.background = Qt::transparent;
            // Crop uses the same selection bounds on every layer, preserving the stack.
            for (auto layer : layers) {
                auto input = layer.pixels;
                if (operation == openpaint::TransformKind::Crop)
                    input = input.copy(cropBounds);
                auto transformed = openpaint::transformImage(input, {}, operation, token, settings);
                if (transformed.cancelled || !transformed.error.isEmpty())
                    return transformed;
                layer.pixels = transformed.image;
                result.layers.push_back(std::move(layer));
                if (qint64(transformed.image.sizeInBytes()) * qint64(layers.size()) >
                    128 * 1024 * 1024) {
                    result.error = tr("This transform exceeds the layer undo budget.");
                    return result;
                }
            }
            result.image = result.layers.front().pixels;
            return result;
        }));
}
void Editor::finishTransform() {
    const auto result = transformWatcher_.result();
    busy_ = loading_ = transforming_ = false;
    if (result.cancelled || cancellation_->load()) {
        emit changed();
        return;
    }
    if (!result.error.isEmpty() || document.revision() != transformRevision_) {
        fail(result.error.isEmpty() ? tr("The drawing changed before the transform completed.")
                                    : result.error);
    } else if (transformColors_) {
        const auto rect = result.mask.isEmpty() ? result.image.rect() : result.mask.boundingRect();
        if (result.image != image() && !document.applyPatch(result.image, rect, transformRevision_))
            fail(tr("This edit exceeds the undo budget. The drawing is unchanged."));
        emit pixelsChanged(rect);
    } else if (transformFromFloating_) {
        floatingImage_ = result.image;
        floatingRegion_ = result.mask;
        floatingPosition_ = transformPosition_;
        floatingComposite_ = {};
        refreshFloating();
    } else if (transformSelection_) {
        floatingOriginalSelection_ = document.selection();
        floatingPosition_ = transformPosition_;
        document.setSelection({});
        if (document.beginPreview()) {
            floatingImage_ = result.image;
            floatingComposite_ = {};
            floatingRegion_ = result.mask;
            floatingActive_ = floatingMove_ = true;
            floatingDragging_ = false;
            tool_ = Tool::Select;
            refreshFloating();
            emit settingsChanged();
        } else {
            document.setSelection(floatingOriginalSelection_);
            fail(tr("Finish the current edit before transforming a selection."));
        }
    } else {
        const auto size = imageSize();
        const bool stackChanged =
            !result.layers.empty() &&
            !std::equal(result.layers.begin(), result.layers.end(), document.layers().begin(),
                        document.layers().end(),
                        [](const openpaint::Layer& a, const openpaint::Layer& b) {
                            return a.pixels == b.pixels;
                        });
        if (!result.layers.empty()
                ? (stackChanged && !document.replaceLayers(result.layers, transformRevision_))
                : (result.image != image() &&
                   !document.replaceImage(result.image, transformRevision_)))
            fail(tr("This transform exceeds the undo budget. The drawing is unchanged."));
        else
            document.setSelection({});
        if (imageSize() != size)
            emit documentReplaced();
        emit pixelsChanged({});
    }
    emit changed();
}
