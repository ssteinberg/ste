//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <list>

namespace ste {
namespace gl {

template <typename T>
class device_memory_block {
private:
	T offset;
	T bytes;

public:
	device_memory_block() = default;
	device_memory_block(T offset, 
						T bytes)
		: offset(offset), bytes(bytes) {}
	~device_memory_block() noexcept {}

	device_memory_block(device_memory_block &&) = default;
	device_memory_block(const device_memory_block &) = default;
	device_memory_block &operator=(device_memory_block &&) = default;
	device_memory_block &operator=(const device_memory_block &) = default;

	bool operator<(const device_memory_block &b) const { return offset < b.offset; }
	bool operator==(const device_memory_block &b) const { return offset == b.offset; }

	auto get_offset() const { return offset; }
	auto get_bytes() const { return bytes; }
};

}
}
