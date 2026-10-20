// SPDX-License-Identifier: GPL-3.0-or-later
#include "ColorSelect.h"
#include "document/Document.h"
#include <vector>
namespace openpaint {
ColorSelectionResult selectColor(const QImage& source, QPoint seed,
                                 const std::shared_ptr<std::atomic_bool>& cancelled) {
    if (!cancelled || !Document::validSize(source.size()) || !source.rect().contains(seed) ||
        source.format() != QImage::Format_ARGB32_Premultiplied)
        return {{}, true};
    try {
        constexpr size_t MaxRects = 16384;
        const auto target = source.pixel(seed);
        std::vector<QRect> rectangles, row, previous;
        size_t bandStart = 0;
        for (int y = 0; y < source.height(); ++y) {
            if (cancelled->load())
                return {};
            row.clear();
            const auto* pixels = reinterpret_cast<const QRgb*>(source.constScanLine(y));
            for (int x = 0; x < source.width();) {
                if (pixels[x] != target) {
                    ++x;
                    continue;
                }
                const int start = x++;
                while (x < source.width() && pixels[x] == target)
                    ++x;
                row.emplace_back(start, 0, x - start, 1);
            }
            if (row == previous) {
                for (size_t i = bandStart; i < rectangles.size(); ++i)
                    rectangles[i].setHeight(rectangles[i].height() + 1);
            } else {
                if (rectangles.size() + row.size() > MaxRects)
                    return {{}, true};
                bandStart = rectangles.size();
                for (auto rect : row) {
                    rect.moveTop(y);
                    rectangles.push_back(rect);
                }
                previous = row;
            }
        }
        if (cancelled->load())
            return {};
        QRegion region;
        // Runs are non-overlapping and sorted in complete horizontal bands.
        region.setRects(rectangles.data(), int(rectangles.size()));
        return {region, false};
    } catch (const std::bad_alloc&) {
        return {{}, true};
    }
}
} // namespace openpaint
