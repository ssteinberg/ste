//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_resource.hpp>
#include <ste_device_queue.hpp>
#include <range.hpp>

#include <buffer_usage.hpp>
#include <unique_fence.hpp>
#include <device_buffer.hpp>
#include <device_resource_allocation_policy.hpp>

#include <memory>
#include <vector>
#include <allow_type_decay.hpp>

namespace ste {
namespace gl {

template <typename Segment>
class ring_buffer : ste_resource_deferred_create_trait, public allow_type_decay<ring_buffer<Segment>, device_buffer<Segment, device_resource_allocation_policy_device>> {
private:
	using buffer_t = device_buffer<Segment, device_resource_allocation_policy_device>;
	using mmap_ptr_t = std::unique_ptr<vk_mmap<Segment>>;
	using lock_t = const unique_fence<void>*;
	static constexpr auto buffer_usage_additional_flags = buffer_usage::transfer_dst;

private:
	buffer_t buffer;
	const std::uint32_t segments;
	mmap_ptr_t ptr{ nullptr };

	std::uint32_t current{ 0 };
	std::vector<lock_t> locks;

public:
	ring_buffer(const ste_context &ctx,
				std::uint32_t segments,
				const buffer_usage &usage)
		: buffer(ctx,
				 segments,
				 usage | buffer_usage_additional_flags),
		segments(segments)
	{
		ptr = buffer.get_underlying_memory().template mmap<Segment>(0, segments);
		locks.resize(segments, lock_t{ nullptr });
	}
	~ring_buffer() noexcept {}

	range<> commit(const Segment &data,
				   const lock_t &l) {
		current = (current + 1) % segments;

		// Wait for segment to become available and lock it
		if (locks[current] != nullptr)
			locks[current]->wait_idle();
		locks[current] = l;

		// Write new data
		(*ptr)[current] = data;
		ptr->flush_ranges({ {current, 1} });

		// Return range
		return{ current, 1 };
	}

	ring_buffer(ring_buffer&&) = default;
	ring_buffer &operator=(ring_buffer&&) = default;

	auto get_segment_count() const { return segments; }

	auto& get() { return buffer; }
	auto& get() const { return buffer; }
};

}
}
