// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QImage>
#include <QQuickPaintedItem>
#include <QtQml/qqmlregistration.h>

// Bundled UI examples only. QPainter clipping also works on software renderers.
class ScreenshotPreview : public QQuickPaintedItem {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool glass READ glass WRITE setGlass NOTIFY changed)
    Q_PROPERTY(bool corner READ corner WRITE setCorner NOTIFY changed)
    Q_PROPERTY(bool angled READ angled WRITE setAngled NOTIFY changed)
  public:
    explicit ScreenshotPreview(QQuickItem* parent = nullptr);
    bool glass() const {
        return glass_;
    }
    bool corner() const {
        return corner_;
    }
    bool angled() const {
        return angled_;
    }
    void setAngled(bool angled);
    void setGlass(bool glass);
    void setCorner(bool corner);
    void paint(QPainter* painter) override;
  signals:
    void changed();

  private:
    void loadImage();
    QImage image_;
    bool angled_ = false;
    bool glass_ = true;
    bool corner_ = true;
};
