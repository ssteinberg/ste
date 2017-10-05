//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

namespace ste {
namespace gl {

class device_memory_block {
private:
	byte_t offset;
	byte_t bytes;

public:
	device_memory_block() = default;
	device_memory_block(byte_t offset,
						byte_t bytes)
		: offset(offset), bytes(bytes) {}
	~device_memory_block() noexcept {}

	device_memory_block(device_memory_block &&) = default;
	device_memory_block(const device_memory_block &) = default;
	device_memory_block &operator=(device_memory_block &&) = default;
	device_memory_block &operator=(const device_memory_block &) = default;

	// For ordered containers
	bool operator<(const device_memory_block &b) const { return offset < b.offset; }
	bool operator==(const device_memory_block &b) const { return offset == b.offset; }

	auto get_offset() const { return offset; }
	auto get_bytes() const { return bytes; }
};

}
}
