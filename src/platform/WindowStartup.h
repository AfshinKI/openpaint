// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QCursor>
#include <QGuiApplication>
#include <QPainterPath>
#include <QQuickWindow>
#include <QRegion>
#include <QScreen>

// Match the rounded client surface's input region. Native decorations stay owned
// by the compositor; maximized/fullscreen clients use the whole rectangle.
inline void updateWindowInputRegion(QQuickWindow* window) {
#ifdef Q_OS_LINUX
    if (window->windowState() == Qt::WindowMaximized ||
        window->windowState() == Qt::WindowFullScreen) {
        window->setMask({});
        return;
    }
    QPainterPath path;
    const qreal w = window->width(), h = window->height(), r = 12;
    path.moveTo(0, 0);
    path.lineTo(w, 0);
    path.lineTo(w, h - r);
    path.quadTo(w, h, w - r, h);
    path.lineTo(r, h);
    path.quadTo(0, h, 0, h - r);
    path.closeSubpath();
    window->setMask(QRegion(path.toFillPolygon().toPolygon()));
#else
    Q_UNUSED(window);
#endif
}

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
    updateWindowInputRegion(window);
    QObject::connect(window, &QWindow::widthChanged, window,
                     [window] { updateWindowInputRegion(window); });
    QObject::connect(window, &QWindow::heightChanged, window,
                     [window] { updateWindowInputRegion(window); });
    QObject::connect(window, &QWindow::windowStateChanged, window,
                     [window] { updateWindowInputRegion(window); });
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
