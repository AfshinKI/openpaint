// SPDX-License-Identifier: GPL-3.0-or-later
#include "Transform.h"
#include "document/Document.h"
#include <QCoreApplication>
#include <QTransform>
namespace openpaint {
TransformResult transformImage(const QImage& source, const QRegion& mask, TransformKind kind,
                               const std::shared_ptr<std::atomic_bool>& token) {
    if (!token || token->load())
        return {{}, {}, {}, true};
    if (source.isNull() || !Document::validSize(source.size()) ||
        source.format() != QImage::Format_ARGB32_Premultiplied ||
        (!mask.isEmpty() && !source.rect().contains(mask.boundingRect())))
        return {
            {}, {}, QCoreApplication::translate("Transform", "Invalid source image or selection.")};
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
