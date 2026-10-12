// SPDX-License-Identifier: GPL-3.0-or-later
#include "Fill.h"
#include "document/Document.h"
#include <algorithm>
#include <cmath>
#include <vector>
namespace openpaint {
FillResult fillRegion(const QImage& source, QPoint seed, QColor color, int tolerance,
                      const std::shared_ptr<std::atomic_bool>& cancelled) {
    if (!Document::validSize(source.size()) || !source.rect().contains(seed) ||
        source.format() != QImage::Format_ARGB32_Premultiplied || !color.isValid() ||
        tolerance < 0 || tolerance > 255 || !cancelled)
        return {{}, {}, true};
    if (cancelled->load())
        return {};
    try {
        const QRgb target = source.pixel(seed), replacement = qPremultiply(color.rgba());
        if (tolerance == 0 && target == replacement)
            return {};
        QImage output = source.copy();
        if (output.isNull())
            return {{}, {}, true};
        const int width = source.width(), height = source.height();
        std::vector<unsigned char> seen(size_t(width) * height, 0);
        std::vector<int> queue;
        queue.reserve(size_t(width) * height);
        auto visit = [&](int x, int y) {
            const int index = y * width + x;
            if (seen[index])
                return;
            seen[index] = 1;
            const auto pixel = source.pixel(x, y);
            if (std::max({std::abs(qRed(pixel) - qRed(target)),
                          std::abs(qGreen(pixel) - qGreen(target)),
                          std::abs(qBlue(pixel) - qBlue(target)),
                          std::abs(qAlpha(pixel) - qAlpha(target))}) <= tolerance)
                queue.push_back(index);
        };
        visit(seed.x(), seed.y());
        QRect changed;
        for (size_t i = 0; i < queue.size(); ++i) {
            if ((i & 4095) == 0 && cancelled->load())
                return {};
            const int x = queue[i] % width, y = queue[i] / width;
            if (source.pixel(x, y) != replacement) {
                reinterpret_cast<QRgb*>(output.scanLine(y))[x] = replacement;
                changed = changed.united(QRect(x, y, 1, 1));
            }
            if (x > 0)
                visit(x - 1, y);
            if (x + 1 < width)
                visit(x + 1, y);
            if (y > 0)
                visit(x, y - 1);
            if (y + 1 < height)
                visit(x, y + 1);
        }
        if (cancelled->load() || changed.isEmpty())
            return {};
        return {output, changed, false};
    } catch (const std::bad_alloc&) {
        return {{}, {}, true};
    }
}
} // namespace openpaint
