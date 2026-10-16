// SPDX-License-Identifier: GPL-3.0-or-later
#include "Fill.h"
#include "document/Document.h"
#include <algorithm>
#include <cmath>
#include <vector>
namespace openpaint {
namespace {
// Recognize a coverage blend of the seed background and the replacement ink.
// Only a one-pixel fringe is painted: these pixels never expand the flood.
bool coverageBlend(QRgb pixel, QRgb background, QRgb ink) {
    const int p[] = {qRed(pixel), qGreen(pixel), qBlue(pixel), qAlpha(pixel)};
    const int b[] = {qRed(background), qGreen(background), qBlue(background), qAlpha(background)};
    const int f[] = {qRed(ink), qGreen(ink), qBlue(ink), qAlpha(ink)};
    int axis = 0;
    for (int i = 1; i < 4; ++i)
        if (std::abs(f[i] - b[i]) > std::abs(f[axis] - b[axis]))
            axis = i;
    const int delta = f[axis] - b[axis], step = p[axis] - b[axis];
    if (!delta || step * delta <= 0 || std::abs(step) >= std::abs(delta))
        return false;
    for (int i = 0; i < 4; ++i)
        if (std::abs((p[i] - b[i]) * delta - (f[i] - b[i]) * step) > std::abs(delta))
            return false;
    return true;
}
} // namespace

FillResult fillRegion(const QImage& source, QPoint seed, QColor color, int tolerance,
                      const std::shared_ptr<std::atomic_bool>& cancelled, const QRegion& selection,
                      bool smoothEdges) {
    if (!Document::validSize(source.size()) || !source.rect().contains(seed) ||
        source.format() != QImage::Format_ARGB32_Premultiplied || !color.isValid() ||
        tolerance < 0 || tolerance > 255 || !cancelled)
        return {{}, {}, true};
    if (cancelled->load() || (!selection.isEmpty() && !selection.contains(seed)))
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
        QRect changed;
        auto visit = [&](int x, int y) {
            const int index = y * width + x;
            if (seen[index])
                return;
            seen[index] = 1;
            if (!selection.isEmpty() && !selection.contains(QPoint(x, y)))
                return;
            const auto pixel = source.pixel(x, y);
            if (std::max({std::abs(qRed(pixel) - qRed(target)),
                          std::abs(qGreen(pixel) - qGreen(target)),
                          std::abs(qBlue(pixel) - qBlue(target)),
                          std::abs(qAlpha(pixel) - qAlpha(target))}) <= tolerance)
                queue.push_back(index);
            else if (smoothEdges && coverageBlend(pixel, target, replacement)) {
                reinterpret_cast<QRgb*>(output.scanLine(y))[x] = replacement;
                changed = changed.united(QRect(x, y, 1, 1));
            }
        };
        visit(seed.x(), seed.y());
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
            if (smoothEdges) {
                for (int dy : {-1, 1})
                    for (int dx : {-1, 1}) {
                        const QPoint neighbor(x + dx, y + dy);
                        if (!source.rect().contains(neighbor) ||
                            (!selection.isEmpty() && !selection.contains(neighbor)))
                            continue;
                        if (coverageBlend(source.pixel(neighbor), target, replacement)) {
                            reinterpret_cast<QRgb*>(output.scanLine(neighbor.y()))[neighbor.x()] =
                                replacement;
                            changed = changed.united(QRect(neighbor, QSize(1, 1)));
                        }
                    }
            }
        }
        if (cancelled->load() || changed.isEmpty())
            return {};
        return {output, changed, false};
    } catch (const std::bad_alloc&) {
        return {{}, {}, true};
    }
}
} // namespace openpaint
