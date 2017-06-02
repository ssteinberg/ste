//	StE
// � Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_resource_traits.hpp>
#include <buffer_usage.hpp>
#include <buffer_view.hpp>
#include <copy_data_buffer.hpp>
#include <vector_common.hpp>

#include <cmd_update_buffer.hpp>
#include <cmd_copy_buffer.hpp>
#include <task.hpp>

#include <device_buffer_sparse.hpp>
#include <device_resource_allocation_policy.hpp>

#include <allow_type_decay.hpp>
#include <functional>
#include <lib/blob.hpp>
#include <lib/vector.hpp>

namespace ste {
namespace gl {

template <
	typename T,
	std::uint64_t minimal_atom_size = 65536,
	std::uint64_t max_sparse_size = 64 * 1024 * 1024
>
class vector :
	ste_resource_deferred_create_trait,
	public allow_type_decay<vector<T, minimal_atom_size, max_sparse_size>, device_buffer_sparse<T, minimal_atom_size, device_resource_allocation_policy_device>>
{
	static_assert(sizeof(T) <= minimal_atom_size, "minimal_atom_size should be at least the size of a single element");
	static_assert(sizeof(T) % 4 == 0, "T size must be a multiple of 4");

private:
	using buffer_t = device_buffer_sparse<T, minimal_atom_size, device_resource_allocation_policy_device>;
	using bind_range_t = typename buffer_t::bind_range_t;
	static constexpr auto buffer_usage_additional_flags = buffer_usage::transfer_dst;

public:
	using value_type = T;

private:
	buffer_t buffer;

	lib::vector<T> host_replica;

public:
	vector(const ste_context &ctx,
		   const buffer_usage &usage)
		: buffer(ctx,
				 max_sparse_size,
				 usage | buffer_usage_additional_flags)
	{}
	vector(const ste_context &ctx,
		   const lib::vector<T> &initial_data,
		   const buffer_usage &usage)
		: vector(ctx, usage)
	{
		host_replica = initial_data;

		// Copy initial static data
		_internal::copy_data_buffer_and_resize(ctx, *this, host_replica);
	}
	~vector() noexcept {}

	vector(vector&&) = default;
	vector &operator=(vector&&) = default;

	/**
	*	@brief	Returns a device command that will overwrite slot at index idx with data.
	*
	*	@param	idx		Slot index to overwrite
	*	@param	data	New data to overwrite
	*/
	auto overwrite_cmd(std::uint64_t idx,
					   const lib::vector<T> &data) {
		assert(idx + data.size() <= size());

		std::copy(data.begin(), data.end(), host_replica.begin() + idx);

		return _internal::vector_cmd_update<vector<T, minimal_atom_size, max_sparse_size>>(data,
																						   idx,
																						   this);
	}
	/**
	*	@brief	Returns a device command that will overwrite slot at index idx with data.
	*
	*	@param	idx		Slot index to overwrite
	*	@param	data	New data to overwrite
	*/
	auto overwrite_cmd(std::uint64_t idx,
					   const T &data) {
		return overwrite_cmd(idx, lib::vector<T>{ data });
	}

	/**
	*	@brief	Erases an element and compacts the remains of the vector by moving the tail of the vector.
	*/
	auto erase_and_shift_cmd(std::uint64_t idx,
							 std::uint64_t count = 1) {
		assert(idx + count <= size());

		host_replica.erase(host_replica.begin() + idx, host_replica.begin() + idx + count);

		return overwrite_cmd(idx,
							 lib::vector<T>(host_replica.begin() + idx + count, host_replica.end()));
	}

	/**
	*	@brief	Returns a device command that will push back data into the vector.
	*			If needed, memory will be bound sprasely to the buffer.
	*
	*	@param	data	Data to push back
	*/
	auto push_back_cmd(const lib::vector<T> &data) {
		auto location = host_replica.size();
		std::copy(data.begin(), data.end(), std::back_inserter(host_replica));

		return _internal::vector_cmd_insert<vector<T, minimal_atom_size, max_sparse_size>>(data, location, this);
	}
	/**
	*	@brief	Returns a device command that will push back data into the vector.
	*			If needed, memory will be bound sprasely to the buffer.
	*
	*	@param	data	Data to push back
	*/
	auto push_back_cmd(const T &data) {
		return push_back_cmd(lib::vector<T>{ data });
	}
	/**
	*	@brief	Returns a device command that will erase some of the elements from the back the vector.
	*			If possible, memory will be unbound sprasely from the buffer.
	*
	*	@param	count_to_pop	Elements count to pop
	*/
	auto pop_back_cmd(std::uint64_t count_to_pop = 1) {
		assert(count_to_pop <= size());
		host_replica.erase(host_replica.end() - count_to_pop, host_replica.end());

		return _internal::vector_cmd_unbind<vector<T, minimal_atom_size, max_sparse_size>>(size(), count_to_pop, this);
	}

	/**
	*	@brief	Returns a device command that will resize the vector.
	*			Memory will be bound or unbound sprasely from the buffer, as needed.
	*
	*	@param	new_size		New vector size
	*/
	auto resize_cmd(std::uint64_t new_size) {
		auto old_size = size();
		host_replica.resize(new_size);

		return _internal::vector_cmd_resize<vector<T, minimal_atom_size, max_sparse_size>>(old_size,
																						   new_size,
																						   this);
	}

	auto size() const { return host_replica.size(); }

	auto& get() { return buffer; }
	auto& get() const { return buffer; }
};

}
}
