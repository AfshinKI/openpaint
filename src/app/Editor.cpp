// SPDX-License-Identifier: GPL-3.0-or-later
#include "Editor.h"
#include "io/ImageIo.h"
#include "tools/Shape.h"
#include "tools/Text.h"
#include <QFileInfo>
#include <QGuiApplication>
#include <QInputMethod>
#include <QtConcurrent>
#include <cmath>
#include <numbers>
void Editor::setEditingSecondary(bool active) {
    if (active == editingSecondary_)
        return;
    editingSecondary_ = active;
    emit settingsChanged();
}
void Editor::setSelectedColor(QColor color) {
    if (!color.isValid() || color == selectedColor())
        return;
    if (!previewActive())
        finish();
    (editingSecondary_ ? secondary : primary) = color;
    emit settingsChanged();
}
void Editor::setTool(Tool tool) {
    if (textActive_ || (loading() && !filling_))
        return;
    if (tool == tool_ || (tool != Tool::Pencil && tool != Tool::Eraser &&
                          tool != Tool::ColorPicker && tool != Tool::Fill && tool != Tool::Brush &&
                          tool != Tool::Shape && tool != Tool::Text && tool != Tool::Select))
        return;
    if (filling_)
        cancelIo();
    finish();
    document.setSelection({});
    tool_ = tool;
    emit pixelsChanged({});
    emit changed();
    emit settingsChanged();
}
void Editor::begin(QPointF point, bool secondaryButton) {
    shapeInputBlocked_ = false;
    if (loading() || textActive_)
        return;
    if (!std::isfinite(point.x()) || !std::isfinite(point.y()) || std::abs(point.x()) > 1000000 ||
        std::abs(point.y()) > 1000000)
        return;
    // A dismissal consumes the complete gesture, including presses in the workspace.
    if (tool_ == Tool::Select && hasSelection() &&
        !selection().contains(QPoint(int(std::floor(point.x())), int(std::floor(point.y()))))) {
        finish();
        document.setSelection({});
        emit pixelsChanged({});
        emit changed();
        return;
    }
    if (shapeActive_ && shapeKind != openpaint::PolygonShape) {
        if (!resizeBounds().contains(point))
            finish();
        shapeInputBlocked_ = true;
        return;
    }
    if (point.x() < 0 || point.y() < 0 || point.x() >= image().width() ||
        point.y() >= image().height())
        return;
    if (tool_ == Tool::Select) {
        const QPoint pixel(int(std::floor(point.x())), int(std::floor(point.y())));
        if (floatingActive_ && selection().contains(pixel)) {
            floatingDragging_ = true;
            floatingDragOffset_ = pixel - floatingPosition_;
            return;
        }
        if (!floatingActive_ && document.selection().contains(pixel)) {
            beginMove(pixel);
            return;
        }
        finish();
        previousSelection_ = document.selection();
        selectionAnchor_ = QPoint(int(std::floor(point.x())), int(std::floor(point.y())));
        selecting_ = true;
        if (freeFormSelection) {
            selectionPoints_ = {selectionAnchor_};
            document.setSelection({});
        } else {
            document.setSelection(QRect(selectionAnchor_, QSize(1, 1)));
        }
        emit pixelsChanged({});
        emit changed();
        return;
    }
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
        fillWatcher_.setFuture(
            QtConcurrent::run([snapshot, color, point, tolerance = 0, token = cancellation_,
                               selection = document.selection(), smoothEdges = true] {
                return openpaint::fillRegion(
                    snapshot, QPoint(int(std::floor(point.x())), int(std::floor(point.y()))), color,
                    tolerance, token, selection, smoothEdges);
            }));
        return;
    }
    if (tool_ == Tool::Text) {
        finish();
        if (!document.beginPreview())
            return;
        textAnchor_ = point;
        textActive_ = textDrawing_ = true;
        textSecondary_ = secondaryButton;
        textDraft_.clear();
        textRect_ =
            QRect(QPoint(int(std::floor(point.x())), int(std::floor(point.y()))), QSize(300, 120))
                .intersected(image().rect());
        emit textDraftChanged();
        refreshText();
        return;
    }
    if (tool_ == Tool::Shape) {
        if (shapeActive_ && shapeKind == openpaint::PolygonShape) {
            if (shapePoints_.size() >= 4096) {
                shapeInputBlocked_ = true;
                fail(tr("A polygon can contain at most 4096 points."));
                return;
            }
            shapePoints_.append(point);
            shapeEnd_ = point;
            refreshShape();
            return;
        }
        finish();
        if (!document.beginPreview())
            return;
        shapeStart_ = shapeEnd_ = point;
        shapePoints_ = {point, point};
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
void Editor::extend(QPointF point, bool constrain) {
    if (floatingActive_) {
        if (floatingDragging_ && std::isfinite(point.x()) && std::isfinite(point.y()) &&
            std::abs(point.x()) <= 1000000 && std::abs(point.y()) <= 1000000)
            moveFloating(int(std::floor(point.x())) - floatingDragOffset_.x(),
                         int(std::floor(point.y())) - floatingDragOffset_.y());
        return;
    }
    if (selecting_) {
        if (std::isfinite(point.x()) && std::isfinite(point.y())) {
            const QPoint end(
                int(std::clamp(std::floor(point.x()), 0.0, double(image().width() - 1))),
                int(std::clamp(std::floor(point.y()), 0.0, double(image().height() - 1))));
            if (!selectionPoints_.isEmpty()) {
                if (selectionPoints_.last() != end) {
                    if (selectionPoints_.size() >= 4096) {
                        cancel();
                        fail(tr("A selection path can contain at most 4096 points."));
                        return;
                    }
                    selectionPoints_.append(end);
                }
            } else {
                document.setSelection(QRect(QPoint(std::min(selectionAnchor_.x(), end.x()),
                                                   std::min(selectionAnchor_.y(), end.y())),
                                            QPoint(std::max(selectionAnchor_.x(), end.x()),
                                                   std::max(selectionAnchor_.y(), end.y()))));
            }
            emit pixelsChanged({});
            emit changed();
        }
        return;
    }
    if (textActive_) {
        if (textDrawing_ && std::isfinite(point.x()) && std::isfinite(point.y()) &&
            std::abs(point.x()) <= 1000000 && std::abs(point.y()) <= 1000000) {
            const auto rect =
                QRectF(textAnchor_, point).normalized().toAlignedRect().intersected(image().rect());
            if (rect.width() >= 8 && rect.height() >= 8) {
                textRect_ = rect;
                refreshText();
            }
        }
        return;
    }
    if (shapeInputBlocked_)
        return;
    if (shapeActive_) {
        if (!std::isfinite(point.x()) || !std::isfinite(point.y()))
            return;
        if (constrain) {
            const QPointF anchor = shapeKind == openpaint::PolygonShape && shapePoints_.size() >= 2
                                       ? shapePoints_[shapePoints_.size() - 2]
                                       : shapeStart_;
            auto delta = point - anchor;
            if (shapeKind == 0 || shapeKind == openpaint::CurveShape ||
                shapeKind == openpaint::PolygonShape) {
                const auto angle =
                    std::round(std::atan2(delta.y(), delta.x()) / (std::numbers::pi / 4)) *
                    (std::numbers::pi / 4);
                const auto length = std::hypot(delta.x(), delta.y());
                point = anchor + QPointF(std::cos(angle) * length, std::sin(angle) * length);
            } else {
                const auto side = std::max(std::abs(delta.x()), std::abs(delta.y()));
                point = anchor +
                        QPointF(std::copysign(side, delta.x()), std::copysign(side, delta.y()));
            }
        }
        setShapeGeometry(shapeStart_.x(), shapeStart_.y(), point.x(), point.y());
        return;
    }
    const auto rect = document.extendStroke(point);
    if (!rect.isEmpty())
        emit pixelsChanged(rect);
}
void Editor::endGesture() {
    if (floatingActive_) {
        floatingDragging_ = false;
        return;
    }
    if (selecting_) {
        finishSelection();
        return;
    }
    if (textActive_) {
        if (textDrawing_) {
            textDrawing_ = false;
            emit changed();
            emit textEditingReady();
        }
        return;
    }
    shapeInputBlocked_ = false;
    if (!shapeActive_)
        finish();
}
void Editor::removeShapePoint() {
    if (!shapeActive_ || shapeKind != openpaint::PolygonShape || shapePoints_.size() <= 2)
        return;
    shapePoints_.removeLast();
    shapeEnd_ = shapePoints_.last();
    refreshShape();
}
void Editor::setShapeGeometry(qreal x1, qreal y1, qreal x2, qreal y2) {
    if (!shapeActive_)
        return;
    for (qreal value : {x1, y1, x2, y2})
        if (!std::isfinite(value) || std::abs(value) > 1000000)
            return;
    shapeStart_ = {x1, y1};
    shapeEnd_ = {x2, y2};
    if (!shapePoints_.isEmpty()) {
        shapePoints_.first() = shapeStart_;
        shapePoints_.last() = shapeEnd_;
    }
    refreshShape();
}
void Editor::refreshShape() {
    if (!shapeActive_)
        return;
    const auto patch = openpaint::renderShape(
        document.previewBase(), shapeStart_, shapeEnd_, shapeKind,
        shapeSecondary_ ? secondary : primary, shapeSecondary_ ? primary : secondary, brushSize,
        shapeOutline, shapeFill, curveBend1, curveBend2, shapePoints_);
    document.updatePreview(patch.image, patch.rect);
    emit pixelsChanged({});
    emit changed();
}
bool Editor::setTextDraft(const QString& text) {
    if (!textActive_)
        return false;
    if (text.size() > 10000) {
        fail(tr("A text box can contain at most 10,000 characters."));
        emit textDraftChanged();
        return false;
    }
    if (textDraft_ == text)
        return true;
    textDraft_ = text;
    emit textDraftChanged();
    refreshText();
    return true;
}
void Editor::setTextBounds(int x, int y, int width, int height) {
    if (!textActive_ || width < 1 || height < 1 || width > 16384 || height > 16384 || x < 0 ||
        y < 0 || x >= image().width() || y >= image().height())
        return;
    textRect_ = QRect(x, y, width, height).intersected(image().rect());
    refreshText();
}
void Editor::refreshText() {
    if (!textActive_ || textSize < 1 || textSize > 512 || textFamily.size() > 128)
        return;
    QFont font(textFamily);
    font.setPixelSize(textSize);
    font.setBold(textBold);
    font.setItalic(textItalic);
    font.setUnderline(textUnderline);
    font.setStrikeOut(textStrikeout);
    const auto patch = openpaint::renderText(
        document.previewBase(), textRect_, textDraft_,
        {font, textColor(), textBackgroundColor(), textBackground, textAlignment});
    if (!patch.rect.isEmpty())
        document.updatePreview(patch.image, patch.rect);
    emit pixelsChanged({});
    emit changed();
}
void Editor::selectRect(int x, int y, int width, int height) {
    if (!canSelect() || x < 0 || y < 0 || width < 1 || height < 1 || width > 16384 ||
        height > 16384 || x >= image().width() || y >= image().height())
        return;
    finish();
    document.setSelection(QRect(x, y, width, height).intersected(image().rect()));
    emit pixelsChanged({});
    emit changed();
}
void Editor::selectAll() {
    if (loading() || textActive_)
        return;
    finish();
    setTool(Tool::Select);
    selectRect(0, 0, image().width(), image().height());
}
void Editor::deselect() {
    if (!canSelect())
        return;
    finish();
    document.setSelection({});
    emit pixelsChanged({});
    emit changed();
}
void Editor::deleteSelection() {
    if (!canSelect() || !hasSelection())
        return;
    finish();
    if (document.deleteSelection(secondary)) {
        emit pixelsChanged({});
        emit changed();
    }
}
void Editor::commitPreview() {
    if (textActive_)
        QGuiApplication::inputMethod()->commit();
    finish();
}
void Editor::finish() {
    finishSelection();
    if (shapeActive_ && shapeKind == openpaint::PolygonShape && shapePoints_.size() < 3) {
        cancel();
        return;
    }
    if (!document.active())
        return;
    shapeActive_ = textActive_ = textDrawing_ = false;
    document.commitStroke();
    if (floatingActive_) {
        document.setSelection(selection().intersected(image().rect()));
        floatingActive_ = floatingDragging_ = false;
        floatingImage_ = floatingComposite_ = {};
        floatingRegion_ = floatingOriginalSelection_ = {};
    }
    emit pixelsChanged(QRect());
    emit changed();
}
void Editor::cancel() {
    if (selecting_) {
        document.setSelection(previousSelection_);
        selecting_ = false;
        selectionPoints_.clear();
        previousSelection_ = {};
    } else if (!document.active() && !loading()) {
        document.setSelection({});
    }
    if (loading())
        cancelIo();
    shapeActive_ = textActive_ = textDrawing_ = false;
    document.cancelStroke();
    if (floatingActive_) {
        document.setSelection(floatingOriginalSelection_);
        floatingActive_ = floatingDragging_ = false;
        floatingImage_ = floatingComposite_ = {};
        floatingRegion_ = floatingOriginalSelection_ = {};
    }
    emit pixelsChanged(QRect());
    emit changed();
}
void Editor::undo() {
    if (loading())
        return;
    if (previewActive()) {
        cancel();
        return;
    }
    finish();
    const auto size = imageSize();
    if (document.undo()) {
        if (imageSize() != size)
            emit documentReplaced();
        emit pixelsChanged(QRect());
        emit changed();
    }
}
void Editor::redo() {
    if (loading() || previewActive())
        return;
    finish();
    const auto size = imageSize();
    if (document.redo()) {
        if (imageSize() != size)
            emit documentReplaced();
        emit pixelsChanged(QRect());
        emit changed();
    }
}

Editor::Editor(QObject* parent) : QObject(parent) {
    connect(&transformWatcher_, &QFutureWatcher<openpaint::TransformResult>::finished, this,
            &Editor::finishTransform);
    connect(&copyWatcher_, &QFutureWatcher<QByteArray>::finished, this, &Editor::finishCopy);
    connect(this, &Editor::settingsChanged, this, &Editor::refreshShape);
    connect(this, &Editor::settingsChanged, this, &Editor::refreshText);
    connect(this, &Editor::settingsChanged, this, &Editor::refreshFloating);
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
        const bool wasPasting = pasting_;
        const bool wasExporting = exporting_;
        busy_ = loading_ = pasting_ = clipboardBusy_ = exporting_ = false;
        if (result.ok()) {
            if (wasExporting) {
                notice_ = tr("Exported a copy. Save still preserves your working drawing as PNG.");
                emit noticeChanged();
            } else if (wasPasting) {
                if (!cancellation_ || !cancellation_->load())
                    pasteImage(result.image);
            } else if (wasLoading && (!cancellation_ || !cancellation_->load())) {
                document.reset(result.image);
                fileUrl_ = result.format == "png" &&
                                   operationUrl_.toLocalFile().endsWith(".png", Qt::CaseInsensitive)
                               ? operationUrl_
                               : QUrl();
                sourceName_ = QFileInfo(operationUrl_.toLocalFile()).fileName();
                notice_ = result.notice;
                if (!notice_.isEmpty())
                    emit noticeChanged();
                emit documentReplaced();
                emit pixelsChanged(QRect());
            } else if (!wasLoading) {
                document.markSaved(savingRevision_);
                fileUrl_ = operationUrl_;
            }
        } else if (!result.cancelled) {
            fail(result.error);
        }
        emit changed();
        if (wasExporting)
            emit exported(result.ok());
        else if (!wasLoading)
            emit saved(result.ok());
    });
}
QString Editor::fileName() const {
    return fileUrl_.isEmpty() ? (sourceName_.isEmpty() ? tr("Untitled") : sourceName_)
                              : QFileInfo(fileUrl_.toLocalFile()).fileName();
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
    sourceName_.clear();
    emit documentReplaced();
    emit pixelsChanged(QRect());
    emit changed();
}
void Editor::openFile(const QUrl& url) {
    if (busy_)
        return;
    finish();
    if (!url.isLocalFile()) {
        fail(tr("Choose a local image file."));
        return;
    }
    operationUrl_ = url;
    busy_ = loading_ = true;
    cancellation_ = std::make_shared<std::atomic_bool>(false);
    emit changed();
    watcher_.setFuture(QtConcurrent::run([path = url.toLocalFile(), token = cancellation_] {
        return openpaint::loadImage(path, token);
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
void Editor::exportFile(const QUrl& url, int format, int quality, bool lossless, bool flatten,
                        QColor background) {
    if (busy_ || document.active() || previewActive())
        return;
    if (!url.isLocalFile() || format < 0 || format > 3 ||
        !openpaint::exportSuffix(url.toLocalFile(), openpaint::ExportFormat(format))) {
        fail(tr("Choose a local filename matching the export format."));
        emit exported(false);
        return;
    }
    const openpaint::ExportOptions options{openpaint::ExportFormat(format), quality, lossless,
                                           flatten, background};
    const QImage snapshot = image();
    busy_ = exporting_ = true;
    cancellation_ = std::make_shared<std::atomic_bool>(false);
    emit changed();
    watcher_.setFuture(
        QtConcurrent::run([path = url.toLocalFile(), snapshot, options, token = cancellation_] {
            return openpaint::exportImage(path, snapshot, options, token);
        }));
}
void Editor::cancelIo() {
    if (cancellation_)
        cancellation_->store(true);
}
