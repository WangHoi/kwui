#pragma once

#include "base/ResourceArchive.h"
#include "windows_header.h"
#include <map>

namespace windows {

struct Resource {
    int id;
    const byte* data;
    size_t size;

    Resource() : id(0), data(NULL), size(0) {}
    Resource(int id_, const byte* d, size_t s)
        : id(id_), data(d), size(s) {}
    inline bool IsValid() const { return id > 0; }
};

class ResourceManager {
public:
    absl::optional<base::ResourceArchive::ResourceItem> LoadResource(int id);
    absl::optional<base::ResourceArchive::ResourceItem> loadResource(const wchar_t* name);
    bool preloadResourceArchive(int id);
    void setResourceRootDir(const char* dir);
    void clearCache();

    static ResourceManager* createInstance(HMODULE hModule);
    static ResourceManager* instance();
    static void releaseInstance();

private:
    ResourceManager(HMODULE hModule) : hmodule_(hModule) {}
    absl::optional<base::ResourceArchive::ResourceItem> loadResourceFromFile(const std::wstring& dir, const wchar_t* name);

    std::unique_ptr<base::ResourceArchive> archive_;
    std::optional<std::wstring> root_dir_;
    std::map<std::wstring, base::ResourceArchive::ResourceItem> root_dir_cache_;
    HMODULE hmodule_;
};

}
