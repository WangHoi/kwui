#pragma once
#include "log.h"
#include "absl/types/optional.h"
#include <memory>
#include <string>
#include <vector>

namespace base {

class ResourceArchive
{
public:
	// static std::unique_ptr<ResourceArchive> CreateFromFile(const wchar_t* filename);
	static std::unique_ptr<ResourceArchive> CreateFromData(const uint8_t* data, size_t size);

	struct Item {
		std::wstring name;
		const uint8_t* data;
		size_t size;
	};
	size_t GetItemCount() const;
	absl::optional<Item> FindItem(const wchar_t* path) const;

private:
	// bool Init(std::unique_ptr<MmapFile> file);
	bool Init(const uint8_t* data, size_t size);
	void IterateNodes(int16_t idx, std::wstring& name, bool& valid);

	struct Node {
		wchar_t splitchar;
		int16_t lokid, eqkid, hikid;
		Node()
			: splitchar(0), lokid(-1), eqkid(-1), hikid(-1) {}
		int16_t value() const {
			CHECK(splitchar == 0);
			return eqkid;
		}
		void value(int16_t v) {
			CHECK(splitchar == 0);
			eqkid = v;
		}
	};
	struct ItemLocation {
		off_t offset;
		size_t length;
		size_t ulength; // = 0, raw data
	};
	//std::unique_ptr<MmapFile> _mmap_file;
	std::vector<Node> _nodes;
	std::vector<ItemLocation> _item_locations;
	std::vector<Item> _items;
	std::vector<std::vector<uint8_t>> _udata; // uncompressed data
};

}
