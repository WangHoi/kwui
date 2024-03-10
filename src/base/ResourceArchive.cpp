#include "ResourceArchive.h"
#include "algorithm/lzf.h"
#include "absl/base/internal/endian.h"
#include "windows/EncodingManager.h"
#include <wchar.h>

namespace base {

namespace {
enum AlgorithmType {
	Store,
	Lzf,
	Lzms,
};
}

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

ResourceArchive::~ResourceArchive()
{
	Reset();
}

std::unique_ptr<ResourceArchive> ResourceArchive::CreateFromData(const uint8_t* data, size_t size)
{
	auto arc = std::make_unique<ResourceArchive>();
	if (!arc->Init(data, size)) {
		LOG(ERROR) << "ResourceArchive: init from data size " << size << " failed";
		return nullptr;
	}
	return arc;
}

size_t ResourceArchive::GetItemCount() const
{
	return _items.size();
}

absl::optional<ResourceArchive::ResourceItem> ResourceArchive::FindItem(const wchar_t* path) const
{
	const wchar_t* p = path;
	int16_t idx = 0;
	while ((size_t)idx < _nodes.size()) {
		const auto& n = _nodes[idx];
		if (*p < n.splitchar) {
			idx = n.lokid;
		} else if (*p > n.splitchar) {
			idx = n.hikid;
		} else {
			if (*p++ == 0) {
				const Item& item = _items[n.eqkid];
				const Chunk& chunk = _chunk_map.find(item.offset)->second;
				ResourceItem ret;
				ret.name = path;
				ret.data = chunk.data;
				ret.size = item.length;
				return ret;
			}
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
	if (size < 24) {
		LOG(WARNING) << "Invalid data length.";
		return false;
	}
	const uint8_t* const data_end = data + size;
	const uint8_t* p = data;
	if (p[0] != 'K' || p[1] != 'A' || p[2] != 'r' || p[3] != ' ') {
		LOG(WARNING) << "Invalid header magic";
		return false;
	}
	p += 4;
	uint16_t version = absl::little_endian::Load16(p);
	p += 2;
	uint16_t flags = absl::little_endian::Load16(p);
	p += 2;
	p += 12; // ChunkSize, DirCount, FileCount
	uint32_t num_nodes = absl::little_endian::Load32(p);
	p += 4;
	if (p + 8 * num_nodes > data_end) {
		LOG(WARNING) << "Invalid data length.";
		return false;
	}
	_nodes.resize(num_nodes);
	for (uint32_t i = 0; i < num_nodes; ++i) {
		Node& n = _nodes[i];
		n.splitchar = absl::little_endian::Load16(p);
		n.lokid = absl::little_endian::Load16(p + 2);
		n.eqkid = absl::little_endian::Load16(p + 4);
		n.hikid = absl::little_endian::Load16(p + 6);
		p += 8;
	}

	if (p + 4 > data_end) {
		LOG(WARNING) << "Invalid data length.";
		return false;
	}
	uint32_t num_items = absl::little_endian::Load32(p);
	p += 4;
	if (p + 12 * num_items > data_end) {
		LOG(WARNING) << "Invalid data length.";
		return false;
	}
	_items.resize(num_items);
	for (uint32_t i = 0; i < num_items; ++i) {
		Item& item = _items[i];
		item.reference = absl::little_endian::Load16(p);
		item.flags = absl::little_endian::Load16(p + 2);
		item.offset = absl::little_endian::Load32(p + 4);
		item.length = absl::little_endian::Load32(p + 8);
		p += 12;
	}

	if (p + 4 > data_end) {
		LOG(WARNING) << "Invalid data length.";
		return false;
	}
	uint32_t num_chunks = absl::little_endian::Load32(p);
	p += 4;
	if (p + 12 * num_chunks > data_end) {
		LOG(WARNING) << "Invalid data length.";
		return false;
	}
	off_t compressed_offset = 0;
	off_t uncompressed_offset = 0;
	for (uint32_t i = 0; i < num_chunks; ++i) {
		Chunk chunk;
		chunk.algorithm = absl::little_endian::Load16(p);
		chunk.flags = absl::little_endian::Load16(p + 2);
		chunk.length = absl::little_endian::Load32(p + 4);
		chunk.compressed_length = absl::little_endian::Load32(p + 8);
		chunk.data = nullptr;
		_chunk_map[uncompressed_offset] = chunk;
		compressed_offset += chunk.compressed_length;
		uncompressed_offset += chunk.length;
		p += 12;
	}

	if (p + compressed_offset > data_end) {
		LOG(WARNING) << "Invalid data length.";
		return false;
	}
	
	// Decompress data
	compressed_offset = 0;
	uncompressed_offset = 0;
	for (auto& chunk_pair : _chunk_map) {
		uncompressed_offset = chunk_pair.first;
		Chunk& chunk = chunk_pair.second;
		if (chunk.algorithm == AlgorithmType::Store) {
			chunk.data = p + compressed_offset;
		} else if (chunk.algorithm == AlgorithmType::Lzf) {
			uint8_t* buf = (uint8_t*)malloc(chunk.length);
			if (buf) {
				unsigned ret = lzf_decompress(p + compressed_offset, chunk.compressed_length, buf, chunk.length);
				if (ret != chunk.length) {
					LOG(WARNING) << "Invalid compress data.";
					return false;
				}
			}
			chunk.data = buf;
		} else {
			LOG(WARNING) << "Invalid compress algorithm.";
			return false;
		}
		compressed_offset += chunk.compressed_length;
	}

	bool valid = true;
	// Validate item's chunk offset
	for (size_t i = 1; i < _items.size(); ++i) {
		auto it = _chunk_map.find(_items[i].offset);
		if (it == _chunk_map.end()) {
			LOG(WARNING) << "Invalid item offset.";
			valid = false;
			break;
		}
	}
	// Validate node's index
	if (valid)
		IterateNodes(0, std::wstring(), valid);
	return valid;
}

void ResourceArchive::Reset()
{
	for (auto& chunk_pair : _chunk_map) {
		Chunk& chunk = chunk_pair.second;
		if (chunk.algorithm != AlgorithmType::Store) {
			free(const_cast<uint8_t*>(chunk.data));
		}
	}
	_nodes.clear();
	_items.clear();
	_chunk_map.clear();
}

void ResourceArchive::IterateNodes(int16_t idx, std::wstring& name, bool& valid)
{
	if (!valid)
		return;
	if ((size_t)idx >= _nodes.size())
		return;

	const auto& n = _nodes.at(idx);
	if (!n.splitchar) {
		if (n.eqkid >= _items.size()) {
			LOG(WARNING) << "Invalid node data index.";
			valid = false;
			return;
		}
		auto& item = _items[n.eqkid];
		LOG(INFO) << "Loaded resource [" << windows::EncodingManager::WideToUTF8(name) << "]";
	}
	IterateNodes(n.lokid, name, valid);
	if (n.splitchar) {
		name.push_back(n.splitchar);
		IterateNodes(n.eqkid, name, valid);
		name.pop_back();
	}
	IterateNodes(n.hikid, name, valid);
}

}
