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
#include <QTransform>
#include <QtEndian>
#include <cstring>
#include <optional>
#include <zlib.h>

namespace openpaint {
namespace {
QString tr(const char* text) {
    return QCoreApplication::translate("PngIo", text);
}
bool cancelled(const Cancellation& token) {
    return token && token->load();
}
// Only IFD0 orientation affects the raster. Other EXIF data is never retained.
std::optional<int> exifOrientation(const QByteArray& data) {
    if (data.size() < 8 || (data.first(2) != "II" && data.first(2) != "MM"))
        return {};
    const bool little = data.first(2) == "II";
    auto u16 = [&](qsizetype p) {
        return little ? qFromLittleEndian<quint16>(data.constData() + p)
                      : qFromBigEndian<quint16>(data.constData() + p);
    };
    auto u32 = [&](qsizetype p) {
        return little ? qFromLittleEndian<quint32>(data.constData() + p)
                      : qFromBigEndian<quint32>(data.constData() + p);
    };
    if (u16(2) != 42)
        return {};
    const quint64 offset = u32(4);
    if (offset < 8 || offset + 2 > quint64(data.size()))
        return {};
    const quint64 count = u16(qsizetype(offset));
    if (offset + 2 + count * 12 + 4 > quint64(data.size()))
        return {};
    int orientation = 1;
    bool found = false;
    for (quint64 i = 0; i < count; ++i) {
        const auto p = qsizetype(offset + 2 + i * 12);
        if (u16(p) != 0x0112)
            continue;
        if (found || u16(p + 2) != 3 || u32(p + 4) != 1)
            return {};
        found = true;
        orientation = u16(p + 8);
        if (orientation < 1 || orientation > 8)
            return {};
    }
    return orientation;
}
QImage orientImage(QImage image, int orientation) {
    switch (orientation) {
    case 2:
        return image.mirrored(true, false);
    case 3:
        return image.mirrored(true, true);
    case 4:
        return image.mirrored(false, true);
    case 5:
        return image.mirrored(false, true).transformed(QTransform().rotate(90));
    case 6:
        return image.transformed(QTransform().rotate(90));
    case 7:
        return image.mirrored(true, false).transformed(QTransform().rotate(90));
    case 8:
        return image.transformed(QTransform().rotate(270));
    default:
        return image;
    }
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
static ImageResult readPng(QIODevice& file, const Cancellation& cancellation) {
    if (cancelled(cancellation))
        return {{}, {}, true};
    if (file.size() > 128 * 1024 * 1024)
        return {{}, tr("The image exceeds the 128 MiB PNG limit.")};
    const qint64 encodedSize = file.size();
    if (file.read(8) != QByteArray::fromHex("89504e470d0a1a0a"))
        return {{}, tr("Only PNG images are supported in this version.")};
    QByteArray filtered = QByteArray::fromHex("89504e470d0a1a0a");
    bool ended = false, removedMetadata = false, seenHeader = false, seenData = false;
    bool seenExif = false, dataEnded = false;
    int orientation = 1;
    // Reject animations rather than silently replacing them with their first frame.
    while (file.pos() < encodedSize) {
        const auto header = file.read(8);
        if (header.size() != 8)
            return {{}, tr("The PNG is truncated.")};
        const auto length = qFromBigEndian<quint32>(header.constData());
        if (qint64(length) + 4 > encodedSize - file.pos())
            return {{}, tr("The PNG contains an invalid chunk.")};
        const auto type = header.mid(4);
        const auto payload = file.read(qint64(length) + 4);
        if (payload.size() != qint64(length) + 4)
            return {{}, tr("The PNG is truncated.")};
        auto crc = crc32(0, reinterpret_cast<const Bytef*>(type.constData()), 4);
        crc = crc32(crc, reinterpret_cast<const Bytef*>(payload.constData()), length);
        if (crc != qFromBigEndian<quint32>(payload.constData() + length))
            return {{}, tr("The PNG contains a damaged chunk.")};
        if ((!seenHeader && type != "IHDR") || (type == "IHDR" && (seenHeader || length != 13)) ||
            (type == "IDAT" && dataEnded) || (type == "IEND" && (length != 0 || !seenData)))
            return {{}, tr("The PNG contains an invalid chunk sequence.")};
        if (type == "IHDR")
            seenHeader = true;
        if (type == "IDAT")
            seenData = true;
        else if (seenData)
            dataEnded = true;
        if (type == "acTL" || type == "fcTL" || type == "fdAT")
            return {{}, tr("Animated PNG is not supported. Open a static PNG instead.")};
        if (type == "iCCP") {
            const auto nameEnd = payload.indexOf('\0');
            if (nameEnd < 1 || nameEnd > 79 || nameEnd + 2 >= length || payload[nameEnd + 1] != 0)
                return {{}, tr("The PNG contains an invalid color profile.")};
            QByteArray profile(4 * 1024 * 1024, 0);
            uLongf size = profile.size();
            if (uncompress(reinterpret_cast<Bytef*>(profile.data()), &size,
                           reinterpret_cast<const Bytef*>(payload.constData() + nameEnd + 2),
                           length - nameEnd - 2) != Z_OK)
                return {{}, tr("The PNG color profile is invalid or exceeds 4 MiB.")};
            profile.resize(qsizetype(size));
            if (!QColorSpace::fromIccProfile(profile).isValid())
                return {{}, tr("The PNG color profile is unsupported or invalid.")};
        }
        if (type == "eXIf") {
            const auto parsed = exifOrientation(payload.first(length));
            if (seenExif || !parsed)
                return {{}, tr("The PNG contains invalid EXIF orientation metadata.")};
            seenExif = true;
            orientation = *parsed;
            removedMetadata = true;
        } else if (type == "tEXt" || type == "iTXt" || type == "zTXt") {
            removedMetadata = true;
        } else {
            filtered += header;
            filtered += payload;
        }
        if (type == "IEND") {
            ended = true;
            break;
        }
        if (cancelled(cancellation))
            return {{}, {}, true};
    }
    if (!ended || file.pos() != encodedSize)
        return {{}, tr("The PNG is truncated.")};
    QBuffer buffer(&filtered);
    buffer.open(QIODevice::ReadOnly);
    QImageReader reader(&buffer, "png");
    reader.setAutoTransform(false);
    if (!Document::validSize(reader.size()))
        return {{}, tr("Image exceeds the limit: 16,384 pixels per side and 32 million pixels.")};
    QImage image = reader.read();
    if (image.isNull())
        return {{}, tr("The PNG could not be decoded. The current drawing is unchanged.")};
    if (cancelled(cancellation))
        return {{}, {}, true};
    image = cleanImage(orientImage(std::move(image), orientation));
    if (image.isNull())
        return {{}, tr("There is not enough memory to open this image.")};
    if (cancelled(cancellation))
        return {{}, {}, true};
    return {image,
            {},
            false,
            removedMetadata ? tr("Metadata was removed for privacy. Saving will write pixels "
                                 "and the sRGB profile only.")
                            : QString()};
}
ImageResult loadPng(const QString& path, const Cancellation& cancellation) {
    if (cancelled(cancellation))
        return {{}, {}, true};
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return {{}, tr("Cannot open this PNG file.")};
    return readPng(file, cancellation);
}
ImageResult decodePng(const QByteArray& bytes, const Cancellation& cancellation) {
    QBuffer buffer;
    buffer.setData(bytes);
    buffer.open(QIODevice::ReadOnly);
    return readPng(buffer, cancellation);
}
bool writePng(QIODevice& device, const QImage& image) {
    // libpng uses longjmp on device errors; Qt 6.8's profile temporary then leaks.
    // Let the codec finish and unwind normally, but remember every failed write.
    class CheckedDevice : public QIODevice {
      public:
        explicit CheckedDevice(QIODevice& destination) : destination(destination) {
            open(WriteOnly);
        }
        bool failed = false;

      protected:
        qint64 readData(char*, qint64) override {
            return -1;
        }
        qint64 writeData(const char* data, qint64 size) override {
            if (!failed && destination.write(data, size) != size)
                failed = true;
            return size;
        }

      private:
        QIODevice& destination;
    } checked(device);
    QImageWriter writer(&checked, "png");
    const bool encoded = writer.write(image);
    return encoded && !checked.failed;
}
ImageResult savePng(const QString& path, const QImage& image, const Cancellation& cancellation,
                    const std::function<bool(QSaveFile&)>& commit,
                    const std::function<bool(QIODevice&, const QImage&)>& encode) {
    if (cancelled(cancellation))
        return {{}, {}, true};
    if (!Document::validSize(image.size()) || image.isNull())
        return {{}, tr("The image has invalid dimensions.")};
    QSaveFile file(path);
    file.setDirectWriteFallback(false);
    if (!file.open(QIODevice::WriteOnly))
        return {{}, tr("Cannot create a safe temporary file here. Choose another destination.")};
    const auto clean = cleanImage(image);
    if (clean.isNull() || !(encode ? encode(file, clean) : writePng(file, clean))) {
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
