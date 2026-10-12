// SPDX-License-Identifier: GPL-3.0-or-later
#include "Text.h"
#include <QPainter>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextOption>
namespace openpaint {
ShapePatch renderText(const QImage& base, QRect rect, const QString& text, const TextStyle& style) {
    if (base.isNull() || rect.isEmpty() || !base.rect().contains(rect) || text.size() > 10000 ||
        style.font.pixelSize() < 1 || style.font.pixelSize() > 512 || !style.foreground.isValid() ||
        !style.background.isValid() || style.alignment < 0 || style.alignment > 2)
        return {};
    QImage image = base.copy(rect);
    QPainter painter(&image);
    if (style.opaque && !text.isEmpty())
        painter.fillRect(image.rect(), style.background);
    QTextDocument document;
    document.setDocumentMargin(0);
    document.setDefaultFont(style.font);
    QTextOption option;
    option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    option.setAlignment(style.alignment == 1   ? Qt::AlignHCenter
                        : style.alignment == 2 ? Qt::AlignRight
                                               : Qt::AlignLeft);
    document.setDefaultTextOption(option);
    document.setPlainText(text);
    document.setTextWidth(rect.width());
    QTextCursor cursor(&document);
    cursor.select(QTextCursor::Document);
    QTextCharFormat format;
    format.setForeground(style.foreground);
    cursor.mergeCharFormat(format);
    painter.setClipRect(image.rect());
    document.drawContents(&painter, image.rect());
    painter.end();
    return {image, rect};
}
} // namespace openpaint
