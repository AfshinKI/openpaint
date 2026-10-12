// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QColor>
#include <QImage>
#include <QPointF>
#include <QRect>
#include <QRegion>
namespace openpaint {
class Brush {
  public:
    bool begin(QSize size, QPointF point, QColor color, qreal width, int style, int opacity,
               const QRegion& selection = {});
    QRect extend(QImage& image, const QImage& original, QPointF point);
    void clear() {
        coverage_ = {};
    }
    static QImage preview(int style, QColor color);

  private:
    QRect dab(QImage& image, const QImage& original, QPointF point);
    QImage coverage_;
    QRegion selection_;
    QPointF previous_;
    QColor color_;
    qreal width_ = 1, carry_ = 0;
    int style_ = 0, opacity_ = 100;
    bool first_ = true;
};
} // namespace openpaint
