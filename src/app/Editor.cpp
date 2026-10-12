// SPDX-License-Identifier: GPL-3.0-or-later
#include "Editor.h"
#include <QFileInfo>
#include <QtConcurrent>
#include <cmath>
void Editor::begin(QPointF point, bool secondaryButton) {
    if (loading_)
        return;
    if (!std::isfinite(point.x()) || !std::isfinite(point.y()) || point.x() < 0 || point.y() < 0 ||
        point.x() >= image().width() || point.y() >= image().height())
        return;
    if (document.beginStroke(point, eraser ? secondary : (secondaryButton ? secondary : primary),
                             brushSize, eraser && transparentEraser)) {
        emit pixelsChanged(QRect());
        emit changed();
    }
}
void Editor::extend(QPointF point) {
    const auto rect = document.extendStroke(point);
    if (!rect.isEmpty())
        emit pixelsChanged(rect);
}
void Editor::finish() {
    if (!document.active())
        return;
    document.commitStroke();
    emit pixelsChanged(QRect());
    emit changed();
}
void Editor::cancel() {
    document.cancelStroke();
    emit pixelsChanged(QRect());
    emit changed();
}
void Editor::undo() {
    if (loading_)
        return;
    finish();
    if (document.undo()) {
        emit pixelsChanged(QRect());
        emit changed();
    }
}
void Editor::redo() {
    if (loading_)
        return;
    finish();
    if (document.redo()) {
        emit pixelsChanged(QRect());
        emit changed();
    }
}

Editor::Editor(QObject* parent) : QObject(parent) {
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
