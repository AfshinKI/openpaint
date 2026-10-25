// SPDX-License-Identifier: GPL-3.0-or-later
#include "ImageIo.h"
#include "document/Document.h"
#include <QBuffer>
#include <QColorSpace>
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QImageReader>
#include <QImageWriter>
#include <QPainter>
#include <QSaveFile>
#include <QtEndian>

namespace openpaint {
namespace {
QString tr(const char* text) {
    return QCoreApplication::translate("ImageIo", text);
}
bool validWebp(const QByteArray& bytes, int& orientation) {
    bool seenExif = false;
    if (bytes.size() < 20 || qFromLittleEndian<quint32>(bytes.constData() + 4) != bytes.size() - 8)
        return false;
    for (qsizetype p = 12; p < bytes.size();) {
        if (bytes.size() - p < 8)
            return false;
        const auto kind = bytes.mid(p, 4);
        const quint64 size = qFromLittleEndian<quint32>(bytes.constData() + p + 4);
        if (size + (size & 1) > quint64(bytes.size() - p - 8))
            return false;
        if (kind == "ANIM" || kind == "ANMF" ||
            (kind == "VP8X" && (size != 10 || (quint8(bytes[p + 8]) & 2))))
            return false;
        if (kind == "ICCP" &&
            (size > 4 * 1024 * 1024 ||
             !QColorSpace::fromIccProfile(bytes.mid(p + 8, qsizetype(size))).isValid()))
            return false;
        if (kind == "EXIF") {
            auto exif = bytes.mid(p + 8, qsizetype(size));
            if (exif.startsWith(QByteArray("Exif\0\0", 6)))
                exif.remove(0, 6);
            const auto parsed = exifOrientation(exif);
            if (seenExif || !parsed)
                return false;
            seenExif = true;
            orientation = *parsed;
        }
        p += qsizetype(8 + size + (size & 1));
    }
    return true;
}
bool validJpeg(const QByteArray& bytes) {
    if (!bytes.endsWith(QByteArray::fromHex("ffd9")))
        return false;
    // Bound segments and collect split ICC data in their declared order.
    QList<QByteArray> profiles;
    int count = 0;
    qsizetype profileSize = 0;
    qsizetype p = 2;
    while (p < bytes.size()) {
        if (quint8(bytes[p++]) != 0xff)
            return false;
        while (p < bytes.size() && quint8(bytes[p]) == 0xff)
            ++p;
        if (p >= bytes.size())
            return false;
        const auto marker = quint8(bytes[p++]);
        if (marker == 0xda) {
            if (!profiles.isEmpty()) {
                QByteArray profile;
                for (const auto& part : profiles) {
                    if (part.isEmpty())
                        return false;
                    profile += part;
                }
                if (!QColorSpace::fromIccProfile(profile).isValid())
                    return false;
            }
            return true; // Qt validates entropy-coded scan data.
        }
        if (p + 2 > bytes.size())
            return false;
        const auto length = qFromBigEndian<quint16>(bytes.constData() + p);
        if (length < 2 || length > bytes.size() - p)
            return false;
        if (marker == 0xe2 && bytes.mid(p + 2, 12) == QByteArray("ICC_PROFILE\0", 12)) {
            if (length < 17)
                return false;
            const int index = quint8(bytes[p + 14]), parts = quint8(bytes[p + 15]);
            if (!parts || !index || index > parts || (count && count != parts))
                return false;
            if (!count) {
                count = parts;
                profiles.resize(count);
            }
            if (!profiles[index - 1].isEmpty())
                return false;
            profileSize += length - 16;
            if (profileSize > 4 * 1024 * 1024)
                return false;
            profiles[index - 1] = bytes.mid(p + 16, length - 16);
        }
        p += length;
    }
    return false;
}
} // namespace
ImageResult loadImage(const QString& path, const Cancellation& token) {
    if (token && token->load())
        return {{}, {}, true};
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return {{}, tr("Cannot open this image file.")};
    if (file.size() > 128 * 1024 * 1024)
        return {{}, tr("The image exceeds the 128 MiB encoded limit.")};
    auto bytes = file.readAll();
    if (bytes.size() != file.size())
        return {{}, tr("The image could not be read completely.")};
    if (bytes.startsWith(QByteArray::fromHex("89504e470d0a1a0a"))) {
        auto result = decodePng(bytes, token);
        result.format = "png";
        return result;
    }
    QByteArray format;
    int orientation = 1;
    if (bytes.startsWith(QByteArray::fromHex("ffd8"))) {
        if (!validJpeg(bytes))
            return {{}, tr("The JPEG container or color profile is invalid.")};
        format = "jpeg";
    } else if (bytes.startsWith("RIFF") && bytes.mid(8, 4) == "WEBP") {
        if (!validWebp(bytes, orientation))
            return {{}, tr("The WebP is damaged, animated, or has invalid metadata.")};
        // Qt 6.8 probes sizeof(WebPBitstreamFeatures) bytes even for smaller valid files.
        // Add a legal ignored RIFF chunk to the private decode buffer only.
        if (bytes.size() < 64) {
            bytes += QByteArray("JUNK", 4) + QByteArray::fromHex("40000000") + QByteArray(64, 0);
            qToLittleEndian<quint32>(quint32(bytes.size() - 8), bytes.data() + 4);
        }
        format = "webp";
    } else if (bytes.startsWith("BM")) {
        if (bytes.size() < 26 ||
            qFromLittleEndian<quint32>(bytes.constData() + 2) != bytes.size() ||
            qFromLittleEndian<quint32>(bytes.constData() + 10) >= bytes.size())
            return {{}, tr("The BMP container is invalid.")};
        // Qt does not normalize calibrated/embedded/linked BMP color profiles.
        if (qFromLittleEndian<quint32>(bytes.constData() + 14) >= 108)
            return {{}, tr("BMP V4/V5 color headers are not supported. Convert to PNG first.")};
        format = "bmp";
    } else {
        return {{}, tr("Choose a static PNG, JPEG, BMP or WebP image.")};
    }
    if (token && token->load())
        return {{}, {}, true};
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::ReadOnly);
    QImageReader reader(&buffer, format);
    reader.setDecideFormatFromContent(false);
    reader.setAutoTransform(true);
    if (!Document::validSize(reader.size()))
        return {{}, tr("Image exceeds the limit: 16,384 pixels per side and 32 million pixels.")};
    auto image = reader.read();
    if (image.isNull() || !Document::validSize(image.size()))
        return {{}, tr("The image could not be decoded. The current drawing is unchanged.")};
    if (token && token->load())
        return {{}, {}, true};
    image = cleanImage(orientImage(std::move(image), orientation));
    if (image.isNull())
        return {{}, tr("There is not enough memory to open this image.")};
    if (token && token->load())
        return {{}, {}, true};
    return {image,
            {},
            false,
            tr("Imported image. Save creates a PNG; the original stays unchanged. Source metadata "
               "was removed and colors were normalized to sRGB."),
            format};
}
QByteArray exportCodec(ExportFormat format) {
    switch (format) {
    case ExportFormat::Png:
        return "png";
    case ExportFormat::Jpeg:
        return "jpeg";
    case ExportFormat::Bmp:
    case ExportFormat::MonoBmp:
        return "bmp";
    case ExportFormat::WebP:
        return "webp";
    }
    return {};
}
bool exportSuffix(const QString& path, ExportFormat format) {
    const auto suffix = QFileInfo(path).suffix().toLower();
    return suffix == QString::fromLatin1(exportCodec(format)) ||
           (format == ExportFormat::Jpeg && suffix == "jpg");
}
ImageResult exportImage(const QString& path, const QImage& image, const ExportOptions& options,
                        const Cancellation& token, const std::function<bool(QSaveFile&)>& commit,
                        const std::function<qint64(QIODevice&, const QByteArray&)>& write) {
    if (token && token->load())
        return {{}, {}, true};
    const auto codec = exportCodec(options.format);
    if (codec.isEmpty() || !exportSuffix(path, options.format) || image.isNull() ||
        !Document::validSize(image.size()) || options.quality < 1 || options.quality > 99 ||
        (options.flatten && (!options.background.isValid() || options.background.alpha() != 255)) ||
        (!options.flatten &&
         (options.format == ExportFormat::Jpeg || options.format == ExportFormat::Bmp ||
          options.format == ExportFormat::MonoBmp)))
        return {{}, tr("Choose valid export options and a matching filename extension.")};
    auto raster = cleanImage(image);
    if (raster.isNull())
        return {{}, tr("There is not enough memory to export this image.")};
    if (options.flatten) {
        QImage opaque(raster.size(), QImage::Format_RGB32);
        if (opaque.isNull())
            return {{}, tr("There is not enough memory to flatten this image.")};
        opaque.fill(options.background);
        QPainter painter(&opaque);
        painter.drawImage(0, 0, raster);
        painter.end();
        opaque.setColorSpace(QColorSpace::SRgb);
        raster = opaque;
    }
    if (options.format == ExportFormat::MonoBmp) {
        QImage mono(raster.size(), QImage::Format_Mono);
        if (mono.isNull())
            return {{}, tr("There is not enough memory to convert this image.")};
        mono.setColorTable({qRgb(0, 0, 0), qRgb(255, 255, 255)});
        mono.fill(0);
        for (int y = 0; y < raster.height(); ++y) {
            if (token && token->load())
                return {{}, {}, true};
            for (int x = 0; x < raster.width(); ++x)
                mono.setPixel(x, y, qGray(raster.pixel(x, y)) >= 128 ? 1 : 0);
        }
        raster = mono;
    }
    // Encode to memory so a failing destination never interrupts codec cleanup.
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    QImageWriter writer(&buffer, codec);
    writer.setQuality(options.format == ExportFormat::WebP && options.lossless ? 100
                                                                               : options.quality);
    if (!writer.write(raster) || bytes.size() > 128 * 1024 * 1024)
        return {{}, tr("Export encoding failed or exceeds the 128 MiB file limit.")};
    if (token && token->load())
        return {{}, {}, true};
    QSaveFile file(path);
    file.setDirectWriteFallback(false);
    if (!file.open(QIODevice::WriteOnly))
        return {{}, tr("Cannot create a safe temporary file here. Choose another destination.")};
    if ((write ? write(file, bytes) : file.write(bytes)) != bytes.size()) {
        file.cancelWriting();
        return {{}, tr("Export writing failed. The original file has been preserved.")};
    }
    if (token && token->load()) {
        file.cancelWriting();
        return {{}, {}, true};
    }
    if (!(commit ? commit(file) : file.commit()))
        return {{}, tr("File replacement failed. The original file has been preserved.")};
    return {};
}
} // namespace openpaint
