// SPDX-License-Identifier: GPL-3.0-or-later
#include "PngIo.h"
#include "document/Document.h"
#include <QBuffer>
#include <QColorSpace>
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QImageReader>
#include <QImageWriter>
#include <QSaveFile>
#include <QtEndian>
#include <cstring>

namespace openpaint {
namespace {
QString tr(const char* text) {
    return QCoreApplication::translate("PngIo", text);
}
bool cancelled(const Cancellation& token) {
    return token && token->load();
}
QImage cleanImage(QImage image) {
    if (image.colorSpace().isValid() && image.colorSpace() != QColorSpace(QColorSpace::SRgb))
        image.convertToColorSpace(QColorSpace::SRgb);
    image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QImage clean(image.size(), image.format());
    if (clean.isNull())
        return {};
    for (int y = 0; y < image.height(); ++y)
        std::memcpy(clean.scanLine(y), image.constScanLine(y), size_t(image.width()) * 4);
    clean.setColorSpace(QColorSpace::SRgb);
    return clean;
}
} // namespace
ImageResult loadPng(const QString& path, const Cancellation& cancellation) {
    if (cancelled(cancellation))
        return {{}, {}, true};
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly) || file.size() > 128 * 1024 * 1024)
        return {{}, tr("Cannot open this file, or it exceeds the 128 MiB PNG limit.")};
    const qint64 encodedSize = file.size();
    if (file.read(8) != QByteArray::fromHex("89504e470d0a1a0a"))
        return {{}, tr("Only PNG images are supported in this version.")};
    QByteArray filtered = QByteArray::fromHex("89504e470d0a1a0a");
    bool ended = false, removedMetadata = false;
    // Reject animations rather than silently replacing them with their first frame.
    while (file.pos() < encodedSize) {
        const auto header = file.read(8);
        if (header.size() != 8)
            return {{}, tr("The PNG is truncated.")};
        const auto length = qFromBigEndian<quint32>(header.constData());
        if (qint64(length) + 4 > encodedSize - file.pos())
            return {{}, tr("The PNG contains an invalid chunk.")};
        if (header.mid(4) == "acTL")
            return {{}, tr("Animated PNG is not supported. Open a static PNG instead.")};
        const auto type = header.mid(4);
        if (type == "IEND" && length != 0)
            return {{}, tr("The PNG contains an invalid end chunk.")};
        if (type == "eXIf")
            return {
                {},
                tr("PNG EXIF orientation is not supported yet. Export a normalized PNG first.")};
        if (type == "tEXt" || type == "iTXt" || type == "zTXt") {
            removedMetadata = true;
            file.seek(file.pos() + length + 4);
        } else {
            filtered += header;
            filtered += file.read(qint64(length) + 4);
        }
        if (type == "IEND") {
            ended = true;
            break;
        }
        if (cancelled(cancellation))
            return {{}, {}, true};
    }
    if (!ended)
        return {{}, tr("The PNG is truncated.")};
    QBuffer buffer(&filtered);
    buffer.open(QIODevice::ReadOnly);
    QImageReader reader(&buffer, "png");
    reader.setAutoTransform(true);
    if (!Document::validSize(reader.size()))
        return {{}, tr("Image exceeds the limit: 16,384 pixels per side and 32 million pixels.")};
    QImage image = reader.read();
    if (image.isNull())
        return {{}, tr("The PNG could not be decoded. The current drawing is unchanged.")};
    if (cancelled(cancellation))
        return {{}, {}, true};
    image = cleanImage(std::move(image));
    if (image.isNull())
        return {{}, tr("There is not enough memory to open this image.")};
    if (cancelled(cancellation))
        return {{}, {}, true};
    return {image,
            {},
            false,
            removedMetadata ? tr("Text metadata was removed for privacy. Saving will write pixels "
                                 "and the sRGB profile only.")
                            : QString()};
}
ImageResult savePng(const QString& path, const QImage& image, const Cancellation& cancellation,
                    const std::function<bool(QSaveFile&)>& commit) {
    if (cancelled(cancellation))
        return {{}, {}, true};
    if (!Document::validSize(image.size()) || image.isNull())
        return {{}, tr("The image has invalid dimensions.")};
    QSaveFile file(path);
    file.setDirectWriteFallback(false);
    if (!file.open(QIODevice::WriteOnly))
        return {{}, tr("Cannot create a safe temporary file here. Choose another destination.")};
    QImageWriter writer(&file, "png");
    const auto clean = cleanImage(image);
    if (clean.isNull() || !writer.write(clean)) {
        file.cancelWriting();
        return {{}, tr("PNG writing failed. The original file has been preserved.")};
    }
    if (cancelled(cancellation)) {
        file.cancelWriting();
        return {{}, {}, true};
    }
    if (!(commit ? commit(file) : file.commit()))
        return {{}, tr("File replacement failed. The original file has been preserved.")};
    return {};
}
} // namespace openpaint
