#pragma once
#include "ResourceProviderInterface.h"
#include "log.h"
#include "absl/types/optional.h"
#include <memory>
#include <string>
#include <vector>
#include <map>

namespace base {

class ResourceArchive : public ResourceProviderInterface
{
public:
	~ResourceArchive();

	// static std::unique_ptr<ResourceArchive> CreateFromFile(const wchar_t* filename);
	static std::unique_ptr<ResourceProviderInterface> createFromData(const uint8_t* data, size_t size);

	absl::optional<ResourceItem> findItem(const wchar_t* path) const override;
	size_t getItemCount() const;

private:
	// bool Init(std::unique_ptr<MmapFile> file);
	bool Init(const uint8_t* data, size_t size);
	void Reset();
	void IterateNodes(int16_t idx, std::wstring& name, bool& valid);

	struct Node {
		wchar_t splitchar;
		uint16_t lokid, eqkid, hikid;
		Node()
			: splitchar(0), lokid(-1), eqkid(-1), hikid(-1) {}
		uint16_t value() const {
			CHECK(splitchar == 0);
			return eqkid;
		}
		void value(uint16_t v) {
			CHECK(splitchar == 0);
			eqkid = v;
		}
	};
	struct Item {
		size_t reference = 0;
		uint16_t flags = 0;
		off_t offset = 0;
		size_t length = 0;
	};
	struct Chunk {
		uint16_t algorithm = 0;
		uint16_t flags = 0;
		size_t length = 0;
		size_t compressed_length = 0;
		const uint8_t* data = nullptr;
	};
	//std::unique_ptr<MmapFile> _mmap_file;
	std::vector<Node> _nodes;
	std::vector<Item> _items;
	std::map<off_t, Chunk> _chunk_map;
};

}
