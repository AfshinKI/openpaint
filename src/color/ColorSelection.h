// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QColor>
#include <QObject>
#include <QtQml/qqmlregistration.h>

class ColorSelection : public QObject {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY changed)
    Q_PROPERTY(QString hex READ hex NOTIFY changed)
    Q_PROPERTY(qreal hue READ hue NOTIFY changed)
    Q_PROPERTY(qreal saturation READ saturation NOTIFY changed)
    Q_PROPERTY(qreal value READ value NOTIFY changed)
    Q_PROPERTY(int red READ red NOTIFY changed)
    Q_PROPERTY(int green READ green NOTIFY changed)
    Q_PROPERTY(int blue READ blue NOTIFY changed)
  public:
    explicit ColorSelection(QObject* parent = nullptr) : QObject(parent) {}
    QColor color() const {
        return color_;
    }
    QString hex() const {
        return color_.name().toUpper();
    }
    qreal hue() const {
        return hue_;
    }
    qreal saturation() const {
        return saturation_;
    }
    qreal value() const {
        return value_;
    }
    int red() const {
        return color_.red();
    }
    int green() const {
        return color_.green();
    }
    int blue() const {
        return color_.blue();
    }
    void setColor(QColor color);
    Q_INVOKABLE bool setHex(const QString& text);
    Q_INVOKABLE void setHsv(qreal hue, qreal saturation, qreal value);
    Q_INVOKABLE void setRgb(int red, int green, int blue);
  signals:
    void changed();

  private:
    QColor color_ = Qt::black;
    qreal hue_ = 0, saturation_ = 0, value_ = 0;
};
