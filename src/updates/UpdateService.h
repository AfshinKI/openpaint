// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QNetworkAccessManager>
#include <QPointer>
#include <QSettings>
#include <QTimer>
#include <QtQml/qqmlregistration.h>
#include <functional>
#include <memory>

class UpdateService : public QObject {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool automatic READ automatic WRITE setAutomatic NOTIFY changed)
    Q_PROPERTY(bool busy READ busy NOTIFY changed)
    Q_PROPERTY(bool available READ available NOTIFY changed)
    Q_PROPERTY(bool supported READ supported CONSTANT)
    Q_PROPERTY(QString summary READ summary NOTIFY changed)
    Q_PROPERTY(QString message READ message NOTIFY changed)
    Q_PROPERTY(QString latestVersion READ latestVersion NOTIFY changed)
  public:
    explicit UpdateService(QObject* parent = nullptr);
    UpdateService(QNetworkAccessManager* network, const QString& settingsFile,
                  std::function<qint64()> clock, QObject* parent = nullptr);
    bool automatic() const {
        return automatic_;
    }
    bool busy() const {
        return !reply_.isNull();
    }
    bool available() const {
        return available_;
    }
    bool supported() const;
    QString summary() const;
    QString message() const {
        return message_;
    }
    QString latestVersion() const {
        return latestVersion_;
    }
    void setAutomatic(bool enabled);
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void check();
    Q_INVOKABLE void cancel();
    Q_INVOKABLE bool openRelease() const;
    void checkAutomatic();
    static bool newerStable(const QString& tag, const QString& installed, bool* valid);
  signals:
    void changed();
    void updateFound();

  private:
    void request();
    void finish();
    void setMessage(const QString& message);
    std::unique_ptr<QSettings> settings_;
    QNetworkAccessManager* network_;
    std::function<qint64()> clock_;
    QPointer<QNetworkReply> reply_;
    QTimer timer_, timeout_;
    QByteArray body_;
    bool automatic_ = false, available_ = false, tooLarge_ = false, timedOut_ = false;
    bool attempted_ = false, verified_ = false;
    qint64 lastAttempt_ = 0, retryAt_ = 0;
    QString latestVersion_, message_;
};
