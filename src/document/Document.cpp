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
    image_ = QImage(size, QImage::Format_ARGB32_Premultiplied);
    image_.fill(Qt::white);
    image_.setColorSpace(QColorSpace::SRgb);
}
bool Document::reset(const QImage& image) {
    if (image.isNull() || !validSize(image.size()))
        return false;
    image_ = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    image_.setColorSpace(QColorSpace::SRgb);
    original_ = {};
    brush_.clear();
    active_ = false;
    history_.clear();
    cursor_ = 0;
    historyBytes_ = 0;
    revision_ = nextRevision_++;
    savedRevision_ = revision_;
    return true;
}
bool Document::beginStroke(QPointF point, QColor color, qreal width, bool clearAlpha,
                           int brushStyle, int opacity) {
    if (active_ || !std::isfinite(point.x()) || !std::isfinite(point.y()) ||
        std::abs(point.x()) > 1000000 || std::abs(point.y()) > 1000000 || !std::isfinite(width) ||
        width <= 0 || width > 256 || !color.isValid())
        return false;
    if (brushStyle < -1 || brushStyle > 8 || opacity < 0 || opacity > 100)
        return false;
    brushed_ = brushStyle >= 0;
    if (brushed_ && !brush_.begin(image_.size(), point, color, width, brushStyle, opacity))
        return false;
    original_ = image_;
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
        const auto rect = brush_.extend(image_, original_, point);
        dirtyRect_ = dirtyRect_.united(rect);
        previous_ = point;
        return rect;
    }
    const qreal margin = width_ / 2 + 2;
    QRect rect = QRectF(previous_, point)
                     .normalized()
                     .adjusted(-margin, -margin, margin, margin)
                     .toAlignedRect()
                     .intersected(image_.rect());
    if (!rect.isEmpty()) {
        QPainter painter(&image_);
        painter.setRenderHint(QPainter::Antialiasing);
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
    brush_.clear();
    if (dirtyRect_.isEmpty() || original_.copy(dirtyRect_) == image_.copy(dirtyRect_)) {
        original_ = {};
        return false;
    }
    if (qint64(dirtyRect_.width()) * dirtyRect_.height() * 8 > budget_) {
        image_ = original_;
        original_ = {};
        return false;
    }
    while (history_.size() > cursor_) {
        historyBytes_ -= history_.back().bytes();
        history_.pop_back();
    }
    Patch patch{dirtyRect_, original_.copy(dirtyRect_), image_.copy(dirtyRect_), revision_,
                nextRevision_++};
    revision_ = patch.to;
    original_ = {};
    historyBytes_ += patch.bytes();
    history_.push_back(std::move(patch));
    cursor_ = history_.size();
    while (!history_.empty() && historyBytes_ > budget_) {
        historyBytes_ -= history_.front().bytes();
        history_.erase(history_.begin());
        --cursor_;
    }
    return true;
}
bool Document::applyPatch(const QImage& candidate, QRect rect, quint64 sourceRevision) {
    if (active_ || sourceRevision != revision_ || candidate.size() != image_.size() ||
        candidate.format() != image_.format() || rect.isEmpty() || !image_.rect().contains(rect) ||
        qint64(rect.width()) * rect.height() * 8 > budget_)
        return false;
    original_ = image_;
    active_ = true;
    dirtyRect_ = rect;
    {
        QPainter painter(&image_);
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.drawImage(rect, candidate, rect);
    }
    return commitStroke();
}
void Document::cancelStroke() {
    brush_.clear();
    if (active_)
        image_ = original_;
    original_ = {};
    active_ = false;
}
void Document::apply(const Patch& patch, bool forward) {
    QPainter painter(&image_);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.drawImage(patch.rect.topLeft(), forward ? patch.after : patch.before);
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
} // namespace openpaint
