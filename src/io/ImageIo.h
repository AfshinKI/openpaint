// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "PngIo.h"
#include <QColor>
namespace openpaint {
enum class ExportFormat { Png, Jpeg, Bmp, WebP, MonoBmp };
struct ExportOptions {
    ExportFormat format = ExportFormat::Png;
    int quality = 90;
    bool lossless = true;
    bool flatten = false;
    QColor background = Qt::white;
};
QByteArray exportCodec(ExportFormat format);
bool exportSuffix(const QString& path, ExportFormat format);
ImageResult exportImage(const QString& path, const QImage& image, const ExportOptions& options,
                        const Cancellation& cancellation = {},
                        const std::function<bool(QSaveFile&)>& commit = {},
                        const std::function<qint64(QIODevice&, const QByteArray&)>& write = {});
ImageResult loadImage(const QString& path, const Cancellation& cancellation = {});
} // namespace openpaint
