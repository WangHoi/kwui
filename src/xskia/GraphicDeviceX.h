#pragma once
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkImage.h"
#include "absl/types/span.h"
#include <unordered_map>
#include <string>
#ifdef _WIN32
#include "src/utils/win/SkWGL.h"
#endif

namespace xskia {

struct BitmapSubItemX {
	sk_sp<SkImage> frame;
	float dpi_scale;
	operator bool() const {
		return frame != nullptr;
	}

};
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
	BitmapSubItemX getBitmap(const std::string& url, float dpi_scale);
#ifdef _WIN32
	HGLRC rootGLContext() const
	{
		return root_context_ ? root_context_->getGLRC() : nullptr;
	}
#endif

private:
	void loadBitmapToCache(const std::string& name);
	void loadBitmapFromResource(const std::string& name, absl::Span<const uint8_t> res_x1);

	std::unordered_map<std::string, sk_sp<SkTypeface>> font_cache_;
	std::unordered_map<std::string, sk_sp<SkImage>> bitmap_cache_;
#ifdef _WIN32
	sk_sp<SkWGLPbufferContext> root_context_;
#endif
};

}