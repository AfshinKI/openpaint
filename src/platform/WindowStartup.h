// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QQuickWindow>

// Map an invisible native window so Qt can render, then reveal only a submitted
// frame. Keep the opacity change on the GUI thread, not the render thread.
inline void revealAfterFirstFrame(QQuickWindow* window) {
    window->setOpacity(0);
    QObject::connect(
        window, &QQuickWindow::frameSwapped, window, [window] { window->setOpacity(1); },
        Qt::ConnectionType(Qt::QueuedConnection | Qt::SingleShotConnection));
}
