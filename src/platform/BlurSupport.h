// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Appearance.h"
#include <QFutureWatcher>
#include <QObject>
#include <QQuickWindow>
#include <QtQml/qqmlregistration.h>
#include <functional>

// Local desktop capability evidence, never an installation or shell executor.
class BlurSupport : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(bool ready READ ready NOTIFY changed)
    Q_PROPERTY(bool setupAvailable READ setupAvailable NOTIFY changed)
    Q_PROPERTY(bool busy READ busy NOTIFY changed)
    Q_PROPERTY(QString command READ command NOTIFY changed)
    Q_PROPERTY(QString instructions READ instructions NOTIFY changed)
  public:
    struct Result {
        bool ready = false;
        QString command;
        QString instructions;
    };
    using Probe = std::function<Result()>;
    using Runner = std::function<QString(const QString&, const QStringList&)>;
    explicit BlurSupport(QObject* parent = nullptr);
    explicit BlurSupport(Probe probe, QObject* parent = nullptr);
    bool ready() const {
        return result_.ready;
    }
    bool setupAvailable() const {
        return !result_.command.isEmpty();
    }
    bool busy() const {
        return busy_;
    }
    QString command() const {
        return result_.command;
    }
    QString instructions() const {
        return result_.instructions;
    }
    // Inject before refresh, for isolated platform/UI tests.
    void setProbe(Probe probe);
    static Result probeDesktop();
    static Result evaluate(const QString& desktop, const QString& session, const QString& platform,
                           const QString& osId, const QString& osVersion, bool sandboxed,
                           const QString& schemaDirectory, const Runner& run);
    Q_INVOKABLE void attachWindow(QQuickWindow* window, Appearance* appearance);
    Q_INVOKABLE void refresh();
    Q_INVOKABLE bool copyCommand();
  signals:
    void changed();
    void probeFinished();

  private:
    bool nativeMac_ = false;
    bool probeInjected_ = false;
    bool busy_ = false;
    Probe probe_;
    Result result_;
    QFutureWatcher<Result> watcher_;
};
