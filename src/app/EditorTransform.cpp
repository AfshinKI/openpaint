// SPDX-License-Identifier: GPL-3.0-or-later
#include "Editor.h"
#include <QtConcurrent>
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
    if (busy_ || !canSelect())
        return;
    if (operation == openpaint::TransformKind::Crop && !hasSelection())
        return;
    finish();
    const auto source = hasSelection() ? selectedImage() : image();
    const auto mask =
        hasSelection() ? selection().translated(-selectionRect().topLeft()) : QRegion();
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
    } else if (transformSelection_) {
        floatingOriginalSelection_ = document.selection();
        floatingPosition_ = floatingOriginalSelection_.boundingRect().topLeft();
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
