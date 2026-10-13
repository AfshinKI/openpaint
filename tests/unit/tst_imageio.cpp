// SPDX-License-Identifier: GPL-3.0-or-later
#include "app/Editor.h"
#include "io/ImageIo.h"
#include <QBuffer>
#include <QColorSpace>
#include <QFile>
#include <QImageWriter>
#include <QTemporaryDir>
#include <QtEndian>
#include <QtTest>

class ImageIoTest : public QObject {
    Q_OBJECT
    static QByteArray encoded(const QImage& image, const QByteArray& format) {
        QByteArray bytes;
        QBuffer buffer(&bytes);
        buffer.open(QIODevice::WriteOnly);
        QImageWriter writer(&buffer, format);
        writer.setQuality(100);
        if (!writer.write(image))
            return {};
        return bytes;
    }
    static bool write(const QString& path, const QByteArray& bytes) {
        QFile file(path);
        return file.open(QIODevice::WriteOnly) && file.write(bytes) == bytes.size();
    }
  private slots:
    void import_data() {
        QTest::addColumn<QByteArray>("format");
        for (const auto& format : {"png", "jpeg", "bmp", "webp"})
            QTest::newRow(format) << QByteArray(format);
    }
    void import() {
        QFETCH(QByteArray, format);
        QTemporaryDir dir;
        const auto path = dir.filePath("image.data");
        QImage source(40, 30, QImage::Format_ARGB32_Premultiplied);
        source.fill(QColor(80, 140, 200));
        source.setText("Author", "Generated fixture");
        source.setColorSpace(QColorSpace::SRgb);
        const auto bytes = encoded(source, format);
        QVERIFY2(!bytes.isEmpty(), format.constData());
        QVERIFY(write(path, bytes));
        const auto result = openpaint::loadImage(path);
        QVERIFY2(result.ok(), qPrintable(result.error));
        QCOMPARE(result.format, format);
        QCOMPARE(result.image.size(), source.size());
        QCOMPARE(result.image.colorSpace(), QColorSpace(QColorSpace::SRgb));
        QVERIFY(result.image.textKeys().isEmpty());
        const auto color = result.image.pixelColor(20, 20);
        QVERIFY(qAbs(color.red() - 80) <= 2);
        QVERIFY(qAbs(color.green() - 140) <= 2);
        QVERIFY(qAbs(color.blue() - 200) <= 2);
        auto token = std::make_shared<std::atomic_bool>(true);
        QVERIFY(openpaint::loadImage(path, token).cancelled);
        Editor editor;
        editor.openFile(QUrl::fromLocalFile(path));
        QTRY_VERIFY(!editor.busy());
        QCOMPARE(editor.image(), result.image);
        QVERIFY(editor.fileUrl().isEmpty());
        QVERIFY(!editor.dirty());
        QCOMPARE(editor.fileName(), QString("image.data"));
        editor.begin({5, 5}, false);
        editor.finish();
        QVERIFY(editor.dirty());
        const auto saved = dir.filePath("copy.png");
        editor.saveFile(QUrl::fromLocalFile(saved));
        QTRY_VERIFY(!editor.busy());
        QVERIFY(!editor.dirty());
        QFile original(path);
        QVERIFY(original.open(QIODevice::ReadOnly));
        QCOMPARE(original.readAll(), bytes);
    }
    void alphaAndOrientation() {
        QTemporaryDir dir;
        auto path = dir.filePath("test.webp");
        QImage source(32, 24, QImage::Format_ARGB32_Premultiplied);
        source.fill(QColor(60, 120, 180, 128));
        QVERIFY(write(path, encoded(source, "webp")));
        const auto result = openpaint::loadImage(path);
        QVERIFY2(result.ok(), qPrintable(result.error));
        QCOMPARE(result.image.pixel(8, 8), source.pixel(8, 8));
        source.fill(Qt::red);
        auto jpeg = encoded(source, "jpeg");
        // EXIF IFD0 orientation=6 rotates 90 degrees; source metadata is not retained.
        auto exif =
            QByteArray::fromHex("45786966000049492a0008000000010012010300010000000600000000000000");
        QByteArray segment = QByteArray::fromHex("ffe10000") + exif;
        qToBigEndian<quint16>(exif.size() + 2, segment.data() + 2);
        auto webp = encoded(source, "webp");
        const auto tiff = exif.mid(6);
        QByteArray chunk = QByteArray("EXIF", 4) + QByteArray(4, 0) + tiff;
        qToLittleEndian<quint32>(tiff.size(), chunk.data() + 4);
        webp += chunk;
        qToLittleEndian<quint32>(webp.size() - 8, webp.data() + 4);
        QVERIFY(write(path, webp));
        const auto webpRotated = openpaint::loadImage(path);
        QVERIFY2(webpRotated.ok(), qPrintable(webpRotated.error));
        QCOMPARE(webpRotated.image.size(), QSize(24, 32));
        jpeg.insert(2, segment);
        QVERIFY(write(path, jpeg));
        const auto rotated = openpaint::loadImage(path);
        QVERIFY2(rotated.ok(), qPrintable(rotated.error));
        QCOMPARE(rotated.image.size(), QSize(24, 32));
        QVERIFY(rotated.image.textKeys().isEmpty());
    }
    void colorConversionAndCancel() {
        QTemporaryDir dir;
        const auto path = dir.filePath("color.jpg");
        QImage source(32, 24, QImage::Format_RGB32);
        source.fill(QColor(120, 170, 80));
        source.setColorSpace(QColorSpace::DisplayP3);
        QVERIFY(write(path, encoded(source, "jpeg")));
        const auto result = openpaint::loadImage(path);
        QVERIFY2(result.ok(), qPrintable(result.error));
        const auto expected = source.convertedToColorSpace(QColorSpace::SRgb).pixelColor(0, 0);
        const auto actual = result.image.pixelColor(0, 0);
        QVERIFY(qAbs(actual.red() - expected.red()) <= 3);
        QVERIFY(qAbs(actual.green() - expected.green()) <= 3);
        QVERIFY(qAbs(actual.blue() - expected.blue()) <= 3);
        Editor editor;
        const auto before = editor.image();
        editor.openFile(QUrl::fromLocalFile(path));
        editor.cancelIo();
        QTRY_VERIFY(!editor.busy());
        QCOMPARE(editor.image(), before);
        QVERIFY(editor.fileUrl().isEmpty());
    }
    void rejectedInputs() {
        QTemporaryDir dir;
        const auto path = dir.filePath("bad.png");
        QImage source(32, 24, QImage::Format_ARGB32);
        source.fill(Qt::red);
        auto jpeg = encoded(source, "jpeg");
        jpeg.chop(2);
        auto webp = encoded(source, "webp");
        QVERIFY(!webp.isEmpty());
        auto animated = webp + QByteArray("ANIM") + QByteArray(10, 0);
        qToLittleEndian<quint32>(6, animated.data() + webp.size() + 4);
        qToLittleEndian<quint32>(animated.size() - 8, animated.data() + 4);
        auto bmp = encoded(source, "bmp");
        qToLittleEndian<quint32>(20000, bmp.data() + 18);
        for (const auto& bytes : {QByteArray("<svg/>"), QByteArray("GIF89a"), jpeg,
                                  webp.first(webp.size() - 1), animated, bmp}) {
            QVERIFY(write(path, bytes));
            const auto result = openpaint::loadImage(path);
            QVERIFY(!result.ok());
            Editor editor;
            editor.begin({3, 3}, false);
            editor.finish();
            const auto before = editor.image();
            editor.openFile(QUrl::fromLocalFile(path));
            QTRY_VERIFY(!editor.busy());
            QCOMPARE(editor.image(), before);
            QVERIFY(editor.dirty());
        }
        QFile huge(path);
        QVERIFY(huge.open(QIODevice::WriteOnly));
        QVERIFY(huge.resize(128 * 1024 * 1024 + 1));
        huge.close();
        QVERIFY(!openpaint::loadImage(path).ok());
    }
};
QTEST_MAIN(ImageIoTest)
#include "tst_imageio.moc"
