//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <device_resource_memory_allocator.hpp>

#include <ste_context.hpp>
#include <vk_buffer_sparse.hpp>
#include <device_buffer_base.hpp>

#include <buffer_usage.hpp>

#include <vk_queue.hpp>
#include <host_command.hpp>

#include <optional.hpp>
#include <range.hpp>
#include <lib/vector.hpp>
#include <allow_type_decay.hpp>
#include <alias.hpp>

namespace ste {
namespace gl {

template <
	typename T,
	std::uint64_t minimal_atom_size = 65536,
	class allocation_policy = device_resource_allocation_policy_device
>
class device_buffer_sparse : 
	public device_buffer_base,
	public allow_type_decay<device_buffer_sparse<T, minimal_atom_size, allocation_policy>, vk::vk_buffer_sparse, false>
{
private:
	struct ctor {};

public:
	using atom_address_t = std::uint32_t;
	using atom_bind_range_t = range<atom_address_t>;
	using bind_range_t = range<std::uint64_t>;

private:
	using resource_t = vk::vk_buffer_sparse;
	using atom_t = device_memory_heap::allocation_type;
	using bind_map_t = lib::vector<atom_t>;

private:
	alias<const ste_context> ctx;
	resource_t resource;
	VkMemoryRequirements memory_requirements{};

	bind_map_t bound_atoms_map;

public:
	auto atom_size() const {
		return std::max(memory_requirements.alignment, minimal_atom_size);
	}
	atom_bind_range_t atoms_range_contain(const bind_range_t &range) const {
		auto alignment = static_cast<std::size_t>(atom_size());

		atom_bind_range_t ret;
		ret.start = static_cast<std::uint32_t>(range.start * sizeof(T) / alignment);
		ret.length = static_cast<std::uint32_t>((range.length * sizeof(T) + alignment - 1) / alignment);

		return ret;
	}
	atom_bind_range_t atoms_range_intersect(const bind_range_t &range) const {
		auto alignment = static_cast<std::size_t>(atom_size());

		auto atom_start = (range.start * sizeof(T) + alignment - 1) / alignment;
		auto atom_end = (range.start + range.length) / alignment;

		atom_bind_range_t ret;
		ret.start = static_cast<std::uint32_t>(atom_start);
		ret.length = static_cast<std::uint32_t>(atom_end);

		return ret;
	}

private:
	static auto vk_semaphores(const lib::vector<const semaphore*> &s) {
		lib::vector<VkSemaphore> vk_sems;
		vk_sems.reserve(s.size());
		for (auto &sem : s)
			vk_sems.push_back(*sem);

		return vk_sems;
	}

	device_buffer_sparse(ctor,
						 const ste_context &ctx,
						 resource_t &&resource)
		: ctx(ctx),
		resource(std::move(resource))
	{
		memory_requirements = this->resource.get_memory_requirements();
	}

public:
	device_buffer_sparse(const ste_context &ctx,
						 std::uint64_t count,
						 const buffer_usage &usage)
		: device_buffer_sparse(ctor(), ctx,
							   resource_t(ctx.device(), 
										  sizeof(T),
										  count,
										  static_cast<VkBufferUsageFlags>(usage)))
	{}
	~device_buffer_sparse() noexcept {}

	const vk::vk_buffer& get_buffer_handle() const override final { return *this; }

	device_buffer_sparse(device_buffer_sparse &&) = default;
	device_buffer_sparse &operator=(device_buffer_sparse &&) = default;

	/**
	*	@brief	Creates a host command to bind sparse memory.
	*			Unbind and bind regions should not overlap.
	*
	*	@param	unbind_regions		Buffer regions to unbind
	*	@param	bind_regions			Buffer regions to bind
	*	@param	wait_semaphores		Array of pairs of semaphores upon which to wait before execution
	*	@param	signal_semaphores	Sempahores to signal once the command has completed execution
	*	@param	fence				Optional fence, to be signaled when the command has completed execution
	*	
	*	@return	A host command to bound/unbound atoms.
	*/
	host_command cmd_bind_sparse_memory(const lib::vector<bind_range_t> &unbind_regions,
										const lib::vector<bind_range_t> &bind_regions,
										const lib::vector<const semaphore*> &wait_semaphores,
										const lib::vector<const semaphore*> &signal_semaphores,
										const vk::vk_fence *fence = nullptr) {
		return host_command([=](const vk::vk_queue &queue) {
			lib::vector<vk::vk_sparse_memory_bind> memory_binds;
			auto size = atom_size();

			for (std::size_t i = 0; i < unbind_regions.size(); ++i) {
				auto &r = unbind_regions[i];
				auto atoms = atoms_range_intersect(r);

				// Unbind each bound atom individually
				for (atom_address_t p = atoms.start; p != atoms.start + atoms.length; ++p) {
					if (bound_atoms_map.size() > p && bound_atoms_map[p]) {
						bound_atoms_map[p] = atom_t();

						vk::vk_sparse_memory_bind b;
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
						bound_atoms_map[p] = device_resource_memory_allocator<allocation_policy>()(ctx.get().device_memory_allocator(),
																								   size,
																								   memory_requirements);

						vk::vk_sparse_memory_bind b;
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
											vk_semaphores(wait_semaphores),
											vk_semaphores(signal_semaphores),
											fence);
		});
	}

	resource_t& get() { return resource; }
	const resource_t& get() const { return resource; }

	auto& parent_context() const { return ctx.get(); }

	std::uint64_t get_elements_count() const override final { return this->get().get_elements_count(); }
	std::uint32_t get_element_size_bytes() const override final { return sizeof(T); };
	bool is_sparse() const override final { return true; };
};

}
}
