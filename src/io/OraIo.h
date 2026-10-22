// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "PngIo.h"
namespace openpaint {
ImageResult loadOra(const QString& path, const Cancellation& token = {});
ImageResult saveOra(const QString& path, const Layers& layers, const Cancellation& token = {},
                    const std::function<bool(QSaveFile&)>& commit = {});
} // namespace openpaint
