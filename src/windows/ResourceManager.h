#pragma once

#include "base/ResourceArchive.h"
#include "windows_header.h"

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
    absl::optional<base::ResourceArchive::ResourceItem> LoadResource(const wchar_t* name);
    bool preloadResourceArchive(int id);
    
    static ResourceManager* createInstance(HMODULE hModule);
    static ResourceManager* instance();
    static void releaseInstance();

private:
    ResourceManager(HMODULE hModule) : _hmodule(hModule) {}

    std::unique_ptr<base::ResourceArchive> _archive;
    HMODULE _hmodule;
};

}
