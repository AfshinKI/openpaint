// SPDX-License-Identifier: GPL-3.0-or-later
#include "MacGlass.h"
#include "Appearance.h"
#import <AppKit/AppKit.h>
#include <QGuiApplication>
#include <QPointer>
#include <QQuickWindow>
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
        connect(window, &QWindow::visibleChanged, this, [this] { update(); });
        // Observe accessibility changes on the main thread without touching user settings.
        observer_ = [[NSWorkspace sharedWorkspace].notificationCenter
            addObserverForName:NSWorkspaceAccessibilityDisplayOptionsDidChangeNotification
                        object:nil
                         queue:[NSOperationQueue mainQueue]
                    usingBlock:^(NSNotification*) {
                      update();
                    }];
        QTimer::singleShot(0, this, [this] { update(); });
    }
    ~MacGlass() override {
        [[NSWorkspace sharedWorkspace].notificationCenter removeObserver:observer_];
        [effect_ removeFromSuperview];
    }

  private:
    void update() {
        if (!window_ || !appearance_)
            return;
        NSView* content = (__bridge NSView*)(void*)window_->winId();
        NSView* parent = content.superview;
        if (parent && !effect_) {
            effect_ = [[OpenPaintGlassView alloc] initWithFrame:content.frame];
            effect_.blendingMode = NSVisualEffectBlendingModeBehindWindow;
            effect_.material = NSVisualEffectMaterialUnderWindowBackground;
            effect_.state = NSVisualEffectStateActive;
            effect_.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
            [parent addSubview:effect_ positioned:NSWindowBelow relativeTo:content];
            content.window.opaque = NO;
            content.window.backgroundColor = [NSColor clearColor];
        }
        const bool available =
            effect_ && ![NSWorkspace sharedWorkspace].accessibilityDisplayShouldReduceTransparency;
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
    bool available_ = false, reported_ = false;
};
} // namespace

void installMacGlass(QQuickWindow* window, Appearance* appearance,
                     std::function<void(bool)> availabilityChanged) {
    if (QGuiApplication::platformName() == "cocoa")
        new MacGlass(window, appearance, std::move(availabilityChanged));
}
