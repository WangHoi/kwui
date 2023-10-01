#pragma once

#include "absl/types/variant.h"
#include <string>

namespace scene2d {

typedef absl::variant<float, std::string> NodeAttributeValue;

}