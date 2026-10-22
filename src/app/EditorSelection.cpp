// SPDX-License-Identifier: GPL-3.0-or-later
#include "Editor.h"
#include <QBuffer>
#include <QClipboard>
#include <QColorSpace>
#include <QGuiApplication>
#include <QMimeData>
#include <QPainter>
#include <QtConcurrent>

namespace {
QImage colorKeyImage(QImage image, QRgb key) {
    for (int y = 0; y < image.height(); ++y) {
        auto* pixels = reinterpret_cast<QRgb*>(image.scanLine(y));
        if (!pixels)
            return {};
        for (int x = 0; x < image.width(); ++x)
            if (pixels[x] == key)
                pixels[x] = 0;
    }
    return image;
}
} // namespace
void Editor::startColorSelection(QPoint seed) {
    if (busy_)
        return;
    finish();
    colorSelectionRevision_ = document.revision();
    const auto snapshot = image();
    cancellation_ = std::make_shared<std::atomic_bool>(false);
    busy_ = loading_ = true;
    emit changed();
    colorSelectionWatcher_.setFuture(QtConcurrent::run([snapshot, seed, token = cancellation_] {
        return openpaint::selectColor(snapshot, seed, token);
    }));
}
void Editor::finishColorSelection() {
    const auto result = colorSelectionWatcher_.result();
    busy_ = loading_ = false;
    if (!cancellation_->load() && document.revision() == colorSelectionRevision_) {
        if (result.failed)
            fail(tr("This color selection is too complex. Try a smaller image."));
        else if (!result.region.isEmpty()) {
            document.setSelection(result.region);
            emit pixelsChanged({});
        }
    }
    emit changed();
}
void Editor::finishSelection() {
    if (!selecting_)
        return;
    bool tooComplex = false;
    if (!selectionPoints_.isEmpty()) {
        const bool bounded =
            qint64(selectionPoints_.size()) * selectionPoints_.boundingRect().height() <= 4000000;
        const auto region =
            bounded && selectionPoints_.size() >= 3
                ? QRegion(selectionPoints_, Qt::OddEvenFill).intersected(image().rect())
                : QRegion();
        const bool complex = !bounded || region.rectCount() > 16384;
        document.setSelection(region.isEmpty() || complex ? previousSelection_ : region);
        tooComplex = complex;
    }
    selecting_ = false;
    selectionPoints_.clear();
    previousSelection_ = {};
    if (tooComplex)
        fail(tr("This selection path is too complex. Try a simpler outline."));
    emit pixelsChanged({});
    emit changed();
}
void Editor::invertSelection() {
    if (!canSelect() || !hasSelection())
        return;
    finish();
    document.setSelection(QRegion(image().rect()).subtracted(document.selection()));
    emit pixelsChanged({});
    emit changed();
}
QImage Editor::selectedImage(bool merged) const {
    if (!hasSelection() || previewActive())
        return {};
    const auto rect = selectionRect();
    QImage selected(rect.size(), image().format());
    if (selected.isNull())
        return {};
    selected.fill(Qt::transparent);
    selected.setColorSpace(image().colorSpace());
    QPainter painter(&selected);
    painter.setClipRegion(selection().translated(-rect.topLeft()));
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.drawImage(-rect.topLeft(), merged ? document.composite() : image());
    return selected;
}
bool Editor::copySelection() {
    return startCopy(false);
}
bool Editor::copyVisibleLayers() {
    return startCopy(false, true);
}
void Editor::cutSelection() {
    startCopy(true);
}
bool Editor::startCopy(bool cut, bool merged) {
    if (!canSelect() || !hasSelection() || busy_)
        return false;
    finish();
    const auto selected = selectedImage(merged);
    if (selected.isNull()) {
        fail(tr("The selection could not be copied. The drawing is unchanged."));
        return false;
    }
    cutAfterCopy_ = cut;
    busy_ = loading_ = clipboardBusy_ = true;
    cancellation_ = std::make_shared<std::atomic_bool>(false);
    emit changed();
    copyWatcher_.setFuture(
        QtConcurrent::run([selected, token = cancellation_, transparent = transparentSelection,
                           key = qPremultiply(secondary.rgba())] {
            QByteArray png;
            QBuffer buffer(&png);
            buffer.open(QIODevice::WriteOnly);
            if (token->load())
                return QByteArray();
            const auto pixels = transparent ? colorKeyImage(selected, key) : selected;
            if (!openpaint::writePng(buffer, pixels) || token->load())
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
    floatingComposite_ = {};
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
    floatingComposite_ = {};
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
    if (!floatingActive_)
        return;
    const auto key = qPremultiply(secondary.rgba());
    if (floatingComposite_.isNull() || floatingTransparent_ != transparentSelection ||
        (transparentSelection && key != floatingColorKey_)) {
        floatingComposite_ =
            transparentSelection ? colorKeyImage(floatingImage_, key) : floatingImage_;
        if (floatingComposite_.isNull()) {
            cancel();
            fail(tr("There is not enough memory to preview this selection."));
            return;
        }
        floatingTransparent_ = transparentSelection;
        floatingColorKey_ = key;
    }
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
            painter.setCompositionMode(QPainter::CompositionMode_Source);
            painter.fillRect(floatingOriginalSelection_.boundingRect(),
                             needsProject() ? QColor(Qt::transparent) : secondary);
        }
        painter.setClipRegion(destination);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.drawImage(floatingPosition_, floatingComposite_);
    }
    document.updatePreview(patch, rect);
    emit pixelsChanged({});
    emit changed();
}
