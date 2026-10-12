// SPDX-License-Identifier: GPL-3.0-or-later
#include "io/PngIo.h"
#include <QBuffer>
#include <QColorSpace>
#include <QFile>
#include <QImageWriter>
#include <QSaveFile>
#include <QTemporaryDir>
#include <QtEndian>
#include <QtTest>
#include <zlib.h>

static QByteArray chunk(const QByteArray& type, const QByteArray& data) {
    QByteArray result(4, 0);
    qToBigEndian<quint32>(quint32(data.size()), result.data());
    result += type + data;
    const auto crc =
        crc32(0, reinterpret_cast<const Bytef*>(result.constData() + 4), result.size() - 4);
    QByteArray sum(4, 0);
    qToBigEndian<quint32>(crc, sum.data());
    return result + sum;
}
static QByteArray png(const QImage& image) {
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return bytes;
}
static bool writeBytes(const QString& path, const QByteArray& bytes) {
    QFile file(path);
    return file.open(QIODevice::WriteOnly) && file.write(bytes) == bytes.size();
}
static QByteArray exif(int orientation, bool little) {
    QByteArray bytes =
        QByteArray::fromHex(little ? "49492a0008000000010012010300010000000000000000000000"
                                   : "4d4d002a00000008000101120003000000010000000000000000");
    if (little)
        qToLittleEndian<quint16>(orientation, bytes.data() + 18);
    else
        qToBigEndian<quint16>(orientation, bytes.data() + 18);
    return bytes;
}

class PngTest : public QObject {
    Q_OBJECT
  private slots:
    void roundTripAndMetadata() {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QImage image(32, 24, QImage::Format_ARGB32_Premultiplied);
        for (int y = 0; y < image.height(); ++y)
            for (int x = 0; x < image.width(); ++x)
                image.setPixelColor(x, y, QColor(x * 7, y * 10, 50, (x + y) * 4));
        image.setColorSpace(QColorSpace::SRgb);
        image.setText("Author", "Synthetic fixture");
        auto path = dir.filePath("image.png");
        auto result = openpaint::savePng(path, image);
        QVERIFY2(result.ok(), qPrintable(result.error));
        const auto loaded = openpaint::loadPng(path);
        QVERIFY2(loaded.ok(), qPrintable(loaded.error));
        // QImage equality includes metadata, so compare raw pixel data explicitly.
        QCOMPARE(loaded.image.size(), image.size());
        for (int y = 0; y < image.height(); ++y)
            for (int x = 0; x < image.width(); ++x)
                QCOMPARE(loaded.image.pixel(x, y), image.pixel(x, y));
        QVERIFY(loaded.image.textKeys().isEmpty());
        QCOMPARE(loaded.image.colorSpace(), QColorSpace(QColorSpace::SRgb));
    }
    void importedMetadataAndTruncation() {
        QTemporaryDir dir;
        const auto path = dir.filePath("tagged.png");
        QImage image(16, 16, QImage::Format_RGB32);
        image.fill(Qt::green);
        image.setText("Author", "Synthetic fixture");
        QVERIFY(image.save(path, "PNG"));
        const auto loaded = openpaint::loadPng(path);
        QVERIFY(loaded.ok());
        QVERIFY(!loaded.notice.isEmpty());
        QVERIFY(loaded.image.textKeys().isEmpty());
        QFile file(path);
        QVERIFY(file.open(QIODevice::ReadWrite));
        QVERIFY(file.resize(file.size() - 12));
        file.close();
        QVERIFY(!openpaint::loadPng(path).ok());
    }

