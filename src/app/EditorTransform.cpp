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
    transformFromFloating_ = floatingResize;
    transformPosition_ = floatingResize ? floatingPosition_ : selectionRect().topLeft();
    if (!floatingResize)
        finish();
    const auto source = floatingResize   ? floatingImage_
                        : hasSelection() ? selectedImage()
                                         : image();
    const auto mask = floatingResize   ? floatingRegion_
                      : hasSelection() ? selection().translated(-selectionRect().topLeft())
                                       : QRegion();
    transformSelection_ = hasSelection() && operation != openpaint::TransformKind::Crop;
    transformRevision_ = document.revision();
    cancellation_ = std::make_shared<std::atomic_bool>(false);
    busy_ = loading_ = transforming_ = true;
    emit changed();
    transformWatcher_.setFuture(
        QtConcurrent::run([source, mask, operation, options, token = cancellation_] {
            return openpaint::transformImage(source, mask, operation, token, options);
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
        if (result.image != image() && !document.replaceImage(result.image, transformRevision_))
            fail(tr("This transform exceeds the undo budget. The drawing is unchanged."));
        else
            document.setSelection({});
        if (imageSize() != size)
            emit documentReplaced();
        emit pixelsChanged({});
    }
    emit changed();
}
