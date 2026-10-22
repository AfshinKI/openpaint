// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "document/Document.h"
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
    Crop,
    ResizeSkew,
    CanvasResize,
    InvertColors
};
struct TransformOptions {
    QSize size;
    qreal skewX = 0, skewY = 0;
    bool smooth = true;
    QPoint canvasOrigin;
    QColor background = Qt::white;
};
struct TransformResult {
    QImage image;
    QRegion mask;
    QString error;
    bool cancelled = false;
    Layers layers = {};
};
TransformResult transformImage(const QImage& source, const QRegion& mask, TransformKind kind,
                               const std::shared_ptr<std::atomic_bool>& token,
                               const TransformOptions& options = {});
} // namespace openpaint
