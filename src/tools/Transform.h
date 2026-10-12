// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QImage>
#include <QRegion>
#include <QString>
#include <atomic>
#include <memory>
namespace openpaint {
enum class TransformKind {
    RotateRight,
    RotateLeft,
    RotateHalf,
    FlipHorizontal,
    FlipVertical,
    Crop
};
struct TransformResult {
    QImage image;
    QRegion mask;
    QString error;
    bool cancelled = false;
};
TransformResult transformImage(const QImage& source, const QRegion& mask, TransformKind kind,
                               const std::shared_ptr<std::atomic_bool>& token);
} // namespace openpaint
