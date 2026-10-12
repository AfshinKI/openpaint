#pragma once

#include <QColor>
#include <QImage>
#include <QPointF>
#include <QRect>
#include <vector>

namespace openpaint {
// GUI-thread owned. Consumers receive implicitly shared, immutable snapshots.
class Document {
  public:
    explicit Document(QSize size = {1152, 648}, qsizetype budget = 256 * 1024 * 1024);
    static bool validSize(QSize size);
    const QImage& image() const {
        return image_;
    }
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
    bool beginStroke(QPointF point, QColor color, qreal width, bool clearAlpha = false);
    QRect extendStroke(QPointF point);
    bool commitStroke();
    void cancelStroke();
    bool undo();
    bool redo();

  private:
    struct Patch {
        QRect rect;
        QImage before, after;
        quint64 from, to;
        qsizetype bytes() const {
            return before.sizeInBytes() + after.sizeInBytes();
        }
    };
    void apply(const Patch& patch, bool forward);
    QImage image_, original_;
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
