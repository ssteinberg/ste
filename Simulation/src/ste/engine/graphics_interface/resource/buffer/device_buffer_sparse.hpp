//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <ste_context.hpp>
#include <vk_buffer.hpp>
#include <vk_queue.hpp>

#include <device_resource_memory_allocator.hpp>

#include <range.hpp>
#include <vector>

namespace StE {
namespace GL {

template <
	typename T,
	std::uint64_t minimal_atom_size = 65536,
	class allocation_policy = device_resource_allocation_policy_device
>
class device_buffer_sparse {
private:
	struct ctor {};

public:
	using atom_address_t = std::uint32_t;
	using bind_range_t = range<atom_address_t>;

private:
	using resource_t = vk_buffer<T, true>;
	using atom_t = device_memory_heap::allocation_type;
	using bind_map_t = std::vector<atom_t>;

private:
	resource_t resource;
	VkMemoryRequirements memory_requirements{};

	bind_map_t bound_atoms_map;

public:
	auto atom_size() const {
		return std::max(memory_requirements.alignment, minimal_atom_size);
	}
	bind_range_t atoms_range(const bind_range_t &range) const {
		auto alignment = static_cast<std::size_t>(atom_size());

		bind_range_t ret;
		ret.start = range.start * sizeof(T) / alignment;
		ret.length = (range.length * sizeof(T) + alignment - 1) / alignment;

		return ret;
	}

public:
	const ste_context &ctx;
	device_resource_queue_ownership queue_ownership;

private:
	device_buffer_sparse(ctor,
						 const ste_context &ctx,
						 const device_resource_queue_ownership::resource_queue_selector_t &selector,
						 resource_t &&resource)
		: resource(std::move(resource)),
		ctx(ctx),
		queue_ownership(ctx, selector)
	{
		memory_requirements = this->resource.get_memory_requirements();
	}
	device_buffer_sparse(ctor,
						 const ste_context &ctx,
						 const device_resource_queue_ownership::queue_index_t &queue_index,
						 resource_t &&resource)
		: resource(std::move(resource)),
		ctx(ctx),
		queue_ownership(queue_index)
	{
		memory_requirements = this->resource.get_memory_requirements();
	}

public:
	template <typename ... Args>
	device_buffer_sparse(const ste_context &ctx,
						 const device_resource_queue_ownership::resource_queue_selector_t &selector,
						 Args&&... args)
		: device_buffer_sparse(ctor(), ctx, selector,
							   resource_t(ctx.device().logical_device(), std::forward<Args>(args)...))
	{}
	template <typename ... Args>
	device_buffer_sparse(const ste_context &ctx,
						 const device_resource_queue_ownership::queue_index_t &queue_index,
						 Args&&... args)
		: device_buffer_sparse(ctor(), ctx, queue_index,
							   resource_t(ctx.device().logical_device(), std::forward<Args>(args)...))
	{}
	~device_buffer_sparse() noexcept {}

	device_buffer_sparse(device_buffer_sparse &&) = default;
	device_buffer_sparse &operator=(device_buffer_sparse &&) = default;

	/**
	*	@brief	Queues a bind sparse memory command on the queue. Unbind and bind regions should not overlap.
	*
	*	@param	queue				Queue to use
	*	@param	unbind_regions		Buffer regions to unbind
	*	@param	bind_regions		Buffer regions to bind
	*	@param	wait_semaphores		Array of pairs of semaphores upon which to wait before execution
	*	@param	signal_semaphores	Sempahores to signal once the command has completed execution
	*	@param	fence				Optional fence, to be signaled when the command has completed execution
	*	
	*	@return	True if bound atoms need to be changed a sparse binding command was queued, false otherwise.
	*/
	bool cmd_bind_sparse_memory(const vk_queue &queue,
								const std::vector<bind_range_t> &unbind_regions,
								const std::vector<bind_range_t> &bind_regions,
								const std::vector<const vk_semaphore*> &wait_semaphores,
								const std::vector<const vk_semaphore*> &signal_semaphores,
								const vk_fence *fence = nullptr) {
		std::vector<vk_sparse_memory_bind> memory_binds;
		auto size = atom_size();

		for (std::size_t i = 0; i < unbind_regions.size(); ++i) {
			auto &r = unbind_regions[i];
			auto atoms = atoms_range(r);

			// Unbind each bound atom individually
			for (atom_address_t p = atoms.start; p != atoms.start + atoms.length; ++p) {
				if (bound_atoms_map.size() > p && bound_atoms_map[p]) {
					bound_atoms_map[p] = atom_t();

					vk_sparse_memory_bind b;
					b.allocation = nullptr;
					b.resource_offset_bytes = p * size;
					b.size_bytes = size;

					memory_binds.push_back(b);
				}
			}
		}
		for (std::size_t i = 0; i < bind_regions.size(); ++i) {
			auto &r = bind_regions[i];
			auto atoms = atoms_range(r);

			if (bound_atoms_map.size() < atoms.start + atoms.length)
				bound_atoms_map.resize(atoms.start + atoms.length);

			// Bind each bound atom individually
			for (atom_address_t p = atoms.start; p != atoms.start + atoms.length; ++p) {
				if (!bound_atoms_map[p]) {
					bound_atoms_map[p] = device_resource_memory_allocator<allocation_policy>()(ctx.device_memory_allocator(),
																							   size,
																							   memory_requirements);

					vk_sparse_memory_bind b;
					b.allocation = &bound_atoms_map[p];
					b.resource_offset_bytes = p * size;
					b.size_bytes = size;

					memory_binds.push_back(b);
				}
			}
		}

		// Nothing to bind/unbind
		if (!memory_binds.size())
			return false;

		// Queue sparse binding command
		resource.cmd_bind_sparse_memory(queue,
										memory_binds,
										wait_semaphores,
										signal_semaphores,
										fence);
		return true;
	}

	operator resource_t&() { return get(); }
	operator const resource_t&() const { return get(); }

	resource_t& get() { return resource; }
	const resource_t& get() const { return resource; }

	resource_t* operator->() { return &get(); }
	const resource_t* operator->() const { return &get(); }
	resource_t& operator*() { return get(); }
	const resource_t& operator*() const { return get(); }
};

}
}
