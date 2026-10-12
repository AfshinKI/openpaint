// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QColor>
#include <QImage>
#include <QRect>
#include <atomic>
#include <memory>
namespace openpaint {
struct FillResult {
    QImage image;
    QRect rect;
    bool failed = false;
};
FillResult fillRegion(const QImage& source, QPoint seed, QColor color, int tolerance,
                      const std::shared_ptr<std::atomic_bool>& cancelled);
} // namespace openpaint
