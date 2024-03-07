#include "ResourceManager.h"
#include "EncodingManager.h"
#include "absl/types/span.h"

namespace windows {

static ResourceManager* g_manager = nullptr;

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

absl::optional<base::ResourceArchive::ResourceItem> ResourceManager::LoadResource(int id) {
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
}

absl::optional<base::ResourceArchive::ResourceItem> ResourceManager::loadResource(const wchar_t* name)
{
	if (archive_)
		return archive_->FindItem(name);
	if (root_dir_.has_value())
		return loadResourceFromFile(root_dir_.value(), name);
	return absl::nullopt;
}

bool ResourceManager::preloadResourceArchive(int id)
{
	absl::optional<absl::Span<uint8_t>> res = win32_load_resource(hmodule_, id);
	if (res.has_value()) {
		archive_ = base::ResourceArchive::CreateFromData(res->data(), res->size());
		return true;
	} else {
		return false;
	}
}

void ResourceManager::setResourceRootDir(const char* dir)
{
	root_dir_.emplace(EncodingManager::UTF8ToWide(dir));
}

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

	std::wstring path = dir + name;
	FILE* f = _wfopen(path.c_str(), L"rb");
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

ResourceManager* ResourceManager::createInstance(HMODULE hModule)
{
	if (!g_manager)
		g_manager = new ResourceManager(hModule);
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

}
