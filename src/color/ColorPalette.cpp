// SPDX-License-Identifier: GPL-3.0-or-later
#include "ColorPalette.h"

bool ColorPalette::fail(const QString& error) {
    error_ = error;
    emit errorChanged();
    return false;
}
bool ColorPalette::write(const QStringList& colors) {
    colors_ = colors;
    error_.clear();
    emit errorChanged();
    emit changed();
    return true;
}
bool ColorPalette::writeToolbar(const QStringList& colors) {
    toolbarColors_ = colors;
    emit changed();
    return true;
}
bool ColorPalette::add(QColor color) {
    if (!color.isValid() || colors_.size() >= 24)
        return fail(
            tr("Choose a valid color. The editor holds up to 24 custom colors this session."));
    auto next = colors_;
    next.append(color.name());
    return write(next);
}
bool ColorPalette::replace(int index, QColor color) {
    if (!color.isValid() || index < 0 || index >= colors_.size())
        return false;
    auto next = colors_;
    next[index] = color.name();
    return write(next);
}
bool ColorPalette::remove(int index) {
    if (index < 0 || index >= colors_.size())
        return false;
    auto next = colors_;
    next.removeAt(index);
    return write(next);
}
bool ColorPalette::clear() {
    return write({});
}
bool ColorPalette::setToolbarColor(int index, QColor color) {
    if (index < 0 || index >= 10 || !color.isValid())
        return false;
    auto next = toolbarColors_;
    next[index] = color.name();
    return writeToolbar(next);
}
bool ColorPalette::remember(QColor color, int replacementSlot) {
    if (!color.isValid() || replacementSlot < -1 || replacementSlot >= 10)
        return false;
    const auto name = color.name();
    auto toolbar = toolbarColors_;
    auto library = colors_;
    if (replacementSlot >= 0)
        toolbar[replacementSlot] = name;
    else if (!toolbar.contains(name)) {
        const auto empty = toolbar.indexOf(QString());
        if (empty >= 0)
            toolbar[empty] = name;
    }
    if (!library.contains(name) && library.size() < 24)
        library.append(name);
    toolbarColors_ = toolbar;
    return write(library);
}
