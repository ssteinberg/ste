//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_resource_traits.hpp>

#include <vk_mmap.hpp>
#include <buffer_usage.hpp>
#include <pipeline_stage.hpp>
#include <event.hpp>
#include <device_buffer.hpp>
#include <device_resource_allocation_policy.hpp>

#include <command_recorder.hpp>
#include <cmd_wait_events.hpp>

#include <lib/unique_ptr.hpp>
#include <allow_type_decay.hpp>
#include <array>
#include <utility>

namespace ste {
namespace gl {

template <typename SegmentType, unsigned SegmentCount>
class ring_buffer : ste_resource_deferred_create_trait, public allow_type_decay<ring_buffer<SegmentType, SegmentCount>, device_buffer<SegmentType, device_resource_allocation_policy_host_visible>> {
private:
	using buffer_t = device_buffer<SegmentType, device_resource_allocation_policy_host_visible>;
	using mmap_ptr_t = lib::unique_ptr<vk::vk_mmap<SegmentType>>;
	using lock_t = event;
	static constexpr auto buffer_usage_additional_flags = buffer_usage::transfer_dst;

public:
	using value_type = SegmentType;
	static constexpr std::uint32_t segment_count = SegmentCount;

private:
	buffer_t buffer;
	mmap_ptr_t ptr{ nullptr };

	std::uint32_t current{ 0 };
	std::array<lock_t, segment_count> locks;

private:
	static auto generate_lock(const ste_context &ctx, std::size_t) {
		return lock_t(ctx.device());
	}
	template <std::size_t... is>
	static auto generate_array(const ste_context &ctx, std::index_sequence<is...>) {
		return std::array<lock_t, sizeof...(is)>{ generate_lock(ctx, is)... };
	}

public:
	ring_buffer(const ste_context &ctx,
				const buffer_usage &usage)
		: buffer(ctx,
				 segment_count,
				 usage | buffer_usage_additional_flags),
		locks(generate_array(ctx, std::make_index_sequence<segment_count>()))
	{
		ptr = buffer.get_underlying_memory().template mmap<value_type>(0, segment_count);
		for (auto &l : locks)
			l.set();
	}
	~ring_buffer() noexcept {}

	/**
	 *	@brief	Commit new data to the ring buffer. Might block till a slot becomes available. 
	 *			After using the commited data, the consumer should call signal() to unlock the slot.
	 *			
	 *	@Returns The index of the acquired slot.
	 */
	auto commit(const value_type &data) {
		current = (current + 1) % segment_count;

		// Spin lock, waiting for segment to become available
		event& e = locks[current];
		while (!e.is_signaled())
			std::this_thread::yield();
		e.reset();

		// Write new data
		(*ptr)[current] = data;
		ptr->flush_ranges({ {current, 1} });

		// Return range
		return current;
	}

	/**
	*	@brief	Unlocks a slot. Must be paired with commit() calls.
	*	
	*	@param	recorder		Command recorder that records a buffer that will be submitted to device
	*	@oaram	segment_index	Index of slot. Use commit()'s return value.
	*	@param	stage			Pipeline stage at which to unlock the slot
	*/
	void signal(command_recorder &recorder,
				std::uint32_t segment_index,
				pipeline_stage stage) const {
		// Signal event
		auto& e = locks[segment_index];
		recorder << cmd_set_event(e, stage);
	}

	ring_buffer(ring_buffer&&) = default;
	ring_buffer &operator=(ring_buffer&&) = default;

	auto& get() { return buffer; }
	auto& get() const { return buffer; }
};

}
}
