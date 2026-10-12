// SPDX-License-Identifier: GPL-3.0-or-later
#include "io/PngIo.h"
#include <QColorSpace>
#include <QFile>
#include <QSaveFile>
#include <QTemporaryDir>
#include <QtTest>
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
        for (const auto& type : {QByteArray("acTL"), QByteArray("eXIf")}) {
            QFile file(path);
            QVERIFY(file.open(QIODevice::WriteOnly));
            file.write(QByteArray::fromHex("89504e470d0a1a0a00000000") + type +
                       QByteArray(4, '\0'));
            file.close();
            const auto result = openpaint::loadPng(path);
            QVERIFY(!result.ok());
            QVERIFY(result.error.contains(type == "acTL" ? "Animated" : "orientation"));
        }
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
