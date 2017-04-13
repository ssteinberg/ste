//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <device_resource_queue_transferable.hpp>
#include <device_resource_memory_allocator.hpp>

#include <ste_context.hpp>
#include <vk_buffer.hpp>
#include <device_buffer_base.hpp>

#include <vk_queue.hpp>
#include <host_command.hpp>

#include <optional.hpp>
#include <range.hpp>
#include <vector>
#include <allow_type_decay.hpp>

namespace StE {
namespace GL {

template <
	typename T,
	std::uint64_t minimal_atom_size = 65536,
	class allocation_policy = device_resource_allocation_policy_device
>
class device_buffer_sparse : 
	public device_buffer_base,
	public allow_type_decay<device_buffer_sparse<T, minimal_atom_size, allocation_policy>, vk_buffer<T, true>, false>
{
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
	const ste_context &ctx;
	resource_t resource;
	VkMemoryRequirements memory_requirements{};

	bind_map_t bound_atoms_map;

public:
	auto atom_size() const {
		return std::max(memory_requirements.alignment, minimal_atom_size);
	}
	bind_range_t atoms_range_contain(const bind_range_t &range) const {
		auto alignment = static_cast<std::size_t>(atom_size());

		bind_range_t ret;
		ret.start = range.start * sizeof(T) / alignment;
		ret.length = (range.length * sizeof(T) + alignment - 1) / alignment;

		return ret;
	}
	bind_range_t atoms_range_intersect(const bind_range_t &range) const {
		auto alignment = static_cast<std::size_t>(atom_size());

		auto atom_start = (range.start * sizeof(T) + alignment - 1) / alignment;
		auto atom_end = (range.start + range.length) / alignment;

		bind_range_t ret;
		ret.start = atom_start;
		ret.length = atom_end;

		return ret;
	}

private:
	template <typename selector_policy>
	device_buffer_sparse(ctor,
						 const ste_context &ctx,
						 const ste_queue_selector<selector_policy> &initial_queue_selector,
						 resource_t &&resource)
		: device_buffer_base(ctx.device().select_queue(initial_queue_selector)->queue_descriptor().family),
		ctx(ctx), 
		resource(std::move(resource))
	{
		memory_requirements = this->resource.get_memory_requirements();
	}
	device_buffer_sparse(ctor,
						 const ste_context &ctx,
						 const device_resource_queue_ownership::family_t &family,
						 resource_t &&resource)
		: device_buffer_base(family),
		ctx(ctx),
		resource(std::move(resource))
	{
		memory_requirements = this->resource.get_memory_requirements();
	}

public:
	template <typename selector_policy, typename ... Args>
	device_buffer_sparse(const ste_context &ctx,
						 const ste_queue_selector<selector_policy> &initial_queue_selector,
						 Args&&... args)
		: device_buffer_sparse(ctor(), ctx, initial_queue_selector,
							   resource_t(ctx.device(), std::forward<Args>(args)...))
	{}
	template <typename ... Args>
	device_buffer_sparse(const ste_context &ctx,
						 const device_resource_queue_ownership::family_t &family,
						 Args&&... args)
		: device_buffer_sparse(ctor(), ctx, family,
							   resource_t(ctx.device(), std::forward<Args>(args)...))
	{}
	~device_buffer_sparse() noexcept {}

	const vk_buffer_base& get_buffer_handle() const override final { return *this; }

	device_buffer_sparse(device_buffer_sparse &&) = default;
	device_buffer_sparse &operator=(device_buffer_sparse &&) = default;

	/**
	*	@brief	Creates a host command to bind sparse memory.
	*			Unbind and bind regions should not overlap.
	*
	*	@param	unbind_regions		Buffer regions to unbind
	*	@param	bind_regions		Buffer regions to bind
	*	@param	wait_semaphores		Array of pairs of semaphores upon which to wait before execution
	*	@param	signal_semaphores	Sempahores to signal once the command has completed execution
	*	@param	fence				Optional fence, to be signaled when the command has completed execution
	*	
	*	@return	A host command to bound/unbound atoms.
	*/
	host_command cmd_bind_sparse_memory(const std::vector<bind_range_t> &unbind_regions,
										const std::vector<bind_range_t> &bind_regions,
										const std::vector<VkSemaphore> &wait_semaphores,
										const std::vector<VkSemaphore> &signal_semaphores,
										const vk_fence *fence = nullptr) {
		return host_command([=](const vk_queue &queue) {
			std::vector<vk_sparse_memory_bind> memory_binds;
			auto size = atom_size();

			for (std::size_t i = 0; i < unbind_regions.size(); ++i) {
				auto &r = unbind_regions[i];
				auto atoms = atoms_range_intersect(r);

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
				auto atoms = atoms_range_contain(r);

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
				return;

			// Queue sparse binding command
			resource.cmd_bind_sparse_memory(queue,
											memory_binds,
											wait_semaphores,
											signal_semaphores,
											fence);
		});
	}

	resource_t& get() { return resource; }
	const resource_t& get() const { return resource; }

	std::uint64_t get_elements_count() const override final { return this->get().get_elements_count(); }
	std::uint32_t get_element_size_bytes() const override final { return sizeof(T); };
	bool is_sparse() const override final { return true; };
};

}
}
