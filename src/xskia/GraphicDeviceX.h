#pragma once
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include <unordered_map>
#include <string>

namespace xskia {

class GraphicDeviceX {
public:
	~GraphicDeviceX();
	static GraphicDeviceX* createInstance();
	static void releaseInstance();
	static GraphicDeviceX* instance();
	void addFont(const char* family_name, const void* data, size_t size);
	// Map font and style to fontFace.
	sk_sp<SkTypeface> getFirstMatchingFontFace(
		const char* family_name,
		SkFontStyle style);

private:
	std::unordered_map<std::string, sk_sp<SkTypeface>> cache_fonts_;
};

}