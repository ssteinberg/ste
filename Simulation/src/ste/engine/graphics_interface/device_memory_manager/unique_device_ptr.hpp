//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>
#include <vk_device_memory.hpp>

#include <optional.hpp>

namespace ste {
namespace gl {

template <typename Heap, typename Ptr>
class unique_device_ptr {
private:
	using device_memory_ptr_t = vk::vk_device_memory*;

public:
	using heap_type = Heap;
	using ptr_type = Ptr;

private:
	device_memory_ptr_t memory;
	optional<heap_type*> heap;
	ptr_type ptr;
	bool private_allocation;

public:
	unique_device_ptr() = default;
	unique_device_ptr(device_memory_ptr_t memory,
					  heap_type *heap,
					  ptr_type &&ptr,
					  bool private_allocation)
		: memory(memory), heap(heap), ptr(std::move(ptr)), private_allocation(private_allocation) 
	{}
	~unique_device_ptr() noexcept { free();  }

	unique_device_ptr(unique_device_ptr &&) = default;
	unique_device_ptr(const unique_device_ptr &) = delete;
	unique_device_ptr &operator=(unique_device_ptr &&) = default;
	unique_device_ptr &operator=(const unique_device_ptr &) = delete;

	void free() {
		if (heap) {
			heap.get()->deallocate(*this);
			heap = none;
		}
	}

	auto& get() { return ptr; }
	auto& get() const { return ptr; }
	auto& operator->() { return ptr; }
	auto& operator->() const { return ptr; }
	auto& operator*() { return *ptr; }
	auto& operator*() const { return *ptr; }

	auto& get_memory() { return memory; }
	auto& get_memory() const { return memory; }
	auto& get_heap() const { return heap.get(); }
	bool is_private_allocation() const { return private_allocation; }

	operator bool() const { return !!heap; }
	bool operator!() const { return !heap; }
};

}
}
