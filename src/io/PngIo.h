// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QImage>
#include <QString>
#include <atomic>
#include <functional>
#include <memory>
class QSaveFile;
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
ImageResult loadPng(const QString& path, const Cancellation& cancellation = {});
// Commit injection exercises replacement failures without special filesystem privileges.
ImageResult savePng(const QString& path, const QImage& image, const Cancellation& cancellation = {},
                    const std::function<bool(QSaveFile&)>& commit = {});
} // namespace openpaint
