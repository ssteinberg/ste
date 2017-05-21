//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>
#include <vk_device_memory.hpp>
#include <device_memory_block.hpp>

#include <optional.hpp>

namespace ste {
namespace gl {

class unique_device_ptr;

class unique_device_ptr_allocator {
public:
	virtual ~unique_device_ptr_allocator() noexcept {}
	virtual void deallocate(const unique_device_ptr &) const = 0;
};

class unique_device_ptr {
private:
	using device_memory_ptr_t = vk::vk_device_memory*;

public:
	using allocator_type = unique_device_ptr_allocator;

private:
	device_memory_ptr_t memory;
	optional<const allocator_type*> allocator;
	device_memory_block<std::uint64_t> block;
	bool private_allocation;
	std::uint32_t memory_type;
	std::uint64_t tag;

public:
	unique_device_ptr() {}
	unique_device_ptr(device_memory_ptr_t memory,
					  const allocator_type *allocator,
					  const device_memory_block<std::uint64_t> &block,
					  bool private_allocation,
					  std::uint32_t memory_type,
					  std::uint64_t tag)
		: memory(memory), 
		allocator(allocator), 
		block(block), 
		private_allocation(private_allocation),
		memory_type(memory_type),
		tag(tag)
	{}
	~unique_device_ptr() noexcept { free();  }

	unique_device_ptr(unique_device_ptr &&) = default;
	unique_device_ptr(const unique_device_ptr &) = delete;
	unique_device_ptr &operator=(unique_device_ptr &&) = default;
	unique_device_ptr &operator=(const unique_device_ptr &) = delete;

	void free() {
		if (allocator) {
			allocator.get()->deallocate(*this);
			allocator = none;
		}
	}

	auto& get() { return block; }
	auto& get() const { return block; }
	auto* operator->() { return &get(); }
	auto* operator->() const { return &get(); }
	auto& operator*() { return get(); }
	auto& operator*() const { return get(); }

	auto& get_memory() { return memory; }
	auto& get_memory() const { return memory; }
	auto& get_memory_type() const { return memory_type; }
	auto& get_heap_tag() const { return tag; }
	bool is_private_allocation() const { return private_allocation; }

	operator bool() const { return !!allocator; }
	bool operator!() const { return !allocator; }
};

}
}
