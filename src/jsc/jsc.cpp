#include "jsc.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

extern "C" {
	extern const uint32_t qjsc_Keact_size;
	extern const uint8_t qjsc_Keact[];
}

namespace jsc {

absl::optional<absl::Span<const uint8_t>> get_module_binary(const char* mod_name)
{
	if (strcmp(mod_name, "Keact") == 0) {
		return absl::Span<const uint8_t>((const uint8_t*)qjsc_Keact, (size_t)qjsc_Keact_size);
	}
	return absl::nullopt;
}

}
