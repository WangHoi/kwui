#include "GraphicDeviceX.h"
#include "include/core/SkStream.h"
#include "include/core/SkFontMgr.h"

namespace xskia {

static GraphicDeviceX* s_instance = nullptr;

GraphicDeviceX::~GraphicDeviceX()
{}
GraphicDeviceX* GraphicDeviceX::createInstance()
{
	if (!s_instance)
		s_instance = new GraphicDeviceX();
	return s_instance;
}
void GraphicDeviceX::releaseInstance()
{
	if (s_instance) {
		delete s_instance;
		s_instance = nullptr;
	}
}
GraphicDeviceX* GraphicDeviceX::instance()
{
	return s_instance;
}
void GraphicDeviceX::addFont(const char* family_name, const void* data, size_t size)
{
	sk_sp<SkTypeface> font_face = SkFontMgr::RefDefault()
		->makeFromStream(SkMemoryStream::MakeCopy(data, size));
	if (font_face)
		cache_fonts_[family_name] = font_face;
}
sk_sp<SkTypeface> GraphicDeviceX::getFirstMatchingFontFace(
	const char* family_name,
	SkFontStyle style)
{
	if (family_name) {
		auto it = cache_fonts_.find(family_name);
		if (it != cache_fonts_.end())
			return it->second;
	}
	return SkTypeface::MakeFromName(family_name, style);
}

}