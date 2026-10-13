// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "PngIo.h"
namespace openpaint {
ImageResult loadImage(const QString& path, const Cancellation& cancellation = {});
} // namespace openpaint