    void failuresPreserveOriginal() {
        QTemporaryDir dir;
        const auto path = dir.filePath("original.png");
        QImage image(16, 16, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::red);
        QVERIFY(openpaint::savePng(path, image).ok());
        QFile original(path);
        QVERIFY(original.open(QIODevice::ReadOnly));
        const auto bytes = original.readAll();
        original.close();
        image.fill(Qt::blue);
        auto result = openpaint::savePng(path, image, {}, [](QSaveFile& file) {
            file.cancelWriting();
            return false;
        });
        QVERIFY(!result.ok());
        QVERIFY(original.open(QIODevice::ReadOnly));
        QCOMPARE(original.readAll(), bytes);
        original.close();
        auto cancellation = std::make_shared<std::atomic_bool>(true);
        QVERIFY(openpaint::savePng(path, image, cancellation).cancelled);
        QVERIFY(openpaint::loadPng(path, cancellation).cancelled);
        QVERIFY(original.open(QIODevice::ReadOnly));
        QCOMPARE(original.readAll(), bytes);
        QVERIFY(!openpaint::savePng(dir.filePath("missing/target.png"), image).ok());
        QVERIFY(!openpaint::savePng(dir.path(), image).ok());
    }
    void colorConversionAndUnsupportedChunks() {
        QTemporaryDir dir;
        const auto path = dir.filePath("fixture.png");
        QImage image(8, 8, QImage::Format_ARGB32_Premultiplied);
        image.fill(QColor(100, 150, 200));
        image.setColorSpace(QColorSpace::DisplayP3);
        const auto expected = image.convertedToColorSpace(QColorSpace::SRgb);
        QVERIFY(image.save(path, "PNG"));
        const auto loaded = openpaint::loadPng(path);
        QVERIFY(loaded.ok());
        QCOMPARE(loaded.image.pixel(0, 0), expected.pixel(0, 0));
        const auto bytes = png(image);
        QVERIFY(
            writeBytes(path, bytes.first(33) + chunk("acTL", QByteArray(8, 0)) + bytes.mid(33)));
        QVERIFY(openpaint::loadPng(path).error.contains("Animated"));
    }

