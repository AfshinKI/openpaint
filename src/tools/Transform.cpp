// SPDX-License-Identifier: GPL-3.0-or-later
#include "Transform.h"
#include "document/Document.h"
#include <QCoreApplication>
#include <QTransform>
#include <cmath>
#include <cstring>
#include <numbers>
namespace openpaint {
TransformResult transformImage(const QImage& source, const QRegion& mask, TransformKind kind,
                               const std::shared_ptr<std::atomic_bool>& token,
                               const TransformOptions& options) {
    if (!token || token->load())
        return {{}, {}, {}, true};
    if (source.isNull() || !Document::validSize(source.size()) ||
        source.format() != QImage::Format_ARGB32_Premultiplied ||
        (!mask.isEmpty() && !source.rect().contains(mask.boundingRect())))
        return {
            {}, {}, QCoreApplication::translate("Transform", "Invalid source image or selection.")};
    if (kind == TransformKind::ResizeSkew) {
        if (!Document::validSize(options.size) || !std::isfinite(options.skewX) ||
            !std::isfinite(options.skewY) || std::abs(options.skewX) > 89 ||
            std::abs(options.skewY) > 89)
            return {{},
                    {},
                    QCoreApplication::translate(
                        "Transform",
                        "Choose valid dimensions and skew angles from -89 to 89 degrees.")};
        QTransform shear;
        shear.shear(std::tan(options.skewX * std::numbers::pi / 180),
                    std::tan(options.skewY * std::numbers::pi / 180));
        const auto bounds = shear.mapRect(QRectF(QPointF(), QSizeF(options.size))).toAlignedRect();
        if (std::abs(shear.determinant()) < 0.000001 || !Document::validSize(bounds.size()))
            return {{},
                    {},
                    QCoreApplication::translate(
                        "Transform",
                        "The resulting image is singular or exceeds the image size limits.")};
        const auto mode = options.smooth ? Qt::SmoothTransformation : Qt::FastTransformation;
        const auto resized = source.scaled(options.size, Qt::IgnoreAspectRatio, mode);
        if (token->load())
            return {{}, {}, {}, true};
        auto image = resized.transformed(shear, mode);
        if (token->load())
            return {{}, {}, {}, true};
        if (image.isNull() || !Document::validSize(image.size()))
            return {{},
                    {},
                    QCoreApplication::translate(
                        "Transform",
                        "The resized image could not be allocated within the size limits.")};
        QRegion resultMask;
        if (!mask.isEmpty()) {
            QImage coverage(source.size(), QImage::Format_Alpha8);
            if (coverage.isNull())
                return {{},
                        {},
                        QCoreApplication::translate(
                            "Transform", "There is not enough memory to transform the selection.")};
            coverage.fill(0);
            for (const auto rect : mask)
                for (int y = rect.top(); y <= rect.bottom(); ++y)
                    std::memset(coverage.scanLine(y) + rect.left(), 255, size_t(rect.width()));
            const auto transformedCoverage =
                coverage.scaled(options.size, Qt::IgnoreAspectRatio, mode).transformed(shear, mode);
            if (transformedCoverage.isNull() || transformedCoverage.size() != image.size())
                return {{},
                        {},
                        QCoreApplication::translate("Transform",
                                                    "The selection could not be transformed.")};
            for (int y = 0; y < image.height(); ++y) {
                if (token->load())
                    return {{}, {}, {}, true};
                for (int x = 0; x < image.width();) {
                    if (!qAlpha(transformedCoverage.pixel(x, y))) {
                        ++x;
                        continue;
                    }
                    const int start = x++;
                    while (x < image.width() && qAlpha(transformedCoverage.pixel(x, y)))
                        ++x;
                    resultMask += QRect(start, y, x - start, 1);
                }
                if (resultMask.rectCount() > 16384)
                    return {
                        {},
                        {},
                        QCoreApplication::translate(
                            "Transform",
                            "The transformed selection is too complex. Try a simpler selection.")};
            }
            // Keep a manipulable box even if a subpixel selection becomes fully transparent.
            if (resultMask.isEmpty())
                resultMask = image.rect();
        }
        return {image, resultMask, {}, false};
    }
    QTransform transform;
    switch (kind) {
    case TransformKind::RotateRight:
        transform.rotate(90);
        break;
    case TransformKind::RotateLeft:
        transform.rotate(-90);
        break;
    case TransformKind::RotateHalf:
        transform.rotate(180);
        break;
    case TransformKind::FlipHorizontal:
        transform.scale(-1, 1);
        break;
    case TransformKind::FlipVertical:
        transform.scale(1, -1);
        break;
    case TransformKind::Crop:
        break;
    default:
        return {{}, {}, QCoreApplication::translate("Transform", "Unknown image transform.")};
    }
    const auto matrix = QImage::trueMatrix(transform, source.width(), source.height());
    auto image = source.transformed(transform, Qt::FastTransformation);
    if (token->load())
        return {{}, {}, {}, true};
    if (image.isNull())
        return {{},
                {},
                QCoreApplication::translate("Transform",
                                            "There is not enough memory to transform the image.")};
    return {image, matrix.map(mask), {}, false};
}
} // namespace openpaint
