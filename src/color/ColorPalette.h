// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QColor>
#include <QObject>
#include <QStringList>
#include <QtQml/qqmlregistration.h>

class ColorPalette : public QObject {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QStringList colors READ colors NOTIFY changed)
    Q_PROPERTY(QStringList toolbarColors READ toolbarColors NOTIFY changed)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
  public:
    explicit ColorPalette(QObject* parent = nullptr) : QObject(parent) {}
    QStringList colors() const {
        return colors_;
    }
    QString error() const {
        return error_;
    }
    QStringList toolbarColors() const {
        return toolbarColors_;
    }
    Q_INVOKABLE bool remember(QColor color, int replacementSlot = -1);
    Q_INVOKABLE bool setToolbarColor(int index, QColor color);
    Q_INVOKABLE bool add(QColor color);
    Q_INVOKABLE bool replace(int index, QColor color);
    Q_INVOKABLE bool remove(int index);
    Q_INVOKABLE bool clear();
  signals:
    void changed();
    void errorChanged();

  private:
    bool writeToolbar(const QStringList& colors);
    bool write(const QStringList& colors);
    bool fail(const QString& error);
    QString error_;
    QStringList colors_;
    QStringList toolbarColors_ = QStringList(10, QString());
};
