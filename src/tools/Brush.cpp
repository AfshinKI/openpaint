// SPDX-License-Identifier: GPL-3.0-or-later
#include "Brush.h"
#include "document/Document.h"
#include <algorithm>
#include <cmath>
namespace openpaint {
bool Brush::begin(QSize size, QPointF point, QColor color, qreal width, int style, int opacity,
                  const QRegion& selection) {
    if (!std::isfinite(point.x()) || !std::isfinite(point.y()) || std::abs(point.x()) > 1000000 ||
        std::abs(point.y()) > 1000000 || !Document::validSize(size) || !color.isValid() ||
        style < 0 || style > 8 || opacity < 0 || opacity > 100 || !std::isfinite(width) ||
        width <= 0 || width > 128)
        return false;
    coverage_ = QImage(size, QImage::Format_Alpha8);
    if (coverage_.isNull())
        return false;
    coverage_.fill(0);
    selection_ = selection;
    previous_ = point;
    color_ = color;
    width_ = width;
    style_ = style;
    opacity_ = opacity;
    carry_ = 0;
    first_ = true;
    return true;
}
QRect Brush::dab(QImage& image, const QImage& original, QPointF point) {
    point = {std::round(point.x() * 1000000) / 1000000, std::round(point.y() * 1000000) / 1000000};
    const qreal radius = width_ / 2;
    const QRect bounds =
        QRectF(point.x() - radius - 1, point.y() - radius - 1, width_ + 2, width_ + 2)
            .toAlignedRect()
            .intersected(image.rect());
    for (int y = bounds.top(); y <= bounds.bottom(); ++y) {
        auto* mask = coverage_.scanLine(y);
        auto* output = reinterpret_cast<QRgb*>(image.scanLine(y));
        const auto* base = reinterpret_cast<const QRgb*>(original.constScanLine(y));
        for (int x = bounds.left(); x <= bounds.right(); ++x) {
            if (!selection_.isEmpty() && !selection_.contains(QPoint(x, y)))
                continue;
            const qreal dx = (x + 0.5 - point.x()) / radius, dy = (y + 0.5 - point.y()) / radius;
            qreal distance = std::hypot(dx, dy);
            const qreal along = (dx + dy) * 0.7071067811865476;
            const qreal across = (dx - dy) * 0.7071067811865476;
            if (style_ == 1 || style_ == 2)
                distance = std::hypot(along, across * (style_ == 1 ? 3 : 6));
            if (style_ == 6)
                distance = std::max(std::abs(dx), std::abs(dy));
            qreal strength = std::clamp((1 - distance) * radius + 0.5, 0.0, 1.0);
            quint32 hash = quint32(x) * 374761393u + quint32(y) * 668265263u;
            hash = (hash ^ (hash >> 13)) * 1274126177u;
            const qreal grain = qreal((hash ^ (hash >> 16)) & 255) / 255;
            switch (style_) {
            case 3:
                strength *= std::max(0.0, 1 - distance * distance);
                break;
            case 4:
                strength *= 0.35 + 0.65 * (0.5 + 0.5 * std::sin(y * 2.3));
                break;
            case 5:
                strength *= grain > 0.25 ? 0.4 + grain * 0.6 : 0;
                break;
            case 7:
                strength *= std::max(0.0, 1 - distance) * (0.3 + grain * 0.7);
                break;
            case 8:
                strength *= 0.35 * std::max(0.0, 1 - distance * distance * distance);
                break;
            default:
                break;
            }
            const int coverage = qRound(strength * 255);
            if (coverage <= mask[x])
                continue;
            mask[x] = uchar(coverage);
            const int alpha = qRound(coverage * color_.alphaF() * opacity_ / 100.0);
            const int inverse = 255 - alpha;
            auto blend = [alpha, inverse](int foreground, int background) {
                return (foreground * alpha + background * inverse + 127) / 255;
            };
            output[x] =
                qRgba(blend(color_.red(), qRed(base[x])), blend(color_.green(), qGreen(base[x])),
                      blend(color_.blue(), qBlue(base[x])),
                      alpha + (qAlpha(base[x]) * inverse + 127) / 255);
        }
    }
    return bounds;
}
QRect Brush::extend(QImage& image, const QImage& original, QPointF point) {
    if (coverage_.isNull() || !std::isfinite(point.x()) || !std::isfinite(point.y()))
        return {};
    QRect dirty;
    if (first_) {
        dirty = dab(image, original, previous_);
        first_ = false;
    }
    const QPointF delta = point - previous_;
    const qreal length = std::hypot(delta.x(), delta.y());
    const qreal spacing = std::max(0.5, width_ * 0.15);
    if (length > 0) {
        // Liang-Barsky clipping bounds stamp work even for a distant pointer.
        qreal enter = 0, leave = 1;
        const qreal margin = width_ / 2 + 1;
        auto clip = [&](qreal p, qreal q) {
            if (p == 0)
                return q >= 0;
            const qreal r = q / p;
            if (p < 0)
                enter = std::max(enter, r);
            else
                leave = std::min(leave, r);
            return enter <= leave;
        };
        if (clip(-delta.x(), previous_.x() + margin) &&
            clip(delta.x(), image.width() + margin - previous_.x()) &&
            clip(-delta.y(), previous_.y() + margin) &&
            clip(delta.y(), image.height() + margin - previous_.y())) {
            const qreal first = spacing - carry_;
            const auto start = std::max(0.0, std::ceil((enter * length - first) / spacing));
            for (qreal distance = first + start * spacing; distance <= leave * length + 1e-8;
                 distance += spacing)
                dirty = dirty.united(dab(image, original, previous_ + delta * (distance / length)));
        }
        carry_ = std::fmod(carry_ + length, spacing);
    }
    previous_ = point;
    return dirty;
}
QImage Brush::preview(int style, QColor color) {
    QImage image(100, 28, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    const QImage original = image;
    Brush brush;
    if (!brush.begin(image.size(), {8, 18}, color, 14, style, 100))
        return image;
    for (int x = 8; x < 93; ++x)
        brush.extend(image, original, {qreal(x), 14 + 5 * std::sin(x * 0.08)});
    return image;
}
} // namespace openpaint
