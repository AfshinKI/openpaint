// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QObject>
#include <QSettings>
#include <QtQml/qqmlregistration.h>
#include <memory>

class Appearance : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(bool glassEnabled READ glassEnabled WRITE setGlassEnabled NOTIFY changed)
    Q_PROPERTY(int surfaceOpacity READ surfaceOpacity WRITE setSurfaceOpacity NOTIFY changed)
  public:
    explicit Appearance(QObject* parent = nullptr);
    explicit Appearance(const QString& settingsFile, QObject* parent = nullptr);
    bool glassEnabled() const {
        return glassEnabled_;
    }
    int surfaceOpacity() const {
        return surfaceOpacity_;
    }
    void setGlassEnabled(bool enabled);
    void setSurfaceOpacity(int opacity);
    Q_INVOKABLE void restoreDefaults();
  signals:
    void changed();

  private:
    void save();
    std::unique_ptr<QSettings> settings_;
    bool glassEnabled_ = true;
    int surfaceOpacity_ = 85;
};
