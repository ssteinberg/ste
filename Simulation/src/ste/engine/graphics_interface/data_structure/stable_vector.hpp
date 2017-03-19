//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_resource.hpp>
#include <ste_device_queue.hpp>
#include <data_structure_common.hpp>

#include <vk_cmd_update_buffer.hpp>

#include <device_buffer_sparse.hpp>
#include <device_resource_allocation_policy.hpp>

namespace StE {
namespace GL {

template <
	typename T,
	std::uint64_t minimal_atom_size = 65536,
	std::uint64_t max_sparse_size = 64 * 1024 * 1024
>
class stable_vector : ste_resource_deferred_create_trait {
	static_assert(sizeof(T) <= minimal_atom_size, "minimal_atom_size should be at least the size of a single element");
	static_assert(sizeof(T) % 4 == 0, "T size must be a multiple of 4");

private:
	using buffer_t = device_buffer_sparse<T, minimal_atom_size, device_resource_allocation_policy_device>;
	using bind_range_t = typename buffer_t::bind_range_t;
	static constexpr VkBufferUsageFlags buffer_usage_additional_flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT;

private:
	// Push back command
	class stable_vector_cmd_push_back : public vk_command {
		std::vector<T> data;
		vk_cmd_update_buffer update_cmd;
		stable_vector *v;

	public:
		template <bool sparse>
		stable_vector_cmd_push_back(const vk_buffer<T, sparse> &buffer,
									const std::vector<T> &data_copy,
									stable_vector *v)
			: data(data_copy), update_cmd(buffer, data.size(), data.data(), v->elements), v(v)
		{}
		virtual ~stable_vector_cmd_push_back() noexcept {}

	private:
		void operator()(const vk_command_buffer &command_buffer) const override final {
			// Bind sparse (if needed) and update vector size
			bind_range_t bind = { v->elements, data.size() };
			buffer.cmd_bind_sparse_memory(ste_device_queue::thread_queue(), {}, { bind }, {}, {});
			v->elements += data.size();

			// Copy data
			update_cmd(command_buffer);
		}
	};
	// Pop back command
	class stable_vector_cmd_pop_back : public vk_command {
		stable_vector *v;
		std::uint32_t count_to_pop;

	public:
		stable_vector_cmd_pop_back(std::uint32_t count_to_pop,
								   stable_vector *v)
			: v(v), count_to_pop(count_to_pop)
		{}
		virtual ~stable_vector_cmd_pop_back() noexcept {}

	private:
		void operator()(const vk_command_buffer &command_buffer) const override final {
			assert(count_to_pop >= v->elements);

			// Unbind sparse (if possible) and update vector size
			bind_range_t unbind = { v->elements - count_to_pop, count_to_pop };
			buffer.cmd_bind_sparse_memory(ste_device_queue::thread_queue(), { unbind }, {}, {}, {});

			v->elements -= count_to_pop;
		}
	};

private:
	const ste_context &ctx;
	buffer_t buffer;
	std::size_t elements{ 0 };

public:
	stable_vector(const ste_context &ctx,
				  const VkBufferUsageFlags &usage)
		: ctx(ctx),
		buffer(ctx,
			   max_sparse_size,
			   usage | buffer_usage_additional_flags)
	{}
	stable_vector(const ste_context &ctx,
				  const std::vector<T> &initial_data,
				  const VkBufferUsageFlags &usage)
		: stable_vector(ctx, usage)
	{
		// Copy initial static data
		copy_initial_data(ctx, buffer, initial_data);
	}
	~stable_vector() noexcept {}

	stable_vector(stable_vector&&) = default;
	stable_vector &operator=(stable_vector&&) = default;

	/**
	*	@brief	Returns a device command that will push back data into the vector.
	*			If needed, memory will be bound sprasely to the buffer.
	*
	*	@param	data	Data to push back
	*/
	auto push_back_cmd(const std::vector<T> &data) {
		return stable_vector_cmd_push_back(buffer, data, size(), this);
	}
	/**
	*	@brief	Returns a device command that will push back data into the vector.
	*			If needed, memory will be bound sprasely to the buffer.
	*
	*	@param	data	Data to push back
	*/
	auto push_back_cmd(const T &data) {
		return stable_vector_cmd_push_back(buffer, { data }, size(), this);
	}
	/**
	*	@brief	Returns a device command that will erase some of the elements from the back the vector.
	*			If possible, memory will be unbound sprasely from the buffer.
	*
	*	@param	count_to_pop	Elements count to pop
	*/
	auto pop_back_cmd(std::uint32_t count_to_pop = 1) {
		return stable_vector_cmd_pop_back(count_to_pop, this);
	}
	/**
	*	@brief	Returns a device command that will copy data to the vector.
	*
	*	@param	data	Data to copy
	*	@param	offset	Vector offset to copy to
	*/
	auto update_cmd(const std::vector<T> &data, std::uint64_t offset) {
		return vk_cmd_update_buffer(buffer.get(), data.size(), data.data(), offset);
	}

	auto size() const { return elements; }

	auto& get_buffer() const { return buffer.get(); }
	operator VkBuffer() const { return get_buffer(); }
};

}
}
