#include "ResourceAndroid.h"
#include "EncodingManager.h"
#include "base/log.h"
#include <stdlib.h>

namespace base {

std::unique_ptr<ResourceAndroid> ResourceAndroid::createFromJava(JNIEnv* env, jobject asset_manager)
{
	auto m = std::make_unique<ResourceAndroid>();
	m->init(env, asset_manager);
	return m;
}

ResourceAndroid::~ResourceAndroid()
{
	for (auto& p : cache_) {
		free((void*)p.second.data);
	}
	cache_.clear();
}

absl::optional<ResourceProviderInterface::ResourceItem> ResourceAndroid::findItem(const wchar_t* path) const
{
	if (path && path[0] == L'/')
		++path;
	std::wstring key(path);
	auto it = cache_.find(path);
	if (it != cache_.end())
		return it->second;

	auto filename = EncodingManager::WideToUTF8(path);
	AAsset* f = AAssetManager_open(manager_, filename.c_str(), AASSET_MODE_BUFFER);
	if (!f) {
		return absl::nullopt;
	}
	ResourceProviderInterface::ResourceItem item;
	item.size = AAsset_getLength(f);
	auto data = malloc(item.size);
	memcpy(data, AAsset_getBuffer(f), item.size);
	AAsset_close(f);
	item.data = (const uint8_t*)data;
	item.name = key;
	cache_[key] = item;
	return item;
}

void ResourceAndroid::init(JNIEnv* env, jobject asset_manager)
{
	manager_ = AAssetManager_fromJava(env, asset_manager);
	LOG(INFO) << "AAssetManager_fromJava return " << manager_;
}

}