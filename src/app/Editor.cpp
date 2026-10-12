// SPDX-License-Identifier: GPL-3.0-or-later
#include "Editor.h"
#include "tools/Shape.h"
#include <QFileInfo>
#include <QtConcurrent>
#include <cmath>
void Editor::setTool(Tool tool) {
    if (tool == tool_ ||
        (tool != Tool::Pencil && tool != Tool::Eraser && tool != Tool::ColorPicker &&
         tool != Tool::Fill && tool != Tool::Brush && tool != Tool::Shape))
        return;
    if (filling_)
        cancelIo();
    finish();
    tool_ = tool;
    emit settingsChanged();
}
void Editor::begin(QPointF point, bool secondaryButton) {
    if (loading())
        return;
    if (!std::isfinite(point.x()) || !std::isfinite(point.y()) || point.x() < 0 || point.y() < 0 ||
        point.x() >= image().width() || point.y() >= image().height())
        return;
    if (tool_ == Tool::ColorPicker) {
        const auto color =
            image().pixelColor(int(std::floor(point.x())), int(std::floor(point.y())));
        auto& target = secondaryButton ? secondary : primary;
        if (target != color) {
            target = color;
            emit settingsChanged();
        }
        return;
    }
    if (tool_ == Tool::Fill) {
        if (busy_)
            return;
        finish();
        fillRevision_ = document.revision();
        const QImage snapshot = image();
        const auto color = secondaryButton ? secondary : primary;
        cancellation_ = std::make_shared<std::atomic_bool>(false);
        busy_ = filling_ = true;
        emit changed();
        fillWatcher_.setFuture(QtConcurrent::run(
            [snapshot, color, point, tolerance = tolerance, token = cancellation_] {
                return openpaint::fillRegion(
                    snapshot, QPoint(int(std::floor(point.x())), int(std::floor(point.y()))), color,
                    tolerance, token);
            }));
        return;
    }
    if (tool_ == Tool::Shape) {
        finish();
        if (!document.beginPreview())
            return;
        shapeStart_ = shapeEnd_ = point;
        shapeActive_ = true;
        shapeSecondary_ = secondaryButton;
        refreshShape();
        return;
    }
    const bool eraser = tool_ == Tool::Eraser;
    if (document.beginStroke(point, eraser ? secondary : (secondaryButton ? secondary : primary),
                             brushSize, eraser && transparentEraser,
                             tool_ == Tool::Brush ? brushStyle : -1,
                             tool_ == Tool::Brush ? opacity : 100)) {
        emit pixelsChanged(QRect());
        emit changed();
    }
}
void Editor::extend(QPointF point) {
    if (shapeActive_) {
        setShapeGeometry(shapeStart_.x(), shapeStart_.y(), point.x(), point.y());
        return;
    }
    const auto rect = document.extendStroke(point);
    if (!rect.isEmpty())
        emit pixelsChanged(rect);
}
void Editor::endGesture() {
    if (!shapeActive_)
        finish();
}
void Editor::setShapeGeometry(qreal x1, qreal y1, qreal x2, qreal y2) {
    if (!shapeActive_)
        return;
    for (qreal value : {x1, y1, x2, y2})
        if (!std::isfinite(value) || std::abs(value) > 1000000)
            return;
    shapeStart_ = {x1, y1};
    shapeEnd_ = {x2, y2};
    refreshShape();
}
void Editor::refreshShape() {
    if (!shapeActive_)
        return;
    const auto patch = openpaint::renderShape(document.previewBase(), shapeStart_, shapeEnd_,
                                              shapeKind, shapeSecondary_ ? secondary : primary,
                                              shapeSecondary_ ? primary : secondary, brushSize,
                                              shapeOutline, shapeFill);
    document.updatePreview(patch.image, patch.rect);
    emit pixelsChanged({});
    emit changed();
}
void Editor::finish() {
    if (!document.active())
        return;
    shapeActive_ = false;
    document.commitStroke();
    emit pixelsChanged(QRect());
    emit changed();
}
void Editor::cancel() {
    if (filling_)
        cancelIo();
    shapeActive_ = false;
    document.cancelStroke();
    emit pixelsChanged(QRect());
    emit changed();
}
void Editor::undo() {
    if (loading())
        return;
    finish();
    if (document.undo()) {
        emit pixelsChanged(QRect());
        emit changed();
    }
}
void Editor::redo() {
    if (loading())
        return;
    finish();
    if (document.redo()) {
        emit pixelsChanged(QRect());
        emit changed();
    }
}

