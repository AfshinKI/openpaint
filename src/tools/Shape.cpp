// SPDX-License-Identifier: GPL-3.0-or-later
#include "Shape.h"
#include "Opacity.h"
#include <QPainter>
#include <QPainterPath>
#include <QTransform>
#include <cmath>
#include <numbers>
namespace openpaint {
ShapePatch renderShape(const QImage& base, QPointF start, QPointF end, int kind,
                       QColor outlineColor, QColor fillColor, int width, bool outline, bool fill,
                       qreal bend1, qreal bend2, const QList<QPointF>& vertices, int opacity) {
    if (opacity < 0 || opacity > 100 || base.isNull() || kind < 0 || kind >= ShapeCount ||
        width < 1 || width > 128 || !outlineColor.isValid() || !fillColor.isValid() ||
        !std::isfinite(bend1) || !std::isfinite(bend2) || std::abs(bend1) > 200 ||
        std::abs(bend2) > 200 || vertices.size() > 4096)
        return {};
    auto valid = [](QPointF p) {
        return std::isfinite(p.x()) && std::isfinite(p.y()) && std::abs(p.x()) <= 1000000 &&
               std::abs(p.y()) <= 1000000;
    };
    if (!valid(start) || !valid(end))
        return {};
    for (auto p : vertices)
        if (!valid(p))
            return {};
    const auto bounds = QRectF(start, end).normalized();
    QPainterPath path;
    bool closed = true;
    auto polygon = [&](const QList<QPointF>& points) {
        if (points.isEmpty())
            return;
        path.moveTo(points.first());
        for (qsizetype i = 1; i < points.size(); ++i)
            path.lineTo(points[i]);
        path.closeSubpath();
    };
    auto normalizedPolygon = [&](const QList<QPointF>& points) {
        QList<QPointF> mapped;
        for (auto p : points)
            mapped.append(
                {bounds.x() + p.x() * bounds.width(), bounds.y() + p.y() * bounds.height()});
        polygon(mapped);
    };
    auto radial = [&](int count, bool star) {
        QList<QPointF> points;
        const int total = star ? count * 2 : count;
        for (int i = 0; i < total; ++i) {
            const qreal angle = 2 * std::numbers::pi * i / total - std::numbers::pi / 2;
            const qreal radius = star && i % 2 ? 0.22 : 0.5;
            points.append({0.5 + radius * std::cos(angle), 0.5 + radius * std::sin(angle)});
        }
        normalizedPolygon(points);
    };
    switch (kind) {
    case 0:
        path.moveTo(start);
        path.lineTo(end);
        closed = false;
        break;
    case 1:
        path.addRect(bounds);
        break;
    case 2:
        path.addEllipse(bounds);
        break;
    case 3:
        path.addRoundedRect(bounds, 20, 20, Qt::RelativeSize);
        break;
    case 4:
        normalizedPolygon({{0.5, 0}, {1, 1}, {0, 1}});
        break;
    case 5:
        normalizedPolygon({{0.5, 0}, {1, 0.5}, {0.5, 1}, {0, 0.5}});
        break;
    case 6:
        radial(5, false);
        break;
    case 7:
        radial(6, false);
        break;
    case 8:
    case 9:
    case 10:
    case 11: {
        QList<QPointF> points{{0, 0.3}, {0.6, 0.3}, {0.6, 0}, {1, 0.5},
                              {0.6, 1}, {0.6, 0.7}, {0, 0.7}};
        for (auto& p : points) {
            if (kind == 9)
                p.setX(1 - p.x());
            else if (kind == 10)
                p = {p.y(), 1 - p.x()};
            else if (kind == 11)
                p = {1 - p.y(), p.x()};
        }
        normalizedPolygon(points);
        break;
    }
    case 12:
        radial(5, true);
        break;
    case 13:
        normalizedPolygon(
            {{0, 0}, {1, 0}, {1, 0.75}, {0.5, 0.75}, {0.25, 1}, {0.3, 0.75}, {0, 0.75}});
        break;
    case CurveShape: {
        const auto delta = end - start;
        const QPointF normal(-delta.y(), delta.x());
        path.moveTo(start);
        path.cubicTo(start + delta / 3 + normal * (bend1 / 100),
                     start + delta * (2.0 / 3) + normal * (bend2 / 100), end);
        closed = false;
        break;
    }
    case SplineShape: {
        QList<QPointF> points;
        for (const auto point : vertices)
            if (points.isEmpty() || point != points.last())
                points.append(point);
        if (points.size() < 2)
            return {};
        // Uniform Catmull–Rom interpolation expressed as cubic Bezier segments.
        // Repeated endpoints provide a natural end tangent; the path stays open.
        path.moveTo(points.first());
        for (qsizetype i = 0; i + 1 < points.size(); ++i) {
            const auto p0 = points[qMax(qsizetype(0), i - 1)];
            const auto p1 = points[i];
            const auto p2 = points[i + 1];
            const auto p3 = points[qMin(points.size() - 1, i + 2)];
            path.cubicTo(p1 + (p2 - p0) / 6, p2 - (p3 - p1) / 6, p2);
        }
        closed = false;
        break;
    }
    case PolygonShape:
        if (vertices.size() < 2)
            return {};
        polygon(vertices);
        break;
    case 16:
        radial(4, true);
        break;
    case 17:
        radial(6, true);
        break;
    case 18: {
        QPainterPath oval, tail;
        oval.addEllipse(QRectF(bounds.x(), bounds.y(), bounds.width(), bounds.height() * 0.8));
        tail.moveTo(bounds.x() + bounds.width() * 0.3, bounds.y() + bounds.height() * 0.65);
        tail.lineTo(bounds.x() + bounds.width() * 0.2, bounds.bottom());
        tail.lineTo(bounds.x() + bounds.width() * 0.6, bounds.y() + bounds.height() * 0.65);
        tail.closeSubpath();
        path = oval.united(tail);
        break;
    }
    }
    const qreal margin = width / 2.0 + 2;
    const auto rect = path.boundingRect()
                          .adjusted(-margin, -margin, margin, margin)
                          .toAlignedRect()
                          .intersected(base.rect());
    if (rect.isEmpty())
        return {};
    QImage image = base.copy(rect);
    QPainter painter(&image);
    painter.translate(-rect.topLeft());
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(outline ? QPen(outlineColor, width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)
                           : QPen(Qt::NoPen));
    painter.setBrush(fill && closed ? QBrush(fillColor) : Qt::NoBrush);
    painter.drawPath(path);
    painter.end();
    if (opacity < 100) {
        for (int y = 0; y < image.height(); ++y) {
            auto* row = reinterpret_cast<QRgb*>(image.scanLine(y));
            const auto* before = reinterpret_cast<const QRgb*>(base.constScanLine(y + rect.y()));
            for (int x = 0; x < image.width(); ++x)
                row[x] = operationOpacity(before[x + rect.x()], row[x], opacity);
        }
    }
    return {image, rect};
}
} // namespace openpaint
