//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <ste_context.hpp>
#include <device_resource_memory_allocator.hpp>

#include <vk_buffer_sparse.hpp>
#include <device_buffer_base.hpp>
#include <buffer_usage.hpp>

#include <device_sparse_memory_bind.hpp>
#include <device_sparse_binding_batch.hpp>

#include <range.hpp>
#include <range_list.hpp>
#include <lib/vector.hpp>
#include <lib/flat_map.hpp>
#include <allow_type_decay.hpp>
#include <alias.hpp>
#include <mutex>
#include <algorithm>

namespace ste {
namespace gl {

template <
	typename T,
	class allocation_policy = device_resource_allocation_policy_device
>
class device_buffer_sparse :
	public device_buffer_base,
	public allow_type_decay<device_buffer_sparse<T, allocation_policy>, vk::vk_buffer_sparse<>, false> {
private:
	struct ctor {};

public:
	using bind_range_t = range<std::uint64_t>;

	/*
	 *	@brief	Result returned from allocate_sparse_memory.
	 *			Must be passed to bind_sparse_memory.
	 */
	struct allocate_sparse_memory_result_t {
		friend class device_buffer_sparse;
	private:
		lib::vector<device_sparse_memory_bind> memory_binds;

		allocate_sparse_memory_result_t() = default;
	public:
		allocate_sparse_memory_result_t(allocate_sparse_memory_result_t&&) = default;
		allocate_sparse_memory_result_t &operator=(allocate_sparse_memory_result_t&&) = default;
		allocate_sparse_memory_result_t(const allocate_sparse_memory_result_t&) = delete;
		allocate_sparse_memory_result_t &operator=(const allocate_sparse_memory_result_t&) = delete;

		operator bool() const { return memory_binds.size(); }
	};

private:
	using resource_t = vk::vk_buffer_sparse<>;
	using alloc_t = device_memory_heap::allocation_type;

	static constexpr auto element_size = static_cast<bind_range_t::value_type>(sizeof(T));

private:
	alias<const ste_context> ctx;
	resource_t resource;
	VkMemoryRequirements memory_requirements{};

	lib::flat_map<bind_range_t, alloc_t> bound_ranges;
	range_list<std::uint64_t> ranges_to_unbind;
	std::mutex bound_ranges_mutex;

public:
	auto atom_size() const {
		return glm::max<std::size_t>(memory_requirements.alignment, 
									 element_size);
	}

	bind_range_t align(const bind_range_t &range) const {
		auto alignment = static_cast<std::size_t>(atom_size());

		bind_range_t ret = range * element_size;
		return ret.align(alignment);
	}

private:
	template <typename Semaphore>
	static auto vk_semaphores(const lib::vector<Semaphore*> &s) {
		lib::vector<VkSemaphore> vk_sems;
		vk_sems.reserve(s.size());
		for (auto &sem : s)
			vk_sems.push_back(*sem);

		return vk_sems;
	}

	auto unbind(const lib::vector<bind_range_t> &unbind_regions) {
		lib::vector<device_sparse_memory_bind> memory_binds;

		// Accumulate the unbind ranges
		for (auto &r : unbind_regions) {
			const auto aligned_range = align(r);
			ranges_to_unbind.add(aligned_range);
		}

		// Attempt to unbind allocations
		auto pred = [](const std::pair<bind_range_t, alloc_t> &lhs, const bind_range_t &rhs) {
			return lhs.first < rhs;
		};
		auto it_unbind_ranges = ranges_to_unbind.begin();
		for (auto it = it_unbind_ranges == ranges_to_unbind.end() ? bound_ranges.end() : bound_ranges.lower_bound(*it_unbind_ranges);
			 it != bound_ranges.end();
			 it = it_unbind_ranges == ranges_to_unbind.end() ? bound_ranges.end() : std::lower_bound(it, bound_ranges.end(), *it_unbind_ranges, pred)) {
			if (it_unbind_ranges->contains(it->first)) {
				// Can unbind
				device_sparse_memory_bind b;
				b.allocation = nullptr;
				b.resource_offset_bytes = it->first.start;
				b.size_bytes = it->first.length;
				memory_binds.push_back(b);

				// Erase allocation
				it = bound_ranges.erase(it);
				it_unbind_ranges = ranges_to_unbind.remove(it->first);
			}
			else {
				it_unbind_ranges = std::next(it_unbind_ranges);
			}
		}

		return memory_binds;
	}

	auto bind(const lib::vector<bind_range_t> &bind_regions) {
		lib::vector<device_sparse_memory_bind> memory_binds;
		if (!bind_regions.size())
			return memory_binds;

		// Accumulate bind ranges
		range_list<std::uint64_t> ranges_to_bind;
		std::uint64_t bind_range_top = 0;
		for (auto &r : bind_regions) {
			const auto aligned_range = align(r);

			ranges_to_bind.add(aligned_range);
			bind_range_top = aligned_range.start + aligned_range.length;

			// Remove from pending unbind ranges
			ranges_to_unbind.remove(aligned_range);
		}

		// Remove already bound ranges
		{
			auto it = bound_ranges.lower_bound(bind_regions.front());
			if (it != bound_ranges.begin()) it = std::prev(it);
			for (; it != bound_ranges.end() && it->first.start < bind_range_top; ++it)
				ranges_to_bind.remove(it->first);
		}

		// Allocate and bind
		for (auto &r : ranges_to_bind) {
			const auto size = r.length;
			auto it = bound_ranges.emplace(r,
										   device_resource_memory_allocator<allocation_policy>()(ctx.get().device_memory_allocator(),
																								 size,
																								 memory_requirements));

			device_sparse_memory_bind b;
			b.allocation = &it.first->second;
			b.resource_offset_bytes = r.start;
			b.size_bytes = r.length;
			memory_binds.push_back(b);
		}

		return memory_binds;
	}

	device_buffer_sparse(ctor,
						 const ste_context &ctx,
						 resource_t &&resource)
		: ctx(ctx),
		  resource(std::move(resource)) {
		memory_requirements = this->resource.get_memory_requirements();
	}

public:
	device_buffer_sparse(const ste_context &ctx,
						 std::uint64_t count,
						 const buffer_usage &usage,
						 const char *name)
		: device_buffer_sparse(ctor(),
							   ctx,
							   resource_t(ctx.device(),
										  element_size,
										  count,
										  static_cast<VkBufferUsageFlags>(usage),
										  name)) {}

	~device_buffer_sparse() noexcept {}

	const vk::vk_buffer<> &get_buffer_handle() const override final { return *this; }

	device_buffer_sparse(device_buffer_sparse &&) = default;
	device_buffer_sparse &operator=(device_buffer_sparse &&) = default;

	/**
	*	@brief	Allocates memory for sparse binding.
	*			Unbind and bind regions should not overlap.
	*
	*	@param	unbind_regions		Buffer regions to unbind
	*	@param	bind_regions		Buffer regions to bind
	*
	*	@return	An object describing the allocated memory, which must be passed to bind_sparse_memory() to perform the binding.
	*/
	auto allocate_sparse_memory(const lib::vector<bind_range_t> &unbind_regions,
								const lib::vector<bind_range_t> &bind_regions) {
		lib::vector<device_sparse_memory_bind> binds, unbinds;
		{
			std::unique_lock<std::mutex> l(bound_ranges_mutex);
			unbinds = unbind(unbind_regions);
			binds = bind(bind_regions);
		}

		allocate_sparse_memory_result_t result;
		result.memory_binds.reserve(unbinds.size() + binds.size());
		std::copy(unbinds.begin(), unbinds.end(), std::back_inserter(result.memory_binds));
		std::copy(binds.begin(), binds.end(), std::back_inserter(result.memory_binds));

		return result;
	}

	/**
	*	@brief	Binds sparse memory.
	*
	*	@param	allocation			Object allcoated by and returned from allocate_sparse_memory()
	*	@param	wait_semaphores		Array of pairs of semaphores upon which to wait before execution
	*	@param	signal_semaphores	Sempahores to signal once the command has completed execution
	*	
	*	@return	Future to enqueued task.
	*/
	auto bind_sparse_memory(allocate_sparse_memory_result_t &&allocation,
							lib::vector<wait_semaphore> &&wait_semaphores = {},
							lib::vector<semaphore*> &&signal_semaphores = {}) {
		if (!allocation) {
			assert(false);
		}

		auto &q = ctx.get().device().select_queue(ste_queue_selector<ste_queue_selector_policy_flexible>(ste_queue_type::data_transfer_sparse_queue));
		return q.enqueue([=, allocation = std::move(allocation), wait_semaphores = std::move(wait_semaphores), signal_semaphores = std::move(signal_semaphores)]() mutable {
			// Allocate sparse binding batch
			using batch_t = device_sparse_binding_batch<>;
			auto batch = ste_device_queue::thread_allocate_batch_custom<batch_t>();

			batch->signal_semaphores = std::move(signal_semaphores);
			batch->wait_semaphores = std::move(wait_semaphores);

			// Set bind regions
			batch->bind(*this, 
						allocation.memory_binds);

			// Queue sparse binding command
			ste_device_queue::submit_batch(std::move(batch));
		});
	}

	resource_t &get() { return resource; }
	const resource_t &get() const { return resource; }

	auto &parent_context() const { return ctx.get(); }

	std::uint64_t get_elements_count() const override final { return this->get().get_elements_count(); }
	std::uint32_t get_element_size_bytes() const override final { return element_size; };
	bool is_sparse() const override final { return true; };
};

}
}
