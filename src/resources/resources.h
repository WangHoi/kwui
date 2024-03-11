#pragma once
#include "absl/types/span.h"
#include "absl/types/optional.h"
#include <stdint.h>

namespace resources {

absl::optional<absl::Span<const uint8_t>> get_module_binary(const char* mod_name);
absl::optional<absl::Span<const uint8_t>> get_image_data(const std::string& image_path);

}
