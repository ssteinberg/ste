//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>
#include <log.hpp>

#include <lib/alloc.hpp>
#include <lib/string.hpp>

#include <attributed_string.hpp>
#include <attrib.hpp>

namespace ste {
namespace gl {

namespace vk {

template <typename AlignedAllocator = lib::allocator<std::uint8_t>>
class vk_host_allocator {
private:
	static AlignedAllocator shared_allocator;

	using value = typename AlignedAllocator::value_type;
	using pointer = typename AlignedAllocator::pointer;

	static_assert(sizeof(value) == 1, "AlignedAllocator must allocate bytes");

private:
	static auto readable_allocation_scope(VkSystemAllocationScope allocation_scope) {
		switch (allocation_scope) {
		case VK_SYSTEM_ALLOCATION_SCOPE_COMMAND:
			return lib::string("command");
		case VK_SYSTEM_ALLOCATION_SCOPE_OBJECT:
			return lib::string("object");
		case VK_SYSTEM_ALLOCATION_SCOPE_CACHE:
			return lib::string("cache");
		case VK_SYSTEM_ALLOCATION_SCOPE_DEVICE:
			return lib::string("device");
		case VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE:
			return lib::string("instance");
		default:
			return lib::string("unknown");
		}
	}

public:
	static void* allocate(void* user_data,
						  std::size_t bytes,
						  std::size_t alignment,
						  VkSystemAllocationScope allocation_scope) {
		return shared_allocator.allocate_aligned(bytes, alignment);
	}
	static void* reallocate(void* user_data,
							void* ptr,
							std::size_t bytes,
							std::size_t alignment,
							VkSystemAllocationScope allocation_scope) {
		return shared_allocator.reallocate_aligned(reinterpret_cast<pointer>(ptr), bytes, alignment);
	}
	static void free(void *user_data,
					 void *ptr) {
		shared_allocator.deallocate(reinterpret_cast<pointer>(ptr));
	}

	static void internal_allocation_notification(void *user_data,
												 std::size_t bytes,
												 VkInternalAllocationType allocation_type,
												 VkSystemAllocationScope allocation_scope) {
#ifdef DEBUG
		ste_log() << text::attributes::tiny("(VK host allocator) ") << text::attributes::b("+") << " internal " << text::attributes::b(lib::to_string(bytes)) << "bytes --- " << readable_allocation_scope(allocation_scope);
#endif
	}
	static void internal_free_notification(void *user_data,
										   std::size_t bytes,
										   VkInternalAllocationType allocation_type,
										   VkSystemAllocationScope allocation_scope) {
#ifdef DEBUG
		ste_log() << text::attributes::tiny("(VK host allocator) ") << text::attributes::b("-") << " internal " << text::attributes::b(lib::to_string(bytes)) << "bytes --- " << readable_allocation_scope(allocation_scope);
#endif
	}

	static VkAllocationCallbacks allocation_callbacks() {
		VkAllocationCallbacks callbacks = {};
		callbacks.pUserData = nullptr;
		callbacks.pfnAllocation = allocate;
		callbacks.pfnReallocation = reallocate;
		callbacks.pfnFree = free;
		callbacks.pfnInternalAllocation = internal_allocation_notification;
		callbacks.pfnInternalFree = internal_free_notification;

		return callbacks;
	}
};

template <typename A>
A vk_host_allocator<A>::shared_allocator;

}

}
}
