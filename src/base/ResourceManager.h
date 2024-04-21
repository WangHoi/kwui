#pragma once

#include "ResourceArchive.h"
#ifdef _WIN32
#include "windows/windows_header.h"
#endif
#include <map>

namespace base {

struct Resource {
    int id;
    const uint8_t* data;
    size_t size;

    Resource() : id(0), data(NULL), size(0) {}
    Resource(int id_, const uint8_t* d, size_t s)
        : id(id_), data(d), size(s) {}
    inline bool IsValid() const { return id > 0; }
};

class ResourceManager {
public:
    absl::optional<base::ResourceArchive::ResourceItem> LoadResource(int id);
    // path must be started with '/', eg. "/xx/aaa.bb"
    absl::optional<base::ResourceArchive::ResourceItem> loadResource(const wchar_t* path);
    bool preloadResourceArchive(int id);
    void setResourceRootDir(const char* dir);
    void setResourceRootData(const uint8_t* data, size_t size);
    void clearCache();

    static ResourceManager* createInstance();
    static ResourceManager* instance();
    static void releaseInstance();

private:
    ResourceManager();
    absl::optional<base::ResourceArchive::ResourceItem> loadResourceFromFile(const std::wstring& dir, const wchar_t* name);

    std::unique_ptr<base::ResourceProviderInterface> archive_;
    std::optional<std::wstring> root_dir_;
    std::map<std::wstring, base::ResourceArchive::ResourceItem> root_dir_cache_;
#ifdef _WIN32
    HMODULE hmodule_;
#endif
};

}
