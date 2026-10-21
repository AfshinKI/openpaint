// SPDX-License-Identifier: GPL-3.0-or-later
#include "OraIo.h"
#include <QBuffer>
#include <QColorSpace>
#include <QCoreApplication>
#include <QFile>
#include <QMap>
#include <QPainter>
#include <QSaveFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <archive.h>
#include <archive_entry.h>
#include <cmath>
#include <memory>
namespace openpaint {
namespace {
constexpr qint64 archiveLimit = 128 * 1024 * 1024;
bool cancelled(const Cancellation& token) {
    return token && token->load();
}
ImageResult error() {
    return {{},
            QCoreApplication::translate(
                "OraIo", "The project is invalid, unsupported, or exceeds the layer limits.")};
}
bool safePath(const QString& path) {
    if (path.isEmpty() || path.size() > 240 || path.startsWith('/') || path.contains('\\') ||
        path.contains(':'))
        return false;
    const auto parts = path.split('/');
    return !parts.contains("..") && !parts.contains(".") && !parts.contains("");
}
QByteArray png(const QImage& image) {
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    if (!writePng(buffer, image))
        return {};
    return data;
}
struct Output {
    QSaveFile* file;
    Cancellation token;
    qint64 written = 0;
};
la_ssize_t writeArchive(archive*, void* client, const void* data, size_t size) {
    auto& output = *static_cast<Output*>(client);
    if (cancelled(output.token) || size > size_t(archiveLimit - output.written))
        return -1;
    const auto written = output.file->write(static_cast<const char*>(data), qint64(size));
    if (written != qint64(size))
        return -1;
    output.written += written;
    return written;
}
} // namespace
ImageResult saveOra(const QString& path, const Layers& layers, const Cancellation& token,
                    const std::function<bool(QSaveFile&)>& commit) {
    if (!Document::validLayers(layers))
        return error();
    if (cancelled(token))
        return {{}, {}, true};
    QSaveFile file(path);
    file.setDirectWriteFallback(false);
    if (!file.open(QIODevice::WriteOnly))
        return {{}, QCoreApplication::translate("OraIo", "Could not create the project file.")};
    std::unique_ptr<archive, decltype(&archive_write_free)> writer(archive_write_new(),
                                                                   archive_write_free);
    Output output{&file, token};
    if (!writer || archive_write_set_format_zip(writer.get()) != ARCHIVE_OK ||
        archive_write_set_format_option(writer.get(), "zip", "compression", "store") !=
            ARCHIVE_OK ||
        archive_write_open(writer.get(), &output, nullptr, writeArchive, nullptr) != ARCHIVE_OK)
        return error();
    const auto entry = [&](const char* name, const QByteArray& bytes) {
        if (bytes.isEmpty() || cancelled(token))
            return false;
        std::unique_ptr<archive_entry, decltype(&archive_entry_free)> e(archive_entry_new(),
                                                                        archive_entry_free);
        archive_entry_set_pathname(e.get(), name);
        archive_entry_set_filetype(e.get(), AE_IFREG);
        archive_entry_set_perm(e.get(), 0644);
        archive_entry_set_size(e.get(), bytes.size());
        return archive_write_header(writer.get(), e.get()) == ARCHIVE_OK &&
               archive_write_data(writer.get(), bytes.constData(), size_t(bytes.size())) ==
                   bytes.size() &&
               archive_write_finish_entry(writer.get()) == ARCHIVE_OK;
    };
    bool ok = entry("mimetype", "image/openraster");
    QByteArray xml;
    QXmlStreamWriter x(&xml);
    x.writeStartDocument();
    x.writeStartElement("image");
    x.writeAttribute("version", "0.0.3");
    x.writeAttribute("w", QString::number(layers.front().pixels.width()));
    x.writeAttribute("h", QString::number(layers.front().pixels.height()));
    x.writeStartElement("stack");
    for (int i = int(layers.size()) - 1; i >= 0 && ok; --i) {
        const auto& layer = layers[size_t(i)];
        const auto name = QStringLiteral("data/layer%1.png").arg(i);
        x.writeStartElement("layer");
        x.writeAttribute("name", layer.name);
        x.writeAttribute("src", name);
        x.writeAttribute("opacity", QString::number(layer.opacity, 'g', 17));
        x.writeAttribute("visibility", layer.visible ? "visible" : "hidden");
        x.writeAttribute("composite-op", "svg:src-over");
        x.writeEndElement();
        ok = entry(name.toUtf8().constData(), png(layer.pixels));
    }
    x.writeEndElement();
    x.writeEndElement();
    x.writeEndDocument();
    if (ok)
        ok = !x.hasError() && entry("stack.xml", xml);
    if (ok) {
        const auto merged = Document::composite(layers);
        ok = entry("mergedimage.png", png(merged));
        const auto thumbnail =
            merged.width() > 256 || merged.height() > 256
                ? merged.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation)
                : merged;
        if (ok)
            ok = entry("Thumbnails/thumbnail.png", png(thumbnail));
    }
    const bool closed = archive_write_close(writer.get()) == ARCHIVE_OK;
    writer.reset();
    if (cancelled(token)) {
        file.cancelWriting();
        return {{}, {}, true};
    }
    if (!ok || !closed || file.error() != QFileDevice::NoError ||
        !(commit ? commit(file) : file.commit())) {
        file.cancelWriting();
        return {{},
                QCoreApplication::translate(
                    "OraIo", "Project save failed. The original file was preserved.")};
    }
    return {};
}
ImageResult loadOra(const QString& path, const Cancellation& token) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly) || file.size() <= 0 || file.size() > archiveLimit)
        return error();
    const auto bytes = file.read(archiveLimit + 1);
    if (bytes.size() > archiveLimit || file.error() != QFileDevice::NoError)
        return error();
    std::unique_ptr<archive, decltype(&archive_read_free)> reader(archive_read_new(),
                                                                  archive_read_free);
    archive_read_support_format_zip(reader.get());
    if (archive_read_open_memory(reader.get(), bytes.constData(), size_t(bytes.size())) !=
        ARCHIVE_OK)
        return error();
    QMap<QString, QByteArray> files;
    qint64 total = 0;
    int count = 0;
    archive_entry* e = nullptr;
    int status;
    while ((status = archive_read_next_header(reader.get(), &e)) == ARCHIVE_OK) {
        if (cancelled(token))
            return {{}, {}, true};
        const auto name = QString::fromUtf8(archive_entry_pathname(e));
        const auto size = archive_entry_size(e);
        if (++count > 80 || size < 0 || size > 160 * 1024 * 1024 || archive_entry_is_encrypted(e) ||
            archive_entry_symlink(e) || archive_entry_hardlink(e))
            return error();
        if (archive_entry_filetype(e) == AE_IFDIR) {
            if (!safePath(name.endsWith('/') ? name.chopped(1) : name) || size != 0)
                return error();
            continue;
        }
        if (!safePath(name) || archive_entry_filetype(e) != AE_IFREG || files.contains(name))
            return error();
        total += size;
        if (total > archiveLimit || total > qMax<qint64>(1024 * 1024, bytes.size() * 200))
            return error();
        QByteArray data;
        data.resize(size);
        qint64 read = 0;
        while (read < size) {
            if (cancelled(token))
                return {{}, {}, true};
            const auto n = archive_read_data(reader.get(), data.data() + read,
                                             size_t(qMin<qint64>(size - read, 65536)));
            if (n <= 0)
                return error();
            read += n;
        }
        char extra;
        if (archive_read_data(reader.get(), &extra, 1) != 0)
            return error();
        files.insert(name, std::move(data));
    }
    if (status != ARCHIVE_EOF || files.value("mimetype") != "image/openraster" ||
        !files.contains("stack.xml") || files.value("stack.xml").size() > 65536)
        return error();
    QXmlStreamReader x(files.value("stack.xml"));
    Layers layers;
    QSize canvas;
    int depth = 0;
    bool imageSeen = false, stackSeen = false;
    while (!x.atEnd()) {
        x.readNext();
        if (cancelled(token))
            return {{}, {}, true};
        if (x.isDTD() || x.isEntityReference())
            return error();
        if (x.isEndElement()) {
            --depth;
            continue;
        }
        if (!x.isStartElement())
            continue;
        ++depth;
        const auto a = x.attributes();
        if (depth == 1 && x.name() == "image" && !imageSeen) {
            imageSeen = true;
            bool w = false, h = false;
            canvas = QSize(a.value("w").toInt(&w), a.value("h").toInt(&h));
            if (!w || !h || !Document::validSize(canvas))
                return error();
        } else if (depth == 2 && x.name() == "stack" && !stackSeen) {
            stackSeen = true;
            if (!a.isEmpty())
                return error();
        } else if (depth == 3 && stackSeen && x.name() == "layer") {
            if (layers.size() >= 32 ||
                qint64(canvas.width()) * canvas.height() * 4 * qint64(layers.size() + 1) >
                    128 * 1024 * 1024)
                return error();
            const auto src = a.value("src").toString();
            if (!safePath(src) || !files.contains(src) || !src.endsWith(".png") ||
                (a.hasAttribute("composite-op") && a.value("composite-op") != "svg:src-over"))
                return error();
            qreal opacity = 1;
            bool valid = true;
            if (a.hasAttribute("opacity"))
                opacity = a.value("opacity").toDouble(&valid);
            if (!valid || !std::isfinite(opacity) || opacity < 0 || opacity > 1)
                return error();
            const auto visibility = a.value("visibility");
            if (!visibility.isEmpty() && visibility != "visible" && visibility != "hidden")
                return error();
            int dx = 0, dy = 0;
            if (a.hasAttribute("x")) {
                dx = a.value("x").toInt(&valid);
                if (!valid)
                    return error();
            }
            if (a.hasAttribute("y")) {
                dy = a.value("y").toInt(&valid);
                if (!valid)
                    return error();
            }
            if (std::abs(qint64(dx)) > 16384 || std::abs(qint64(dy)) > 16384)
                return error();
            const auto name = a.value("name").toString();
            if (name.size() > 128)
                return error();
            auto decoded = decodePng(files.value(src), token);
            if (!decoded.ok() || decoded.image.isNull())
                return decoded.ok() ? error() : decoded;
            // Reject off-canvas pixels rather than silently losing editable content.
            if (!QRect(QPoint(), canvas).contains(QRect(QPoint(dx, dy), decoded.image.size())))
                return error();
            QImage pixels(canvas, QImage::Format_ARGB32_Premultiplied);
            pixels.fill(Qt::transparent);
            pixels.setColorSpace(QColorSpace::SRgb);
            {
                QPainter p(&pixels);
                p.drawImage(dx, dy, decoded.image);
            }
            layers.insert(layers.begin(), Layer{quint64(layers.size() + 1), name, pixels,
                                                visibility != "hidden", opacity});
        } else
            return error();
    }
    if (x.hasError() || !imageSeen || !stackSeen || !Document::validLayers(layers))
        return error();
    ImageResult result;
    result.layers = std::move(layers);
    result.format = "ora";
    return result;
}
} // namespace openpaint