    void orientations_data() {
        QTest::addColumn<int>("orientation");
        QTest::addColumn<bool>("little");
        for (int o = 1; o <= 8; ++o)
            for (bool le : {false, true})
                QTest::newRow(qPrintable(QString("%1-%2").arg(o).arg(le))) << o << le;
    }
    void orientations() {
        QFETCH(int, orientation);
        QFETCH(bool, little);
        QTemporaryDir dir;
        const auto path = dir.filePath("oriented.png");
        QImage source(3, 2, QImage::Format_ARGB32_Premultiplied);
        for (int y = 0; y < 2; ++y)
            for (int x = 0; x < 3; ++x)
                source.setPixelColor(x, y, QColor(20 + x * 70, 30 + y * 100, 10, 128 + x * 30));
        const auto bytes = png(source);
        QVERIFY(writeBytes(path, bytes.first(33) + chunk("eXIf", exif(orientation, little)) +
                                     bytes.mid(33)));
        const auto loaded = openpaint::loadPng(path);
        QVERIFY2(loaded.ok(), qPrintable(loaded.error));
        QVERIFY(!loaded.notice.isEmpty());
        QCOMPARE(loaded.image.size(), orientation >= 5 ? QSize(2, 3) : QSize(3, 2));
        for (int y = 0; y < 2; ++y) {
            for (int x = 0; x < 3; ++x) {
                const QPoint positions[] = {{x, y}, {2 - x, y}, {2 - x, 1 - y}, {x, 1 - y},
                                            {y, x}, {1 - y, x}, {1 - y, 2 - x}, {y, 2 - x}};
                QCOMPARE(loaded.image.pixel(positions[orientation - 1]), source.pixel(x, y));
            }
        }
        const auto saved = dir.filePath("normalized.png");
        QVERIFY(openpaint::savePng(saved, loaded.image).ok());
        QCOMPARE(openpaint::loadPng(saved).image, loaded.image);
        QFile file(saved);
        QVERIFY(file.open(QIODevice::ReadOnly));
        QVERIFY(!file.readAll().contains("eXIf"));
    }
    void invalidExifAndChunks() {
        QTemporaryDir dir;
        const auto path = dir.filePath("invalid.png");
        QImage image(3, 2, QImage::Format_RGB32);
        image.fill(Qt::red);
        const auto bytes = png(image);
        QList<QByteArray> invalid{QByteArray(), exif(0, true), exif(9, false)};
        auto offset = exif(1, true);
        offset.replace(4, 4, QByteArray(4, char(255)));
        invalid << offset << exif(1, true).first(20);
        auto type = exif(1, true);
        type[12] = 4;
        invalid << type;
        auto count = exif(1, true);
        count[14] = 2;
        invalid << count;
        auto duplicate = exif(1, true);
        duplicate[8] = 2;
        duplicate.insert(22, duplicate.mid(10, 12));
        invalid << duplicate;
        for (const auto& data : invalid) {
            QVERIFY(writeBytes(path, bytes.first(33) + chunk("eXIf", data) + bytes.mid(33)));
            QVERIFY(!openpaint::loadPng(path).ok());
        }
        const auto tag = chunk("eXIf", exif(1, true));
        for (const auto& damaged :
             {bytes.first(33) + tag + tag + bytes.mid(33), bytes + QByteArray("trailing"),
              bytes.first(33) + bytes.mid(8, 25) + bytes.mid(33),
              bytes.first(8) + chunk("IEND", {})}) {
            QVERIFY(writeBytes(path, damaged));
            QVERIFY(!openpaint::loadPng(path).ok());
        }
        auto badCrc = bytes;
        badCrc[32] = char(badCrc[32] ^ 1);
        QVERIFY(writeBytes(path, badCrc));
        QVERIFY(!openpaint::loadPng(path).ok());
        // Valid IFD0 without an orientation tag defaults to the stored pixel order.
        const auto emptyIfd = QByteArray::fromHex("49492a0008000000000000000000");
        QVERIFY(writeBytes(path, bytes.first(33) + chunk("eXIf", emptyIfd) + bytes.mid(33)));
        QVERIFY(openpaint::loadPng(path).ok());
    }
    void invalidColorProfile() {
        QTemporaryDir dir;
        const auto path = dir.filePath("profile.png");
        QImage image(3, 2, QImage::Format_RGB32);
        image.fill(Qt::red);
        const auto bytes = png(image);
        for (const auto& profile : {QByteArray("invalid"), QByteArray(4 * 1024 * 1024 + 1, 'x')}) {
            const auto payload = QByteArray("profile\0\0", 9) + qCompress(profile).mid(4);
            QVERIFY(writeBytes(path, bytes.first(33) + chunk("iCCP", payload) + bytes.mid(33)));
            QVERIFY(!openpaint::loadPng(path).ok());
        }
    }
    void writeFailuresAndLateCancel() {
        QTemporaryDir dir;
        const auto path = dir.filePath("original.png");
        QImage image(32, 24, QImage::Format_RGB32);
        image.fill(Qt::red);
        const auto original = png(image);
        QVERIFY(writeBytes(path, original));
        image.fill(Qt::blue);
        // A device that accepts a prefix, then reports the same failure as ENOSPC.
        class FullDevice : public QIODevice {
          public:
            explicit FullDevice(QIODevice& target) : target(target) {
                open(WriteOnly);
            }
            qint64 readData(char*, qint64) override {
                return -1;
            }
            qint64 writeData(const char* data, qint64 size) override {
                if (remaining == 0) {
                    setErrorString("No space left on device");
                    return -1;
                }
                const auto amount = std::min(size, remaining);
                remaining -= amount;
                return target.write(data, amount);
            }
            QIODevice& target;
            qint64 remaining = 40;
        };
        auto result =
            openpaint::savePng(path, image, {}, {}, [](QIODevice& target, const QImage& clean) {
                FullDevice full(target);
                return openpaint::writePng(full, clean);
            });
        QVERIFY(!result.ok());
        QFile file(path);
        QVERIFY(file.open(QIODevice::ReadOnly));
        QCOMPARE(file.readAll(), original);
        file.close();
        auto cancellation = std::make_shared<std::atomic_bool>(false);
        result = openpaint::savePng(path, image, cancellation, {},
                                    [&](QIODevice& target, const QImage& clean) {
                                        const bool ok = openpaint::writePng(target, clean);
                                        cancellation->store(true);
                                        return ok;
                                    });
        QVERIFY(result.cancelled);
        QVERIFY(file.open(QIODevice::ReadOnly));
        QCOMPARE(file.readAll(), original);
        file.close();
        QVERIFY(QFile::setPermissions(dir.path(), QFileDevice::ReadOwner | QFileDevice::ExeOwner));
        const auto denied = openpaint::savePng(path, image);
        QVERIFY(QFile::setPermissions(dir.path(), QFileDevice::ReadOwner | QFileDevice::WriteOwner |
                                                      QFileDevice::ExeOwner));
        QVERIFY(!denied.ok());
        QVERIFY(file.open(QIODevice::ReadOnly));
        QCOMPARE(file.readAll(), original);
    }

    void malformedAndOversized() {
        QTemporaryDir dir;
        const auto path = dir.filePath("broken.png");
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("not a png");
        file.close();
        QVERIFY(!openpaint::loadPng(path).ok());
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(QByteArray::fromHex("89504e470d0a1a0a7fffffff49444154"));
        file.close();
        QVERIFY(!openpaint::loadPng(path).ok());
        // Generated valid PNG header with dimensions outside our allocation bounds.
        QImage wide(16385, 1, QImage::Format_RGB32);
        wide.fill(Qt::white);
        QVERIFY(wide.save(path, "PNG"));
        QVERIFY(!openpaint::loadPng(path).ok());
    }
};
QTEST_GUILESS_MAIN(PngTest)
#include "tst_pngio.moc"
