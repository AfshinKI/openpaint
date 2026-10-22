// SPDX-License-Identifier: GPL-3.0-or-later
#include "MacGlass.h"
#include "Appearance.h"
#import <AppKit/AppKit.h>
#include <QEvent>
#include <QGuiApplication>
#include <QPointer>
#include <QQuickWindow>
#include <QScopedValueRollback>
#include <QTimer>

// This view never intercepts drawing, menu or window-control input.
@interface OpenPaintGlassView : NSVisualEffectView
@end
@implementation OpenPaintGlassView
- (NSView*)hitTest:(NSPoint)point {
    (void)point;
    return nil;
}
@end

namespace {
class MacGlass final : public QObject {
  public:
    MacGlass(QQuickWindow* window, Appearance* appearance, std::function<void(bool)> changed)
        : QObject(window), window_(window), appearance_(appearance), changed_(std::move(changed)) {
        connect(appearance, &Appearance::changed, this, [this] { update(); });
        connect(window, &QWindow::widthChanged, this, [this] { update(); });
        connect(window, &QWindow::heightChanged, this, [this] { update(); });
        connect(window, &QWindow::visibleChanged, this, [this] { scheduleUpdate(); });
        window->installEventFilter(this);
        // Observe accessibility changes on the main thread without touching user settings.
        observer_ = [[NSWorkspace sharedWorkspace].notificationCenter
            addObserverForName:NSWorkspaceAccessibilityDisplayOptionsDidChangeNotification
                        object:nil
                         queue:[NSOperationQueue mainQueue]
                    usingBlock:^(NSNotification*) {
                      update();
                    }];
        scheduleUpdate();
    }
    ~MacGlass() override {
        [[NSWorkspace sharedWorkspace].notificationCenter removeObserver:observer_];
        [effect_ removeFromSuperview];
    }

  private:
    bool eventFilter(QObject* watched, QEvent* event) override {
        if (watched == window_ && event->type() == QEvent::Expose)
            scheduleUpdate();
        return QObject::eventFilter(watched, event);
    }
    void scheduleUpdate() {
        QTimer::singleShot(0, this, [this] { update(); });
    }
    void update() {
        // Wait until Qt finishes installing/showing its native content view.
        if (updating_ || !window_ || !appearance_ || !window_->isExposed())
            return;
        QScopedValueRollback<bool> updating(updating_, true);
        NSView* content = (__bridge NSView*)(void*)window_->winId();
        NSWindow* nativeWindow = content.window;
        if (nativeWindow && (!effect_ || effect_.window != nativeWindow)) {
            [effect_ removeFromSuperview];
            // Own a content container instead of modifying AppKit's private frame.
            NSView* container = [[NSView alloc] initWithFrame:content.frame];
            NSView* retainedContent = content;
            [retainedContent removeFromSuperview];
            nativeWindow.contentView = container;
            retainedContent.frame = container.bounds;
            retainedContent.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
            effect_ = [[OpenPaintGlassView alloc] initWithFrame:container.bounds];
            effect_.blendingMode = NSVisualEffectBlendingModeBehindWindow;
            effect_.material = NSVisualEffectMaterialUnderWindowBackground;
            effect_.state = NSVisualEffectStateActive;
            effect_.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
            [container addSubview:effect_];
            [container addSubview:retainedContent];
            nativeWindow.opaque = NO;
            nativeWindow.backgroundColor = [NSColor clearColor];
        }
        const bool attached =
            effect_ && effect_.window == nativeWindow && effect_.superview == content.superview;
        const bool available =
            attached && ![NSWorkspace sharedWorkspace].accessibilityDisplayShouldReduceTransparency;
        if (effect_) {
            effect_.frame = content.frame;
            effect_.hidden =
                !available || !appearance_->glassEnabled() || appearance_->surfaceOpacity() >= 100;
            effect_.appearance = appearance_->themeMode() == 1
                                     ? [NSAppearance appearanceNamed:NSAppearanceNameAqua]
                                 : appearance_->themeMode() == 2
                                     ? [NSAppearance appearanceNamed:NSAppearanceNameDarkAqua]
                                     : nil;
        }
        window_->setProperty("nativeGlassAttached", attached);
        window_->setProperty("nativeGlassAvailable", available);
        if (!reported_ || available != available_) {
            reported_ = true;
            available_ = available;
            changed_(available);
        }
    }
    QPointer<QQuickWindow> window_;
    QPointer<Appearance> appearance_;
    std::function<void(bool)> changed_;
    OpenPaintGlassView* __strong effect_ = nil;
    id __strong observer_ = nil;
    bool available_ = false, reported_ = false, updating_ = false;
};
} // namespace

void installMacGlass(QQuickWindow* window, Appearance* appearance,
                     std::function<void(bool)> availabilityChanged) {
    if (QGuiApplication::platformName() == "cocoa")
        new MacGlass(window, appearance, std::move(availabilityChanged));
}
