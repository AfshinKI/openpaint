// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QObject>
#include <QSettings>
#include <QUrl>
#include <QtQml/qqmlregistration.h>
#include <memory>

class Appearance : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(bool smartGuides READ smartGuides WRITE setSmartGuides NOTIFY changed)
    Q_PROPERTY(bool glassEnabled READ glassEnabled WRITE setGlassEnabled NOTIFY changed)
    Q_PROPERTY(bool desktopBlurAvailable READ desktopBlurAvailable WRITE setDesktopBlurAvailable
                   NOTIFY changed)
    Q_PROPERTY(bool effectiveGlassEnabled READ effectiveGlassEnabled NOTIFY changed)
    Q_PROPERTY(int surfaceOpacity READ surfaceOpacity WRITE setSurfaceOpacity NOTIFY changed)
    Q_PROPERTY(QUrl applicationIcon READ applicationIcon NOTIFY changed)
    Q_PROPERTY(bool welcomePending READ welcomePending NOTIFY changed)
    Q_PROPERTY(int themeMode READ themeMode WRITE setThemeMode NOTIFY changed)
  public:
    explicit Appearance(QObject* parent = nullptr);
    explicit Appearance(const QString& settingsFile, QObject* parent = nullptr);
    bool smartGuides() const {
        return smartGuides_;
    }
    void setSmartGuides(bool enabled);
    bool glassEnabled() const {
        return glassEnabled_;
    }
    bool desktopBlurAvailable() const {
        return desktopBlurAvailable_;
    }
    bool effectiveGlassEnabled() const {
        return glassEnabled_ && desktopBlurAvailable_;
    }
    void setDesktopBlurAvailable(bool available);
    int surfaceOpacity() const {
        return surfaceOpacity_;
    }
    int themeMode() const {
        return themeMode_;
    }
    QUrl applicationIcon() const;
    void setThemeMode(int mode);
    void setGlassEnabled(bool enabled);
    void setSurfaceOpacity(int opacity);
    bool welcomePending() const;
    Q_INVOKABLE void dismissWelcome();
    Q_INVOKABLE void restoreDefaults();
  signals:
    void changed();

  private:
    void save();
    void updateApplicationIcon();
    std::unique_ptr<QSettings> settings_;
    bool smartGuides_ = false;
    bool glassEnabled_ = true;
    bool desktopBlurAvailable_ = false;
    int surfaceOpacity_ = 40;
    int themeMode_ = 0;
    bool systemDark_ = false;
};
