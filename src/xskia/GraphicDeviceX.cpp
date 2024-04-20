#include "GraphicDeviceX.h"
#include "include/core/SkStream.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkData.h"
#ifdef _WIN32
#include "include/ports/SkImageGeneratorWIC.h"
#endif
#ifdef __ANDROID__
#include "include/ports/SkImageGeneratorNDK.h"
#endif
#include "absl/strings/match.h"
#include "resources/resources.h"
#include "base/ResourceManager.h"
#include "base/EncodingManager.h"

namespace xskia {

static GraphicDeviceX* s_instance = nullptr;

GraphicDeviceX::~GraphicDeviceX()
{}
GraphicDeviceX* GraphicDeviceX::createInstance()
{
	if (!s_instance) {
		s_instance = new GraphicDeviceX();
		SkGraphics::Init();
#ifdef _WIN32
		SkGraphics::SetImageGeneratorFromEncodedDataFactory(&SkImageGeneratorWIC::MakeFromEncodedWIC);
#endif
#ifdef __ANDROID__
		SkGraphics::SetImageGeneratorFromEncodedDataFactory(&SkImageGeneratorNDK::MakeFromEncodedNDK);
#endif
	}
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
		font_cache_[family_name] = font_face;
}
sk_sp<SkTypeface> GraphicDeviceX::getFirstMatchingFontFace(
	const char* family_name,
	SkFontStyle style)
{
	if (family_name) {
		auto it = font_cache_.find(family_name);
		if (it != font_cache_.end())
			return it->second;
	}
	return SkTypeface::MakeFromName(family_name, style);
}
BitmapSubItemX GraphicDeviceX::getBitmap(const std::string& name, float dpi_scale)
{
	auto it = bitmap_cache_.find(name);
	if (it == bitmap_cache_.end()) {
		loadBitmapToCache(name);
		it = bitmap_cache_.find(name);
	}
	if (it == bitmap_cache_.end())
		return BitmapSubItemX();
	BitmapSubItemX res;
	res.frame = it->second;
	res.dpi_scale = dpi_scale;
	return res;
}
void GraphicDeviceX::loadBitmapToCache(const std::string& name)
{
	if (absl::StartsWith(name, "kwui::")) {
		std::string name_res = name.substr(6);
		absl::optional<absl::Span<const uint8_t>> x1;
		x1 = resources::get_image_data(name_res);
		if (!x1.has_value())
			return;
		loadBitmapFromResource(name, x1.value());
	} else if (absl::StartsWith(name, ":")) {
		auto RM = base::ResourceManager::instance();
		std::string name_res = name.substr(1);
		absl::optional<base::ResourceArchive::ResourceItem> x1, x1_5, x2;
		x1 = RM->loadResource(base::EncodingManager::UTF8ToWide(name_res).c_str());
		if (!x1.has_value())
			return;
		loadBitmapFromResource(name, absl::MakeSpan(x1->data, x1->size));
	} else {
		FILE* f = nullptr;
#ifdef _WIN32
		auto filename_x1 = base::EncodingManager::UTF8ToWide(name);
		f = _wfopen(filename_x1.c_str(), L"rb");
#else
		f = fopen(name.c_str(), "rb");
#endif
		if (f) {
			sk_sp<SkImage> image = SkImage::MakeFromEncoded(SkData::MakeFromFILE(f));
			if (image) {
				bitmap_cache_[name] = image;
			}
			fclose(f);
		}
	}
}
void GraphicDeviceX::loadBitmapFromResource(const std::string& name, absl::Span<const uint8_t> res_x1) {
	sk_sp<SkImage> image = SkImage::MakeFromEncoded(SkData::MakeWithoutCopy(res_x1.data(), res_x1.size()));
	if (image) {
		bitmap_cache_[name] = image;
	}
}
}
