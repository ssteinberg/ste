//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>
#include <vk_device_memory.hpp>

#include <optional.hpp>

namespace StE {
namespace GL {

template <typename Heap, typename Ptr>
class unique_device_ptr {
public:
	using heap_type = Heap;
	using ptr_type = Ptr;

private:
	const vk_device_memory *memory;
	optional<heap_type*> heap;
	ptr_type ptr;

public:
	unique_device_ptr() = default;
	unique_device_ptr(const vk_device_memory *memory,
					  heap_type *heap,
					  ptr_type &&ptr)
		: memory(memory), heap(heap), ptr(std::move(ptr)) {}
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
	auto& operator->() { return *ptr; }
	auto& operator->() const { return *ptr; }
	auto& operator*() { return *ptr; }
	auto& operator*() const { return *ptr; }

	auto& get_memory() const { return memory; }
	auto& get_heap() const { return heap.get(); }

	operator bool() const { return !!heap; }
	bool operator!() const { return !heap; }
};

}
}
