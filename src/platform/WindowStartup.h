// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QCursor>
#include <QGuiApplication>
#include <QQuickWindow>
#include <QScreen>

inline void centerStartupWindow(QQuickWindow* window) {
    if (auto* screen = window->screen()) {
        const auto area = screen->availableGeometry();
        const auto frame = window->frameGeometry().size();
        window->setFramePosition(area.topLeft() +
                                 QPoint(qMax(0, (area.width() - frame.width()) / 2),
                                        qMax(0, (area.height() - frame.height()) / 2)));
    }
}

// Map an invisible native window so Qt can render, then reveal only a submitted
// frame. Keep the opacity change on the GUI thread, not the render thread.
inline void revealAfterFirstFrame(QQuickWindow* window) {
    auto* screen = QGuiApplication::screenAt(QCursor::pos());
    if (!screen)
        screen = QGuiApplication::primaryScreen();
    if (screen)
        window->setScreen(screen);
    centerStartupWindow(window);
    window->setOpacity(0);
    QObject::connect(
        window, &QQuickWindow::frameSwapped, window,
        [window] {
            centerStartupWindow(window);
            window->setOpacity(1);
        },
        Qt::ConnectionType(Qt::QueuedConnection | Qt::SingleShotConnection));
}
