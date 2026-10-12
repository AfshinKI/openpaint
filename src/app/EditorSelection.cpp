// SPDX-License-Identifier: GPL-3.0-or-later
#include "Editor.h"
#include <QBuffer>
#include <QClipboard>
#include <QGuiApplication>
#include <QMimeData>
#include <QPainter>
#include <QtConcurrent>

QImage Editor::selectedImage() const {
    if (!hasSelection() || previewActive())
        return {};
    const auto rect = selectionRect();
    QImage selected(rect.size(), image().format());
    if (selected.isNull())
        return {};
    selected.fill(Qt::transparent);
    QPainter painter(&selected);
    painter.setClipRegion(selection().translated(-rect.topLeft()));
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.drawImage(-rect.topLeft(), image());
    return selected;
}
bool Editor::copySelection() {
    return startCopy(false);
}
void Editor::cutSelection() {
    startCopy(true);
}
bool Editor::startCopy(bool cut) {
    if (!canSelect() || !hasSelection() || busy_)
        return false;
    finish();
    const auto selected = selectedImage();
    if (selected.isNull()) {
        fail(tr("The selection could not be copied. The drawing is unchanged."));
        return false;
    }
    cutAfterCopy_ = cut;
    busy_ = loading_ = clipboardBusy_ = true;
    cancellation_ = std::make_shared<std::atomic_bool>(false);
    emit changed();
    copyWatcher_.setFuture(QtConcurrent::run([selected, token = cancellation_] {
        QByteArray png;
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        if (token->load() || !openpaint::writePng(buffer, selected) || token->load())
            return QByteArray();
        return png;
    }));
    return true;
}
void Editor::finishCopy() {
    const auto png = copyWatcher_.result();
    busy_ = loading_ = clipboardBusy_ = false;
    if (!cancellation_->load()) {
        if (png.isEmpty()) {
            fail(tr("The selection could not be copied. The drawing is unchanged."));
        } else {
            auto* mime = new QMimeData;
            mime->setData(QStringLiteral("image/png"), png);
            QGuiApplication::clipboard()->setMimeData(mime, QClipboard::Clipboard);
            if (cutAfterCopy_)
                deleteSelection();
        }
    }
    cutAfterCopy_ = false;
    emit changed();
}
void Editor::pasteClipboard() {
    if (!canSelect() || busy_)
        return;
    const auto* mime = QGuiApplication::clipboard()->mimeData(QClipboard::Clipboard);
    if (!mime || !mime->hasFormat(QStringLiteral("image/png"))) {
        fail(tr("The clipboard has no PNG image. Copy an image as PNG, then try again."));
        return;
    }
    const auto bytes = mime->data(QStringLiteral("image/png"));
    if (bytes.size() > 128 * 1024 * 1024) {
        fail(tr("The clipboard image exceeds the 128 MiB PNG limit."));
        return;
    }
    finish();
    busy_ = loading_ = pasting_ = clipboardBusy_ = true;
    cancellation_ = std::make_shared<std::atomic_bool>(false);
    emit changed();
    watcher_.setFuture(QtConcurrent::run(
        [bytes, token = cancellation_] { return openpaint::decodePng(bytes, token); }));
}
bool Editor::pasteImage(const QImage& source) {
    if (!canSelect() || source.isNull() || !openpaint::Document::validSize(source.size()))
        return false;
    finish();
    floatingImage_ = source.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    if (floatingImage_.isNull())
        return false;
    setTool(Tool::Select);
    floatingOriginalSelection_ = document.selection();
    document.setSelection({});
    if (!document.beginPreview()) {
        document.setSelection(floatingOriginalSelection_);
        floatingImage_ = {};
        return false;
    }
    floatingActive_ = true;
    floatingMove_ = floatingDragging_ = false;
    floatingPosition_ = {};
    floatingRegion_ = floatingImage_.rect();
    refreshFloating();
    return true;
}
void Editor::beginMove(QPoint point) {
    floatingImage_ = selectedImage();
    if (floatingImage_.isNull())
        return;
    floatingOriginalSelection_ = document.selection();
    floatingPosition_ = floatingOriginalSelection_.boundingRect().topLeft();
    floatingRegion_ = floatingOriginalSelection_.translated(-floatingPosition_);
    floatingDragOffset_ = point - floatingPosition_;
    document.setSelection({});
    if (!document.beginPreview()) {
        document.setSelection(floatingOriginalSelection_);
        floatingImage_ = {};
        return;
    }
    floatingActive_ = floatingMove_ = floatingDragging_ = true;
    refreshFloating();
}
void Editor::nudgeSelection(int dx, int dy) {
    if (dx < -16384 || dx > 16384 || dy < -16384 || dy > 16384 || loading() ||
        (previewActive() && !floatingActive_) || !hasSelection())
        return;
    if (!floatingActive_) {
        setTool(Tool::Select);
        beginMove(selectionRect().topLeft());
        floatingDragging_ = false;
    }
    moveFloating(floatingPosition_.x() + dx, floatingPosition_.y() + dy);
}
void Editor::moveFloating(int x, int y) {
    if (!floatingActive_ || x <= -floatingImage_.width() || y <= -floatingImage_.height() ||
        x >= image().width() || y >= image().height())
        return;
    floatingPosition_ = {x, y};
    refreshFloating();
}
void Editor::refreshFloating() {
    const auto destination = selection().intersected(image().rect());
    const auto affected =
        floatingMove_ ? destination.united(floatingOriginalSelection_) : destination;
    const auto rect = affected.boundingRect();
    QImage patch = document.previewBase().copy(rect);
    if (patch.isNull())
        return;
    {
        QPainter painter(&patch);
        painter.translate(-rect.topLeft());
        if (floatingMove_) {
            painter.setClipRegion(floatingOriginalSelection_);
            painter.setCompositionMode(QPainter::CompositionMode_Clear);
            painter.fillRect(floatingOriginalSelection_.boundingRect(), Qt::transparent);
        }
        painter.setClipRegion(destination);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.drawImage(floatingPosition_, floatingImage_);
    }
    document.updatePreview(patch, rect);
    emit pixelsChanged({});
    emit changed();
}
