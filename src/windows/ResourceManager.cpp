#include "ResourceManager.h"
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

absl::optional<base::ResourceArchive::Item> ResourceManager::LoadResource(int id) {
    absl::optional<absl::Span<uint8_t>> res = win32_load_resource(_hmodule, id);
    if (res.has_value()) {
        base::ResourceArchive::Item item;
        item.name = L"<" + std::to_wstring(id) + L">";
        item.data = res->data();
        item.size = res->size();
        return item;
    } else {
        return absl::nullopt;
    }
}

absl::optional<base::ResourceArchive::Item> ResourceManager::LoadResource(const wchar_t* name)
{
    if (_archive)
        return _archive->FindItem(name);
    return absl::nullopt;
}

bool ResourceManager::preloadResourceArchive(int id)
{
    absl::optional<absl::Span<uint8_t>> res = win32_load_resource(_hmodule, id);
    if (res.has_value()) {
        _archive = base::ResourceArchive::CreateFromData(res->data(), res->size());
        return true;
    } else {
        return false;
    }
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