Editor::Editor(QObject* parent) : QObject(parent) {
    connect(this, &Editor::settingsChanged, this, &Editor::refreshShape);
    connect(&fillWatcher_, &QFutureWatcher<openpaint::FillResult>::finished, this, [this] {
        const auto result = fillWatcher_.result();
        busy_ = filling_ = false;
        if (!cancellation_->load()) {
            if (result.failed)
                fail(tr("Fill could not be completed within the image limits."));
            else if (!result.rect.isEmpty()) {
                if (document.applyPatch(result.image, result.rect, fillRevision_))
                    emit pixelsChanged(result.rect);
                else
                    fail(tr("Fill was not applied because the document changed or undo memory was "
                            "exceeded."));
            }
        }
        emit changed();
    });
    connect(&watcher_, &QFutureWatcher<openpaint::ImageResult>::finished, this, [this] {
        const auto result = watcher_.result();
        const bool wasLoading = loading_;
        busy_ = loading_ = false;
        if (result.ok()) {
            if (wasLoading) {
                document.reset(result.image);
                fileUrl_ = operationUrl_;
                notice_ = result.notice;
                if (!notice_.isEmpty())
                    emit noticeChanged();
                emit documentReplaced();
                emit pixelsChanged(QRect());
            } else {
                document.markSaved(savingRevision_);
                fileUrl_ = operationUrl_;
            }
        } else if (!result.cancelled) {
            fail(result.error);
        }
        emit changed();
        if (!wasLoading)
            emit saved(result.ok());
    });
}
QString Editor::fileName() const {
    return fileUrl_.isEmpty() ? tr("Untitled") : QFileInfo(fileUrl_.toLocalFile()).fileName();
}
void Editor::fail(const QString& message) {
    error_ = message;
    emit errorChanged();
}
void Editor::newDocument() {
    if (busy_)
        return;
    finish();
    QImage image({1152, 648}, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);
    document.reset(image);
    fileUrl_ = QUrl();
    emit documentReplaced();
    emit pixelsChanged(QRect());
    emit changed();
}
void Editor::openFile(const QUrl& url) {
    if (busy_)
        return;
    finish();
    if (!url.isLocalFile()) {
        fail(tr("Choose a local PNG file."));
        return;
    }
    operationUrl_ = url;
    busy_ = loading_ = true;
    cancellation_ = std::make_shared<std::atomic_bool>(false);
    emit changed();
    watcher_.setFuture(QtConcurrent::run([path = url.toLocalFile(), token = cancellation_] {
        return openpaint::loadPng(path, token);
    }));
}
void Editor::saveFile(const QUrl& url) {
    if (busy_)
        return;
    finish();
    if (!url.isLocalFile() || !url.toLocalFile().endsWith(".png", Qt::CaseInsensitive)) {
        fail(tr("Choose a local filename ending in .png."));
        emit saved(false);
        return;
    }
    operationUrl_ = url;
    savingRevision_ = document.revision();
    const QImage snapshot = image();
    busy_ = true;
    cancellation_ = std::make_shared<std::atomic_bool>(false);
    emit changed();
    watcher_.setFuture(
        QtConcurrent::run([path = url.toLocalFile(), snapshot, token = cancellation_] {
            return openpaint::savePng(path, snapshot, token);
        }));
}
void Editor::cancelIo() {
    if (cancellation_)
        cancellation_->store(true);
}
