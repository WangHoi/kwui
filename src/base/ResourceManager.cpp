#include "ResourceManager.h"
#include "EncodingManager.h"
#include "absl/types/span.h"
#ifdef __ANDROID__
#include "ResourceAndroid.h"
#endif
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

namespace base {

static ResourceManager* g_manager = nullptr;

#ifdef _WIN32
static absl::optional<absl::Span<uint8_t>> win32_load_resource(HMODULE hmod, int id)
{
	HRSRC info = FindResourceW(hmod, MAKEINTRESOURCEW(id), L"MYFILE");
	if (!info)
		return absl::nullopt;

	HGLOBAL res = ::LoadResource(hmod, info);

	if (res) {
		LPVOID data = LockResource(res);
		DWORD size = SizeofResource(hmod, info);
		return absl::Span<uint8_t>((uint8_t*)data, size);
	} else {
		return absl::nullopt;
	}

}
#endif
absl::optional<base::ResourceArchive::ResourceItem> ResourceManager::LoadResource(int id) {
#ifdef _WIN32
	absl::optional<absl::Span<uint8_t>> res = win32_load_resource(hmodule_, id);
	if (res.has_value()) {
		base::ResourceArchive::ResourceItem item;
		item.name = L"<" + std::to_wstring(id) + L">";
		item.data = res->data();
		item.size = res->size();
		return item;
	} else {
		return absl::nullopt;
	}
#else
	return absl::nullopt;
#endif
}

absl::optional<base::ResourceArchive::ResourceItem> ResourceManager::loadResource(const wchar_t* path)
{
	if (path[0] == L':')
		++path;
	if (archive_)
		return archive_->findItem(path);
	if (root_dir_.has_value())
		return loadResourceFromFile(root_dir_.value(), path);
	return absl::nullopt;
}

bool ResourceManager::preloadResourceArchive(int id)
{
#ifdef _WIN32
	absl::optional<absl::Span<uint8_t>> res = win32_load_resource(hmodule_, id);
	if (res.has_value()) {
		archive_ = base::ResourceArchive::createFromData(res->data(), res->size());
		return true;
	} else {
		return false;
	}
#else
	return false;
#endif
}

void ResourceManager::setResourceRootDir(const char* dir)
{
	root_dir_.emplace(EncodingManager::UTF8ToWide(dir));
}
void ResourceManager::setResourceRootData(const uint8_t* data, size_t size)
{
	archive_ = base::ResourceArchive::createFromData(data, size);
}
#ifdef __ANDROID__
void ResourceManager::preloadAndroidAssets(JNIEnv* env, jobject asset_manager)
{
	archive_ = base::ResourceAndroid::createFromJava(env, asset_manager);
}
#endif
void ResourceManager::clearCache()
{
	root_dir_cache_.clear();
}

absl::optional<base::ResourceArchive::ResourceItem> ResourceManager::loadResourceFromFile(
	const std::wstring& dir, const wchar_t* name)
{
	std::wstring key = name;
	auto it = root_dir_cache_.find(key);
	if (it != root_dir_cache_.end())
		return it->second;

	std::wstring wpath = dir + name;
	std::string path = EncodingManager::WideToUTF8(wpath);
	FILE* f = fopen(path.c_str(), "rb");
	if (!f)
		return absl::nullopt;
	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, 0, SEEK_SET);
	auto data = (uint8_t*)malloc(size);
	fread(data, 1, size, f);
	base::ResourceArchive::ResourceItem item;
	item.size = size;
	item.data = data;
	item.name = key;
	root_dir_cache_[key] = item;
	return item;
}

#ifdef __ANDROID__
ResourceManager* ResourceManager::createInstance(JNIEnv* env, jobject asset_manager)
#else
ResourceManager* ResourceManager::createInstance()
#endif
{
	if (!g_manager) {
		g_manager = new ResourceManager();
#ifdef __ANDROID__
		g_manager->preloadAndroidAssets(env, asset_manager);
#endif
	}
	return g_manager;
}

ResourceManager* ResourceManager::instance()
{
	return g_manager;
}

void ResourceManager::releaseInstance()
{
	if (g_manager) {
		delete g_manager;
		g_manager = nullptr;
	}
}

ResourceManager::ResourceManager()
{
#ifdef _WIN32
	hmodule_ = ::GetModuleHandleW(NULL);
#endif
}

}
