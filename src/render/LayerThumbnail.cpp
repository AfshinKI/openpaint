// SPDX-License-Identifier: GPL-3.0-or-later
#include "LayerThumbnail.h"
#include <QPainter>
void LayerThumbnail::setEditor(Editor* editor) {
    if (editor_ == editor)
        return;
    if (editor_)
        disconnect(editor_, nullptr, this, nullptr);
    editor_ = editor;
    if (editor_) {
        connect(editor_, &Editor::pixelsChanged, this, [this] { update(); });
        connect(editor_, &Editor::changed, this, [this] { update(); });
    }
    update();
    emit changed();
}
void LayerThumbnail::paint(QPainter* painter) {
    if (!editor_)
        return;
    const auto image = editor_->layerImage(index_);
    if (image.isNull())
        return;
    const auto size = QSizeF(image.size()).scaled(boundingRect().size(), Qt::KeepAspectRatio);
    const QRectF box((width() - size.width()) / 2, (height() - size.height()) / 2, size.width(),
                     size.height());
    painter->save();
    painter->setClipRect(box);
    painter->fillRect(box, Qt::white);
    for (int y = 0; y < height(); y += 6)
        for (int x = 0; x < width(); x += 6)
            if ((x / 6 + y / 6) % 2)
                painter->fillRect(x, y, 6, 6, QColor("#cccccc"));
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->drawImage(box, image);
    painter->restore();
}
