// SPDX-License-Identifier: GPL-3.0-or-later
#include <QDir>
#include <QGuiApplication>
#include <QIcon>
#include <QImage>
int main(int argc, char** argv) {
    QGuiApplication app(argc, argv);
    if (argc != 3)
        return 2;
    const QIcon icon(QString::fromLocal8Bit(argv[1]));
    QDir directory(QString::fromLocal8Bit(argv[2]));
    if (icon.isNull() || !directory.mkpath("."))
        return 1;
    for (int size : {16, 32, 128, 256, 512}) {
        for (int scale : {1, 2}) {
            const auto name = QString("icon_%1x%1%2.png").arg(size).arg(scale == 2 ? "@2x" : "");
            if (!icon.pixmap(size * scale).save(directory.filePath(name)))
                return 1;
        }
    }
    return 0;
}
