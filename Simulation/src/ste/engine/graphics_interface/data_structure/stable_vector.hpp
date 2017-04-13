//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_resource.hpp>
#include <buffer_view.hpp>
#include <data_structure_common.hpp>

#include <command_recorder.hpp>
#include <cmd_update_buffer.hpp>

#include <device_buffer_sparse.hpp>
#include <device_resource_allocation_policy.hpp>

#include <allow_type_decay.hpp>

namespace StE {
namespace GL {

template <
	typename T,
	std::uint64_t minimal_atom_size = 65536,
	std::uint64_t max_sparse_size = 64 * 1024 * 1024
>
class stable_vector : 
	ste_resource_deferred_create_trait, 
	public allow_type_decay<stable_vector<T, minimal_atom_size, max_sparse_size>, device_buffer_sparse<T, minimal_atom_size, device_resource_allocation_policy_device>>
{
	static_assert(sizeof(T) <= minimal_atom_size, "minimal_atom_size should be at least the size of a single element");
	static_assert(sizeof(T) % 4 == 0, "T size must be a multiple of 4");

private:
	using buffer_t = device_buffer_sparse<T, minimal_atom_size, device_resource_allocation_policy_device>;
	using bind_range_t = typename buffer_t::bind_range_t;
	static constexpr VkBufferUsageFlags buffer_usage_additional_flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT;

private:
	// Resize command
	class stable_vector_cmd_resize : public command {
		std::uint32_t new_size;
		stable_vector *v;

	public:
		stable_vector_cmd_resize(std::uint32_t new_size,
									stable_vector *v)
			: new_size(new_size), v(v)
		{}
		virtual ~stable_vector_cmd_resize() noexcept {}

	private:
		void operator()(const command_buffer &, command_recorder &recorder) const override final {
			// (Un)bind sparse (if needed) and update vector size
			if (new_size > v->elements) {
				bind_range_t bind = { v->elements, new_size - v->elements };
				recorder << v->buffer.cmd_bind_sparse_memory({}, { bind }, {}, {});
			}
			else if (new_size < v->elements) {
				bind_range_t unbind = { new_size,  v->elements - new_size };
				recorder << v->buffer.cmd_bind_sparse_memory({ unbind }, {}, {}, {});
			}
			
			auto vptr = v;
			auto new_size = this->new_size;
			recorder << host_command([=](const vk_queue &) { vptr->elements = new_size; });
		}
	};
	// Push back command
	class stable_vector_cmd_push_back : public command {
		std::vector<T> data;
		cmd_update_buffer update_cmd;
		stable_vector *v;

	public:
		stable_vector_cmd_push_back(const std::vector<T> &data_copy,
									stable_vector *v)
			: data(data_copy), update_cmd(buffer_view(v->buffer, 
													  v->elements, 
													  data.size()), 
										  data.size(), data.data()), v(v)
		{}
		virtual ~stable_vector_cmd_push_back() noexcept {}

	private:
		void operator()(const command_buffer &, command_recorder &recorder) const override final {
			// Bind sparse (if needed) and update vector size
			bind_range_t bind = { v->elements, data.size() };
			recorder << v->buffer.cmd_bind_sparse_memory({}, { bind }, {}, {});

			auto vptr = v;
			auto data_size = data.size();
			recorder << host_command([=](const vk_queue &) { vptr->elements += data_size; });

			// Copy data
			recorder << update_cmd;
		}
	};
	// Pop back command
	class stable_vector_cmd_pop_back : public command {
		stable_vector *v;
		std::uint32_t count_to_pop;

	public:
		stable_vector_cmd_pop_back(std::uint32_t count_to_pop,
								   stable_vector *v)
			: v(v), count_to_pop(count_to_pop)
		{}
		virtual ~stable_vector_cmd_pop_back() noexcept {}

	private:
		void operator()(const command_buffer &, command_recorder &recorder) const override final {
			assert(count_to_pop >= v->elements);

			// Unbind sparse (if possible) and update vector size
			bind_range_t unbind = { v->elements - count_to_pop, count_to_pop };
			recorder << v->buffer.cmd_bind_sparse_memory({ unbind }, {}, {}, {});

			auto vptr = v;
			auto pop = count_to_pop;
			recorder << host_command([=](const vk_queue &) { vptr->elements -= pop; });
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
			   make_sparse_binding_queue_selector(),
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
		return stable_vector_cmd_push_back(data, this);
	}
	/**
	*	@brief	Returns a device command that will push back data into the vector.
	*			If needed, memory will be bound sprasely to the buffer.
	*
	*	@param	data	Data to push back
	*/
	auto push_back_cmd(const T &data) {
		return stable_vector_cmd_push_back({ data }, this);
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
	*	@brief	Returns a device command that will resize the vector.
	*			Memory will be bound or unbound sprasely from the buffer, as needed.
	*
	*	@param	new_size		New vector size
	*/
	auto resize_cmd(std::uint32_t new_size) {
		return stable_vector_cmd_resize(new_size, this);
	}
	/**
	*	@brief	Returns a device command that will copy data to the vector.
	*
	*	@param	data	Data to copy
	*	@param	offset	Vector offset to copy to
	*/
	auto update_cmd(const std::vector<T> &data, std::uint64_t offset) {
		return cmd_update_buffer(buffer_view(buffer, 
											 offset, 
											 data.size()), 
								 data.size(), data.data());
	}

	auto size() const { return elements; }

	auto& get() { return buffer; }
	auto& get() const { return buffer; }
};

}
}
