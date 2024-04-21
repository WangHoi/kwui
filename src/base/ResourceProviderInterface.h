#pragma once
#include "log.h"
#include "absl/types/optional.h"
#include <memory>
#include <string>
#include <vector>
#include <map>

namespace base {

class ResourceProviderInterface
{
public:
	struct ResourceItem {
		std::wstring name;
		const uint8_t* data;
		size_t size;
	};
	virtual ~ResourceProviderInterface() {}
	virtual absl::optional<ResourceItem> findItem(const wchar_t* path) const = 0;
};

}
