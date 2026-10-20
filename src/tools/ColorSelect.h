// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QImage>
#include <QRegion>
#include <atomic>
#include <memory>
namespace openpaint {
struct ColorSelectionResult {
    QRegion region;
    bool failed = false;
};
ColorSelectionResult selectColor(const QImage& source, QPoint seed,
                                 const std::shared_ptr<std::atomic_bool>& cancelled);
} // namespace openpaint
