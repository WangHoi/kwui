#include "resources.h"
#include "absl/base/macros.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

extern "C" {
	extern const uint32_t qjsc_Keact_size;
	extern const uint8_t qjsc_Keact[];
}

const uint8_t vscroll_top_button_png[] = {
#include "vscroll_top_button_png.txt"
};
const uint8_t vscroll_bottom_button_png[] = {
#include "vscroll_bottom_button_png.txt"
};
const uint8_t hscroll_left_button_png[] = {
#include "hscroll_left_button_png.txt"
};
const uint8_t hscroll_right_button_png[] = {
#include "hscroll_right_button_png.txt"
};

namespace resources {

absl::optional<absl::Span<const uint8_t>> get_module_binary(const char* mod_name)
{
	if (strcmp(mod_name, "Keact") == 0) {
		return absl::Span<const uint8_t>((const uint8_t*)qjsc_Keact, (size_t)qjsc_Keact_size);
	}
	return absl::nullopt;
}

absl::optional<absl::Span<const uint8_t>> get_image_data(const std::string& image_path)
{
	if (image_path == "vscroll_top_button.png") {
		return absl::Span<const uint8_t>(vscroll_top_button_png, ABSL_ARRAYSIZE(vscroll_top_button_png));
	} else if (image_path == "vscroll_bottom_button.png") {
		return absl::Span<const uint8_t>(vscroll_bottom_button_png, ABSL_ARRAYSIZE(vscroll_bottom_button_png));
	} else if (image_path == "hscroll_left_button.png") {
		return absl::Span<const uint8_t>(hscroll_left_button_png, ABSL_ARRAYSIZE(hscroll_left_button_png));
	} else if (image_path == "hscroll_right_button.png") {
		return absl::Span<const uint8_t>(hscroll_right_button_png, ABSL_ARRAYSIZE(hscroll_right_button_png));
	}
	return absl::nullopt;
}

}
