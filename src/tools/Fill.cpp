// SPDX-License-Identifier: GPL-3.0-or-later
#include "Fill.h"
#include "Opacity.h"
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
// Recover a local ink/background mixture without expanding the flood region.
// Raster pixels do not retain coverage metadata; only accept an edge whose RGBA
// values lie on the line between the seed background and a nearby stronger ink.
QRgb recolorEdge(const QImage& source, QPoint point, QRgb background, QRgb replacement) {
    const QRgb pixel = source.pixel(point);
    if (pixel == background || pixel == replacement)
        return pixel;
    if (coverageBlend(pixel, background, replacement))
        return replacement; // Retain same-color fringe cleanup.
    QRgb ink = pixel;
    int strongest = 0;
    for (int dy = -2; dy <= 2; ++dy) {
        for (int dx = -2; dx <= 2; ++dx) {
            const QPoint neighbor = point + QPoint(dx, dy);
            if (!source.rect().contains(neighbor))
                continue;
            const QRgb candidate = source.pixel(neighbor);
            const int contrast = std::max({std::abs(qRed(candidate) - qRed(background)),
                                           std::abs(qGreen(candidate) - qGreen(background)),
                                           std::abs(qBlue(candidate) - qBlue(background)),
                                           std::abs(qAlpha(candidate) - qAlpha(background))});
            if (contrast > strongest && coverageBlend(pixel, background, candidate)) {
                ink = candidate;
                strongest = contrast;
            }
        }
    }
    if (!strongest)
        return pixel;
    const int p[] = {qRed(pixel), qGreen(pixel), qBlue(pixel), qAlpha(pixel)};
    const int b[] = {qRed(background), qGreen(background), qBlue(background), qAlpha(background)};
    const int f[] = {qRed(ink), qGreen(ink), qBlue(ink), qAlpha(ink)};
    const int r[] = {qRed(replacement), qGreen(replacement), qBlue(replacement),
                     qAlpha(replacement)};
    int axis = 0;
    for (int i = 1; i < 4; ++i)
        if (std::abs(f[i] - b[i]) > std::abs(f[axis] - b[axis]))
            axis = i;
    const double coverage = double(p[axis] - b[axis]) / (f[axis] - b[axis]);
    int result[4];
    for (int i = 0; i < 4; ++i)
        result[i] = std::clamp(qRound(f[i] * coverage + r[i] * (1 - coverage)), 0, 255);
    return qRgba(std::min(result[0], result[3]), std::min(result[1], result[3]),
                 std::min(result[2], result[3]), result[3]);
}
} // namespace

FillResult fillRegion(const QImage& source, QPoint seed, QColor color, int tolerance,
                      const std::shared_ptr<std::atomic_bool>& cancelled, const QRegion& selection,
                      bool smoothEdges, int opacity, bool allMatching) {
    if (!Document::validSize(source.size()) || !source.rect().contains(seed) ||
        source.format() != QImage::Format_ARGB32_Premultiplied || !color.isValid() ||
        tolerance < 0 || tolerance > 255 || opacity < 0 || opacity > 100 || !cancelled)
        return {{}, {}, true};
    if (opacity == 0 || cancelled->load() || (!selection.isEmpty() && !selection.contains(seed)))
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
        auto fringe = [&](QPoint point) {
            if (!source.rect().contains(point) ||
                (!selection.isEmpty() && !selection.contains(point)))
                return;
            const auto pixel = recolorEdge(source, point, target, replacement);
            if (pixel != source.pixel(point)) {
                reinterpret_cast<QRgb*>(output.scanLine(point.y()))[point.x()] = pixel;
                changed = changed.united(QRect(point, QSize(1, 1)));
            }
        };
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
            else if (smoothEdges)
                fringe({x, y});
        };
        if (allMatching) {
            // Seed every matching island, then reuse the bounded edge treatment.
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    if ((x & 4095) == 0 && cancelled->load())
                        return {};
                    const auto pixel = source.pixel(x, y);
                    if (std::max({std::abs(qRed(pixel) - qRed(target)),
                                  std::abs(qGreen(pixel) - qGreen(target)),
                                  std::abs(qBlue(pixel) - qBlue(target)),
                                  std::abs(qAlpha(pixel) - qAlpha(target))}) <= tolerance)
                        visit(x, y);
                }
            }
        } else {
            visit(seed.x(), seed.y());
        }
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
                        fringe({x + dx, y + dy});
                    }
            }
        }
        if (cancelled->load() || changed.isEmpty())
            return {};
        if (opacity < 100) {
            for (int y = changed.top(); y <= changed.bottom(); ++y) {
                if (cancelled->load())
                    return {};
                auto* row = reinterpret_cast<QRgb*>(output.scanLine(y));
                const auto* before = reinterpret_cast<const QRgb*>(source.constScanLine(y));
                for (int x = changed.left(); x <= changed.right(); ++x)
                    row[x] = operationOpacity(before[x], row[x], opacity);
            }
        }
        return {output, changed, false};
    } catch (const std::bad_alloc&) {
        return {{}, {}, true};
    }
}
} // namespace openpaint
