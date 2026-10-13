// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QObject>
#include <QUrl>
#include <QVariant>
#include <QtQml/qqmlregistration.h>

class AppInfo : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(QString channel READ channel CONSTANT)
    Q_PROPERTY(QString installation READ installation CONSTANT)
    Q_PROPERTY(QString instructions READ instructions CONSTANT)
    Q_PROPERTY(QUrl releaseUrl READ releaseUrl CONSTANT)
  public:
    explicit AppInfo(QObject* parent = nullptr) : QObject(parent) {}
    static QString version();
    QString channel() const;
    QString installation() const;
    QString instructions() const;
    QUrl releaseUrl() const;
    Q_INVOKABLE QString shortcutText(const QVariant& shortcut) const;
    Q_INVOKABLE bool openReleases() const;
};
