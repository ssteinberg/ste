//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_resource_traits.hpp>
#include <buffer_usage.hpp>
#include <buffer_view.hpp>
#include <copy_data_buffer.hpp>
#include <vector_common.hpp>

#include <device_buffer_sparse.hpp>
#include <device_resource_allocation_policy.hpp>

#include <allow_type_decay.hpp>
#include <lib/vector.hpp>
#include <array>
#include <mutex>

namespace ste {
namespace gl {

template <
	typename T,
	std::uint64_t minimal_atom_size = 65536,
	std::uint64_t max_sparse_size = 1024 * 1024
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

	using insert_cmd_t = _internal::vector_cmd_insert<vector<T, minimal_atom_size, max_sparse_size>>;
	using resize_cmd_t = _internal::vector_cmd_resize<vector<T, minimal_atom_size, max_sparse_size>>;
	using unbind_cmd_t = _internal::vector_cmd_unbind<vector<T, minimal_atom_size, max_sparse_size>>;
	using update_cmd_t = _internal::vector_cmd_update<vector<T, minimal_atom_size, max_sparse_size>>;

private:
	buffer_t buffer;

	lib::vector<T> host_replica;

	mutable std::mutex mutex;

private:
	/**
	 *	@brief	Move constructor that hold a lock
	 */
	vector(std::unique_lock<std::mutex> &&guard, vector &&o) noexcept
		: buffer(std::move(o.buffer)), host_replica(std::move(o.host_replica))
	{}

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
		_internal::copy_data_buffer_and_resize(ctx,
											   *this,
											   host_replica.data(),
											   host_replica.size());
	}
	template <std::size_t N>
	vector(const ste_context &ctx,
		   const std::array<T, N> &initial_data,
		   const buffer_usage &usage)
		: vector(ctx, usage)
	{
		host_replica = lib::vector<T>(initial_data.begin(),
									  initial_data.end());

		// Copy initial static data
		_internal::copy_data_buffer_and_resize(ctx,
											   *this,
											   host_replica.data(),
											   host_replica.size());
	}
	~vector() noexcept {}

	vector(vector &&o) noexcept : vector(std::unique_lock<std::mutex>(o.mutex), std::move(o)) {}
	vector &operator=(vector &&o) noexcept {
		std::unique_lock<std::mutex> lock1(mutex, std::defer_lock);
		std::unique_lock<std::mutex> lock2(o.mutex, std::defer_lock);
		std::lock(lock1, lock2);

		buffer = std::move(o.buffer);
		host_replica = std::move(o.host_replica);

		return *this;
	}

	/**
	*	@brief	Returns a device command that will overwrite slot at index idx with data.
	*
	*	@param	idx		Slot index to overwrite
	*	@param	data	New data to overwrite
	*/
	auto overwrite_cmd(std::uint64_t idx,
					   const lib::vector<T> &data) {

		{
			std::unique_lock<std::mutex> l(mutex);

			assert(idx + data.size() <= size());
			std::copy(data.begin(), data.end(), host_replica.begin() + static_cast<int>(idx));
		}

		return update_cmd_t(data,
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
		lib::vector<T> overwrite_data;
		{
			std::unique_lock<std::mutex> l(mutex);

			assert(idx + count <= size());

			overwrite_data = lib::vector<T>(host_replica.begin() + idx + count, host_replica.end());
			host_replica.erase(host_replica.begin() + idx, host_replica.begin() + static_cast<int>(idx + count));
		}

		return overwrite_cmd(idx,
							 overwrite_data);
	}

	/**
	*	@brief	Returns a device command that will push back data into the vector.
	*			If needed, memory will be bound sprasely to the buffer.
	*
	*	@param	data	Data to push back
	*/
	auto push_back_cmd(const lib::vector<T> &data) {
		std::size_t location;
		{
			std::unique_lock<std::mutex> l(mutex);

			location = size();
			std::copy(data.begin(), data.end(), std::back_inserter(host_replica));
		}

		return insert_cmd_t(data, location, this);
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

		std::size_t location;
		{
			std::unique_lock<std::mutex> l(mutex);

			location = size();
			host_replica.erase(host_replica.end() - count_to_pop, host_replica.end());
		}

		return unbind_cmd_t(location, count_to_pop, this);
	}

	/**
	*	@brief	Returns a device command that will resize the vector.
	*			Memory will be bound or unbound sprasely from the buffer, as needed.
	*
	*	@param	new_size		New vector size
	*/
	auto resize_cmd(std::uint64_t new_size) {
		std::size_t old_size;
		{
			std::unique_lock<std::mutex> l(mutex);

			old_size = size();
			host_replica.resize(new_size);
		}

		return resize_cmd_t(old_size,
							new_size,
							this);
	}

	auto size() const { return host_replica.size(); }

	auto& get() { return buffer; }
	auto& get() const { return buffer; }
};

}
}
