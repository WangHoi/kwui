#pragma once
#include "api/kwui/Application.h"
#include "absl/log/log.h"
#include "absl/log/check.h"
#include "absl/types/optional.h"

namespace base {
void initialize_log(kwui::LogCallback func = nullptr);
}
