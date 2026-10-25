// SPDX-License-Identifier: GPL-3.0-or-later
#include "app/Editor.h"
#include "io/OraIo.h"
#include <QBuffer>
#include <QColorSpace>
#include <QFile>
#include <QSaveFile>
#include <QTemporaryDir>
#include <QtTest>
#include <archive.h>
#include <archive_entry.h>
#include <limits>
using namespace openpaint;
class LayerEditor : public Editor {
  public:
    using Editor::document;
};
class LayersTest : public QObject {
    Q_OBJECT
    static QImage solid(QColor color) {
        QImage image(12, 8, QImage::Format_ARGB32_Premultiplied);
        image.fill(color);
        image.setColorSpace(QColorSpace::SRgb);
        return image;
    }
    static void zip(const QString& path, const QList<QPair<QByteArray, QByteArray>>& entries) {
        auto* a = archive_write_new();
        archive_write_set_format_zip(a);
        archive_write_open_filename(a, path.toUtf8().constData());
        for (const auto& pair : entries) {
            auto* e = archive_entry_new();
            archive_entry_set_pathname(e, pair.first.constData());
            archive_entry_set_filetype(e, AE_IFREG);
            archive_entry_set_perm(e, 0600);
            archive_entry_set_size(e, pair.second.size());
            archive_write_header(a, e);
            archive_write_data(a, pair.second.constData(), size_t(pair.second.size()));
            archive_entry_free(e);
        }
        archive_write_close(a);
        archive_write_free(a);
    }
  private slots:
    void panelInitializationAndPinnedBackground() {
        LayerEditor e;
        const auto original = e.image();
        QVERIFY(e.enableLayers());
        QCOMPARE(e.layers().size(), 2);
        QCOMPARE(e.activeLayer(), 1);
        QCOMPARE(e.image().pixelColor(0, 0).alpha(), 0);
        QCOMPARE(e.document.layers()[0].pixels, original);
        const auto revision = e.document.revision();
        QVERIFY(e.enableLayers());
        QCOMPARE(e.document.revision(), revision);
        e.undo();
        QCOMPARE(e.layers().size(), 1);
        QVERIFY(e.enableLayers());
        QCOMPARE(e.layers().size(), 1);
        e.redo();
        QCOMPARE(e.layers().size(), 2);
        e.addLayer();
        const auto bottomId = e.document.layers()[0].id;
        const auto topId = e.layers()[0].toMap()["id"].toString();
        e.reorderLayer(topId, 0);
        QCOMPARE(e.activeLayer(), 2);
        e.reorderLayer(topId, 1);
        QCOMPARE(e.activeLayer(), 1);
        QCOMPARE(e.document.layers()[0].id, bottomId);
        e.selectLayer(0);
        QVERIFY(!e.document.moveLayer(2));
        QVERIFY(!e.document.deleteLayer());
        e.selectLayer(1);
        e.toggleLayerVisibility(0);
        QCOMPARE(e.activeLayer(), 1);
        QVERIFY(!e.document.layers()[0].visible);
        e.undo();
        QVERIFY(e.document.layers()[0].visible);
        QCOMPARE(e.activeLayer(), 1);
        e.newDocument();
        QVERIFY(e.enableLayers());
        QCOMPARE(e.layers().size(), 2);
        e.begin({20, 20}, false);
        QVERIFY(!e.enableLayers());
        e.cancel();
        QTemporaryDir dir;
        const auto path = QUrl::fromLocalFile(dir.filePath("pinned.ora"));
        e.saveFile(path);
        QTRY_VERIFY(!e.busy());
        e.newDocument();
        e.openFile(path);
        QTRY_VERIFY(!e.busy());
        QVERIFY(e.enableLayers());
        QCOMPARE(e.layers().size(), 2);
        e.selectLayer(0);
        QVERIFY(!e.document.moveLayer(1));
        QVERIFY(!e.document.deleteLayer());
    }
    void stackCommandsAndHistory() {
        Document d({12, 8});
        QVERIFY(d.addLayer());
        const auto id = d.layers()[1].id;
        QCOMPARE(d.image().pixelColor(0, 0).alpha(), 0);
        QVERIFY(d.renameLayer("Ink"));
        QVERIFY(d.setLayerOpacity(.5));
        QVERIFY(d.beginStroke({4, 4}, Qt::red, 3));
        QVERIFY(!d.selectLayer(0));
        QVERIFY(!d.deleteLayer());
        QVERIFY(d.commitStroke());
        const auto ink = d.image();
        QVERIFY(d.selectLayer(0));
        QCOMPARE(d.image(), solid(Qt::white));
        QVERIFY(d.undo());
        QCOMPARE(d.activeLayer(), 1);
        QCOMPARE(d.image().pixelColor(4, 4).alpha(), 0);
        QVERIFY(d.redo());
        QCOMPARE(d.image(), ink);
        d.markSaved(d.revision());
        QVERIFY(!d.dirty());
        QVERIFY(d.selectLayer(0));
        QVERIFY(!d.dirty());
        QVERIFY(d.selectLayer(1));
        QVERIFY(d.addLayer(true));
        QCOMPARE(d.image(), ink);
        QVERIFY(d.layers()[2].id != id);
        QVERIFY(!d.moveLayer(0));
        QVERIFY(d.moveLayer(1));
        QCOMPARE(d.layers()[1].pixels, ink);
        QVERIFY(d.undo());
        QCOMPARE(d.activeLayer(), 2);
        QVERIFY(d.deleteLayer());
        QCOMPARE(d.layers().size(), size_t(2));
        QVERIFY(d.undo());
        QCOMPARE(d.layers().size(), size_t(3));
        QVERIFY(d.redo());
        QVERIFY(d.setLayerVisible(false));
        QCOMPARE(d.composite(), solid(Qt::white));
        QVERIFY(d.undo());
        QVERIFY(d.layers()[1].visible);
        QVERIFY(d.renameLayer("Branch"));
        QVERIFY(!d.canRedo());
    }
    void compositingAndMerge() {
        Document d({12, 8});
        QVERIFY(d.reset(solid(Qt::blue)));
        QVERIFY(d.addLayer());
        QVERIFY(d.applyPatch(solid(Qt::red), d.image().rect(), d.revision()));
        QVERIFY(d.setLayerOpacity(.5));
        const auto merged = d.composite();
        const auto color = merged.pixelColor(2, 2);
        QVERIFY(qAbs(color.red() - 128) <= 1);
        QVERIFY(qAbs(color.blue() - 127) <= 1);
        QVERIFY(d.mergeDown());
        QCOMPARE(d.layers().size(), size_t(1));
        QCOMPARE(d.image(), merged);
        QVERIFY(d.undo());
        QCOMPARE(d.layers().size(), size_t(2));
        QCOMPARE(d.composite(), merged);
        QVERIFY(d.redo());
        QCOMPARE(d.composite(), merged);
        QVERIFY(!d.deleteLayer());
        QVERIFY(!d.mergeDown());
    }
    void limitsAndSavedRevision() {
        Document d({12, 8}, 800);
        QVERIFY(!d.addLayer());
        QVERIFY(!d.dirty());
        QCOMPARE(d.layers().size(), size_t(1));
        Document normal({12, 8});
        QVERIFY(!normal.setLayerOpacity(std::numeric_limits<double>::quiet_NaN()));
        QVERIFY(!normal.renameLayer(QString(129, 'a')));
        QVERIFY(normal.addLayer());
        normal.markSaved(normal.revision());
        QVERIFY(normal.renameLayer("Changed"));
        QVERIFY(normal.dirty());
        QVERIFY(normal.undo());
        QVERIFY(!normal.dirty());
        QVERIFY(normal.redo());
        QVERIFY(normal.dirty());
    }
    void roundTripAndFailedSave() {
        QTemporaryDir dir;
        const auto path = dir.filePath("painting.ora");
        Layers layers{
            {1, "Background", solid(Qt::blue)},
            {2, QString::fromUtf8("Ink & <é>"), solid(QColor(255, 0, 0, 128)), false, .37}};
        QVERIFY(saveOra(path, layers).ok());
        const auto result = loadOra(path);
        QVERIFY2(result.ok(), qPrintable(result.error));
        QCOMPARE(result.layers.size(), layers.size());
        for (size_t i = 0; i < layers.size(); ++i) {
            QCOMPARE(result.layers[i].pixels, layers[i].pixels);
            QCOMPARE(result.layers[i].name, layers[i].name);
            QCOMPARE(result.layers[i].visible, layers[i].visible);
            QCOMPARE(result.layers[i].opacity, layers[i].opacity);
        }
        QFile file(path);
        QVERIFY(file.open(QIODevice::ReadOnly));
        const auto original = file.readAll();
        file.close();
        layers[0].pixels.fill(Qt::green);
        QVERIFY(!saveOra(path, layers, {}, [](QSaveFile&) { return false; }).ok());
        QVERIFY(file.open(QIODevice::ReadOnly));
        QCOMPARE(file.readAll(), original);
        file.close();
        layers[1].name = QString(QChar(0));
        QVERIFY(!saveOra(path, layers).ok());
        QVERIFY(file.open(QIODevice::ReadOnly));
        QCOMPARE(file.readAll(), original);
        file.close();
        auto token = std::make_shared<std::atomic_bool>(true);
        QVERIFY(saveOra(path, layers, token).cancelled);
        QVERIFY(loadOra(path, token).cancelled);
    }
    void activeLayerToolsAndSelection() {
        LayerEditor e;
        e.document.reset(solid(Qt::blue));
        e.addLayer();
        const auto background = e.document.layers()[0].pixels;
        e.setTool(Editor::Tool::Fill);
        e.primary = Qt::red;
        e.begin({2, 2}, false);
        e.selectLayer(0);
        QCOMPARE(e.activeLayer(), 1);
        QTRY_VERIFY(!e.busy());
        QCOMPARE(e.image(), solid(Qt::red));
        QCOMPARE(e.document.layers()[0].pixels, background);
        e.selectRect(0, 0, 3, 3);
        e.deleteSelection();
        QCOMPARE(e.image().pixelColor(1, 1).alpha(), 0);
        QCOMPARE(e.displayImage().pixelColor(1, 1), QColor(Qt::blue));
        QCOMPARE(e.selectedImage().pixelColor(1, 1).alpha(), 0);
        QCOMPARE(e.selectedImage(true).pixelColor(1, 1), QColor(Qt::blue));
        e.undo();
        QCOMPARE(e.image(), solid(Qt::red));
        e.setLayerOpacity(0);
        e.setTool(Editor::Tool::ColorPicker);
        e.begin({1, 1}, false);
        QCOMPARE(e.primary, QColor(Qt::red));
        e.sampleMerged = true;
        e.begin({1, 1}, false);
        QCOMPARE(e.primary, QColor(Qt::blue));
        e.setLayerVisible(false);
        e.setTool(Editor::Tool::Pencil);
        const auto revision = e.document.revision();
        e.begin({1, 1}, false);
        e.finish();
        QCOMPARE(e.document.revision(), revision);
    }
    void canvasExtensionUsesBackgroundColor() {
        for (const QColor background : {QColor(Qt::white), QColor(Qt::green),
                                        QColor(30, 60, 90, 128), QColor(Qt::transparent)}) {
            for (bool layered : {false, true}) {
                LayerEditor e;
                e.document.reset(solid(Qt::blue));
                if (layered)
                    e.addLayer();
                const auto before = e.displayImage();
                e.secondary = background;
                e.resizeCanvas(-2, -3, 16, 14);
                QTRY_VERIFY(!e.busy());
                QCOMPARE(e.imageSize(), QSize(16, 14));
                QCOMPARE(e.document.layers()[0].pixels.pixel(0, 0),
                         qPremultiply(background.rgba()));
                QCOMPARE(e.displayImage().pixel(0, 0), qPremultiply(background.rgba()));
                QCOMPARE(e.displayImage().pixelColor(2, 3), QColor(Qt::blue));
                if (layered)
                    QCOMPARE(e.document.layers()[1].pixels.pixelColor(0, 0).alpha(), 0);
                const auto expanded = e.displayImage();
                e.undo();
                QCOMPARE(e.displayImage(), before);
                e.redo();
                QCOMPARE(e.displayImage(), expanded);
            }
        }
    }
    void wholeStackCropResizeAndCancel() {
        LayerEditor e;
        e.document.reset(solid(Qt::blue));
        e.addLayer();
        QVERIFY(e.document.applyPatch(solid(Qt::red), e.image().rect(), e.document.revision()));
        const auto unchangedRevision = e.document.revision();
        e.transformImage(int(TransformKind::RotateHalf));
        QTRY_VERIFY(!e.busy());
        QCOMPARE(e.document.revision(), unchangedRevision);
        e.selectRect(2, 2, 5, 4);
        e.transformImage(int(TransformKind::Crop));
        QTRY_VERIFY(!e.busy());
        QCOMPARE(e.imageSize(), QSize(5, 4));
        QCOMPARE(e.document.layers()[0].pixels.size(), QSize(5, 4));
        e.undo();
        QCOMPARE(e.imageSize(), QSize(12, 8));
        e.resizeCanvas(-2, -2, 16, 12);
        QTRY_VERIFY(!e.busy());
        for (const auto& layer : e.document.layers()) {
            QCOMPARE(layer.pixels.size(), QSize(16, 12));
        }
        QCOMPARE(e.document.layers()[0].pixels.pixelColor(0, 0), e.secondary);
        QCOMPARE(e.document.layers()[1].pixels.pixelColor(0, 0).alpha(), 0);
        e.undo();
        e.resizeImage(6, 4, 0, 0, false);
        QTRY_VERIFY(!e.busy());
        QCOMPARE(e.imageSize(), QSize(6, 4));
        QCOMPARE(e.document.layers()[0].pixels.size(), QSize(6, 4));
        e.undo();
        const auto before = e.displayImage();
        e.transformImage(int(TransformKind::RotateRight));
        e.cancelIo();
        QTRY_VERIFY(!e.busy());
        QCOMPARE(e.displayImage(), before);
    }
    void boundedCompositing() {
        Layers layers;
        for (int i = 0; i < 8; ++i) {
            QImage raster(2048, 2048, QImage::Format_ARGB32_Premultiplied);
            raster.setColorSpace(QColorSpace::SRgb);
            raster.fill(QColor::fromHsv(i * 40, 180, 210, 160));
            layers.push_back({quint64(i + 1), QString::number(i), raster, true, .8});
        }
        QVERIFY(Document::validLayers(layers));
        std::vector<double> times;
        for (int i = 0; i < 35; ++i) {
            QElapsedTimer timer;
            timer.start();
            const auto composed = Document::composite(layers);
            const double ms = timer.nsecsElapsed() / 1000000.0;
            QCOMPARE(composed.size(), QSize(2048, 2048));
            QVERIFY(composed.pixelColor(100, 100).alpha() > 250);
            if (i >= 5)
                times.push_back(ms);
        }
        std::sort(times.begin(), times.end());
        qInfo("Eight-layer 2048px CPU composition: p50 %.2f ms; p95 %.2f ms (30 samples)",
              times[14], times[28]);
        auto excessive = layers;
        excessive.push_back(layers.back());
        QVERIFY(!Document::validLayers(excessive));
    }
    void malformedProjects() {
        QTemporaryDir dir;
        const auto path = dir.filePath("bad.ora");
        QByteArray image;
        QBuffer buffer(&image);
        buffer.open(QIODevice::WriteOnly);
        QVERIFY(writePng(buffer, solid(Qt::red)));
        const QByteArray prefix = "<image w=\"12\" h=\"8\"><stack>";
        const QByteArray suffix = "</stack></image>";
        const QList<QByteArray> invalid{"<stack/>",
                                        "<layer src=\"data/a.png\" composite-op=\"svg:multiply\"/>",
                                        "<layer src=\"../a.png\"/>",
                                        "<layer src=\"data/missing.png\"/>",
                                        "<layer src=\"data/a.png\" opacity=\"nan\"/>",
                                        "<layer src=\"data/a.png\" x=\"-1\"/>",
                                        "<layer src=\"data/a.png\" visibility=\"maybe\"/>"};
        for (const auto& xml : invalid) {
            zip(path, {{"mimetype", "image/openraster"},
                       {"stack.xml", prefix + xml + suffix},
                       {"data/a.png", image}});
            QVERIFY(!loadOra(path).ok());
        }
        zip(path, {{"mimetype", "image/openraster"}, {"../escape", "bad"}});
        QVERIFY(!loadOra(path).ok());
        zip(path, {{"mimetype", "image/openraster"}, {"mimetype", "image/openraster"}});
        QVERIFY(!loadOra(path).ok());
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("not a zip");
        f.close();
        QVERIFY(!loadOra(path).ok());
    }
    void controllerEditsSaveAndTransforms() {
        LayerEditor e;
        e.document.reset(solid(Qt::blue));
        e.addLayer();
        QCOMPARE(e.activeLayer(), 1);
        QVERIFY(e.needsProject());
        e.primary = Qt::red;
        e.brushSize = 3;
        e.begin({4, 4}, false);
        e.selectLayer(0);
        QCOMPARE(e.activeLayer(), 1);
        e.finish();
        QCOMPARE(e.document.layers()[0].pixels, solid(Qt::blue));
        QVERIFY(e.image().pixelColor(4, 4).red() > 0);
        QTemporaryDir dir;
        QSignalSpy saved(&e, &Editor::saved);
        e.saveFile(QUrl::fromLocalFile(dir.filePath("bad.png")));
        QCOMPARE(saved.count(), 1);
        QCOMPARE(saved.takeFirst()[0].toBool(), false);
        QVERIFY(e.dirty());
        const auto before = e.displayImage();
        e.saveFile(QUrl::fromLocalFile(dir.filePath("drawing.ora")));
        QTRY_VERIFY(!e.busy());
        QVERIFY(!e.dirty());
        e.transformImage(int(TransformKind::RotateRight));
        QTRY_VERIFY(!e.busy());
        QCOMPARE(e.document.layers().size(), size_t(2));
        QCOMPARE(e.imageSize(), QSize(8, 12));
        e.undo();
        QCOMPARE(e.displayImage(), before);
        QVERIFY(!e.dirty());
        e.newDocument();
        e.openFile(QUrl::fromLocalFile(dir.filePath("drawing.ora")));
        QTRY_VERIFY(!e.busy());
        QCOMPARE(e.document.layers().size(), size_t(2));
        QCOMPARE(e.displayImage(), before);
        e.openFile(QUrl::fromLocalFile(dir.filePath("missing.ora")));
        QTRY_VERIFY(!e.busy());
        QCOMPARE(e.displayImage(), before);
    }
};
QTEST_GUILESS_MAIN(LayersTest)
#include "tst_layers.moc"
