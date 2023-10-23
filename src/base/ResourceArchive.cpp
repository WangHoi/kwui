#include "ResourceArchive.h"
#include "algorithm/lzf.h"
#include "absl/base/internal/endian.h"
#include "windows/EncodingManager.h"
#include <wchar.h>

namespace base {

/*
std::unique_ptr<ResourceArchive> ResourceArchive::CreateFromFile(const wchar_t* filename)
{
	auto EM = EncodingManager::Instance();
	auto mm_file = std::make_unique<MmapFile>();
	if (!mm_file->Open(filename)) {
		c2_log("ResourceArchive: open file [%s] failed\n", EM->WideToUTF8(filename).GetCString());
		return nullptr;
	}
	auto arc = std::make_unique<ResourceArchive>();
	if (!arc->Init(move(mm_file))) {
		c2_log("ResourceArchive: init from file [%s] failed\n", EM->WideToUTF8(filename).GetCString());
		return nullptr;
	}
	return arc;
}
*/

std::unique_ptr<ResourceArchive> ResourceArchive::CreateFromData(const uint8_t* data, size_t size)
{
	auto arc = std::make_unique<ResourceArchive>();
	if (!arc->Init(data, size)) {
		LOG(INFO) << "ResourceArchive: init from data size " << size << " failed";
		return nullptr;
	}
	return arc;
}

size_t ResourceArchive::GetItemCount() const
{
	return _items.size();
}

absl::optional<ResourceArchive::Item> ResourceArchive::FindItem(const wchar_t* path) const
{
	const wchar_t* p = path;
	int16_t idx = 0;
	while ((size_t)idx < _nodes.size()) {
		const auto& n = _nodes[idx];
		wchar_t ch = towlower(*p);
		if (ch < n.splitchar) {
			idx = n.lokid;
		} else if (ch > n.splitchar) {
			idx = n.hikid;
		} else {
			if (*p++ == 0)
				return _items[n.value() - 1];
			idx = n.eqkid;
		}
	}
	return absl::nullopt;
}
/*
bool ResourceArchive::Init(std::unique_ptr<MmapFile> file)
{
	_mmap_file = move(file);
	return Init(_mmap_file->GetData(), _mmap_file->GetSize());
}
*/
bool ResourceArchive::Init(const uint8_t* data, size_t size)
{
	if (size < 8) {
		LOG(WARNING) << "Invalid data length.";
		return false;
	}
	const uint8_t* p = data;
	if (p[0] != 'S' || p[1] != 'A' || p[2] != 'r' || p[3] != '\0') {
		LOG(WARNING) << "Invalid header magic";
		return false;
	}
	p += 4;
	int32_t num_nodes = absl::little_endian::Load32(p);
	p += 4;
	if (size < 8 + 8 * (size_t)num_nodes + 4) {
		LOG(WARNING) << "Invalid data length.";
		return false;
	}
	_nodes.resize(num_nodes);
	for (int32_t i = 0; i < num_nodes; ++i) {
		Node& n = _nodes[i];
		n.splitchar = absl::little_endian::Load16(p);
		n.lokid = absl::little_endian::Load16(p + 2);
		n.eqkid = absl::little_endian::Load16(p + 4);
		n.hikid = absl::little_endian::Load16(p + 6);
		p += 8;
	}

	int32_t num_items = absl::little_endian::Load32(p);
	p += 4;
	if (size < 8 + 8 * (size_t)num_nodes + 4 + 12 * (size_t)num_items) {
		LOG(WARNING) << "Invalid data length.";
		return false;
	}
	_item_locations.resize(num_items);
	_items.resize(num_items);
	_udata.resize(num_items);
	for (int32_t i = 0; i < num_items; ++i) {
		ItemLocation& loc = _item_locations[i];
		loc.offset = absl::little_endian::Load32(p);
		loc.length = absl::little_endian::Load32(p + 4);
		loc.ulength = absl::little_endian::Load32(p + 8);
		p += 12;

		if ((size_t)loc.offset > size || (size_t)(loc.offset + loc.length) > size) {
			LOG(WARNING) << "Data offset overflow.";
			return false;
		}

		Item& item = _items[i];
		if (loc.ulength) {
			auto& u = _udata[i];
			u.resize(loc.ulength);
			unsigned int olen = lzf_decompress(data + loc.offset, loc.length, u.data(), loc.ulength);
			if (olen != loc.ulength) {
				LOG(WARNING) << "Decompressed size mismatch.";
				return false;
			}
			item.data = u.data();
			item.size = loc.ulength;
		} else {
			item.data = data + loc.offset;
			item.size = loc.length;
		}
	}

	bool valid = true;
	IterateNodes(0, std::wstring(), valid);
	return valid;
}

void ResourceArchive::IterateNodes(int16_t idx, std::wstring& name, bool& valid)
{
	if (!valid)
		return;
	if (idx < 0 || (size_t)idx >= _nodes.size())
		return;

	const auto& n = _nodes.at(idx);
	if (!n.splitchar) {
		if (n.eqkid <= 0 || (size_t)n.eqkid > _items.size()) {
			LOG(WARNING) << "Invalid node data index.";
			valid = false;
			return;
		}
		auto& item = _items[n.eqkid - 1];
		item.name = name;
		LOG(INFO) << "Loaded resource [" << windows::EncodingManager::WideToUTF8(name) << "]";
	} else {
		IterateNodes(n.lokid, name, valid);
		name.push_back(n.splitchar);
		IterateNodes(n.eqkid, name, valid);
		name.pop_back();
		IterateNodes(n.hikid, name, valid);
	}
}

}
