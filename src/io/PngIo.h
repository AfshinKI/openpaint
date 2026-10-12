// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QImage>
#include <QString>
#include <atomic>
#include <functional>
#include <memory>
class QSaveFile;
class QIODevice;
namespace openpaint {
using Cancellation = std::shared_ptr<std::atomic_bool>;
struct ImageResult {
    QImage image;
    QString error;
    bool cancelled = false;
    QString notice = {};
    bool ok() const {
        return error.isEmpty() && !cancelled;
    }
};
// Encode while deferring device errors until the codec has released its resources.
bool writePng(QIODevice& device, const QImage& image);
ImageResult decodePng(const QByteArray& bytes, const Cancellation& cancellation = {});
ImageResult loadPng(const QString& path, const Cancellation& cancellation = {});
// Commit injection exercises replacement failures without special filesystem privileges.
ImageResult savePng(const QString& path, const QImage& image, const Cancellation& cancellation = {},
                    const std::function<bool(QSaveFile&)>& commit = {},
                    const std::function<bool(QIODevice&, const QImage&)>& encode = {});
} // namespace openpaint
