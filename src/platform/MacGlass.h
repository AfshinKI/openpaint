// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <functional>
class QQuickWindow;
class Appearance;
// GUI-thread Cocoa bridge. No desktop capture or external setup.
void installMacGlass(QQuickWindow* window, Appearance* appearance,
                     std::function<void(bool)> availabilityChanged);
