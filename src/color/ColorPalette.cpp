// SPDX-License-Identifier: GPL-3.0-or-later
#include "ColorPalette.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>

ColorPalette::ColorPalette(QObject* parent, const QString& path) : QObject(parent), path_(path) {
    if (path_.isEmpty()) {
        const auto root = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
        if (root.isEmpty()) {
            fail(tr("The settings folder is unavailable."));
            return;
        }
        path_ = QDir(root).filePath("palette.json");
    }
    QFile file(path_);
    if (!file.exists())
        return;
    if (!file.open(QIODevice::ReadOnly) || file.size() > 4096) {
        fail(tr("Custom colors could not be read. You can save a new palette."));
        return;
    }
    const auto data = QJsonDocument::fromJson(file.read(4097));
    const auto object = data.object();
    const auto entries = object.value("colors");
    if (object.value("version").toInt() != 1 || !entries.isArray() ||
        entries.toArray().size() > 10) {
        fail(tr("Custom colors are invalid. You can save a new palette."));
        return;
    }
    QStringList loaded;
    const QRegularExpression hex("^#[0-9a-fA-F]{6}$");
    for (const auto& value : entries.toArray()) {
        if (!value.isString() || !hex.match(value.toString()).hasMatch()) {
            fail(tr("Custom colors are invalid. You can save a new palette."));
            return;
        }
        loaded.append(QColor(value.toString()).name());
    }
    colors_ = loaded;
}
bool ColorPalette::fail(const QString& error) {
    error_ = error;
    emit errorChanged();
    return false;
}
bool ColorPalette::write(const QStringList& colors) {
    if (path_.isEmpty() || !QDir().mkpath(QFileInfo(path_).absolutePath()))
        return fail(tr("Cannot create the custom-color settings folder."));
    QSaveFile file(path_);
    file.setDirectWriteFallback(false);
    if (!file.open(QIODevice::WriteOnly))
        return fail(tr("Custom colors could not be saved."));
    const auto data =
        QJsonDocument(QJsonObject{{"version", 1}, {"colors", QJsonArray::fromStringList(colors)}})
            .toJson(QJsonDocument::Compact);
    if (file.write(data) != data.size() || !file.commit())
        return fail(tr("Custom colors could not be saved. The previous palette is unchanged."));
    colors_ = colors;
    error_.clear();
    emit errorChanged();
    emit changed();
    return true;
}
bool ColorPalette::add(QColor color) {
    if (!color.isValid() || colors_.size() >= 10)
        return fail(tr("Choose a valid color. At most ten custom colors can be saved."));
    auto next = colors_;
    next.append(color.name());
    return write(next);
}
bool ColorPalette::replace(int index, QColor color) {
    if (!color.isValid() || index < 0 || index >= colors_.size())
        return false;
    auto next = colors_;
    next[index] = color.name();
    return write(next);
}
bool ColorPalette::remove(int index) {
    if (index < 0 || index >= colors_.size())
        return false;
    auto next = colors_;
    next.removeAt(index);
    return write(next);
}
bool ColorPalette::clear() {
    return write({});
}
