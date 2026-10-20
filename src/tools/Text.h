// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Shape.h"
#include <QFont>
#include <QTextDocument>
namespace openpaint {
struct TextStyle {
    QFont font;
    QColor foreground = Qt::black, background = Qt::white;
    bool opaque = false;
    int alignment = 0;
};
ShapePatch renderText(const QImage& base, QRect rect, const QString& text, const TextStyle& style,
                      const QTextDocument* formatted = nullptr);
} // namespace openpaint
