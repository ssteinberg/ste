//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_resource_traits.hpp>
#include <buffer_usage.hpp>
#include <buffer_view.hpp>
#include <host_write_buffer.hpp>
#include <vector_common.hpp>

#include <cmd_update_buffer.hpp>
#include <task.hpp>

#include <device_buffer_sparse.hpp>
#include <device_resource_allocation_policy.hpp>

#include <allow_type_decay.hpp>
#include <lib/vector.hpp>
#include <lib/range_list.hpp>
#include <array>
#include <mutex>
#include <atomic>

namespace ste {
namespace gl {

template <
	typename T,
	std::uint64_t max_sparse_size = 1024 * 1024
>
class stable_vector :
	ste_resource_deferred_create_trait,
	public allow_type_decay<stable_vector<T, max_sparse_size>, device_buffer_sparse<T, device_resource_allocation_policy_device>> {
	static_assert(sizeof(T) % 4 == 0, "T size must be a multiple of 4");

private:
	using buffer_t = device_buffer_sparse<T, device_resource_allocation_policy_device>;
	using bind_range_t = typename buffer_t::bind_range_t;
	static constexpr auto buffer_usage_additional_flags = buffer_usage::transfer_dst;

	using tombstone_ranges_t = lib::range_list<std::uint64_t>;
	using tombstone_range = tombstone_ranges_t::value_type;

public:
	using value_type = T;

	static constexpr bool sparse_container = true;
	static constexpr auto sparse_size = max_sparse_size;

	using insert_cmd_t = _internal::vector_cmd_insert<stable_vector<T, max_sparse_size>>;
	using resize_cmd_t = _internal::vector_cmd_resize<stable_vector<T, max_sparse_size>>;
	using unbind_cmd_t = _internal::vector_cmd_unbind<stable_vector<T, max_sparse_size>>;
	using update_cmd_t = _internal::vector_cmd_update<stable_vector<T, max_sparse_size>>;

private:
	buffer_t buffer;
	std::atomic<std::uint64_t> elements{ 0 };

	tombstone_ranges_t tombstones;

	mutable std::mutex tombstones_mutex;

public:
	stable_vector(const ste_context &ctx,
				  const buffer_usage &usage,
				  const char *name)
		: buffer(ctx,
				 max_sparse_size,
				 usage | buffer_usage_additional_flags,
				 name) {}

	stable_vector(const ste_context &ctx,
				  const lib::vector<T> &initial_data,
				  const buffer_usage &usage,
				  const char *name)
		: stable_vector(ctx, usage, name) {
		// Copy initial static data
		_internal::host_write_buffer(ctx,
									 *this,
									 initial_data.data(),
									 initial_data.size());
		elements.store(initial_data.size());
	}

	template <std::size_t N>
	stable_vector(const ste_context &ctx,
				  const std::array<T, N> &initial_data,
				  const buffer_usage &usage,
				  const char *name)
		: stable_vector(ctx, usage, name) {
		// Copy initial static data
		_internal::host_write_buffer(ctx,
									 *this,
									 initial_data.data(),
									 initial_data.size());
		elements.store(initial_data.size());
	}

	~stable_vector() noexcept {}

	stable_vector(stable_vector &&o) noexcept : buffer(std::move(o.buffer)), elements(o.elements.load()) {
		std::unique_lock<std::mutex> lt(o.tombstones_mutex);
		tombstones = std::move(o.tombstones);
	}

	stable_vector &operator=(stable_vector &&o) noexcept {
		std::unique_lock<std::mutex> lock1(tombstones_mutex, std::defer_lock);
		std::unique_lock<std::mutex> lock2(o.tombstones_mutex, std::defer_lock);
		std::lock(lock1, lock2);

		buffer = std::move(o.buffer);
		elements.store(o.elements.load());
		tombstones = std::move(o.tombstones);

		return *this;
	}

	/**
	*	@brief	Marks an element with a tombstone. Tombstones can be consumed by calling insert(), overwriting the tombstone
	*			with new data.
	*/
	void tombstone(std::uint64_t idx,
				   std::uint64_t count = 1) {
		assert(idx + count <= size());

		std::unique_lock<std::mutex> lt(tombstones_mutex);

		const tombstone_range t(idx, count);
		tombstones.add(t);
	}

	/**
	*	@brief	Returns a device command that will insert data into the vector in an empty slot.
	*			If needed, memory will be bound sprasely to the buffer.
	*
	*	@param	data		Data to insert
	*	@param	location	Outputs the insertion index
	*/
	auto insert_cmd(const lib::vector<T> &data,
					std::uint64_t &location) {
		// If there are tombstones, replace one of them with new element, if possible
		std::unique_lock<std::mutex> lt(tombstones_mutex);

		auto it = tombstones.begin();
		for (; it != tombstones.end(); ++it) {
			if (it->length >= data.size())
				break;
		}

		if (it != tombstones.end()) {
			location = it->start;

			const tombstone_range t(location, data.size());
			tombstones.remove(t);

			lt.unlock();

			return insert_cmd_t(data,
								location,
								this);
		}

		lt.unlock();

		location = elements.fetch_add(data.size());
		return insert_cmd_t(data, location, this);
	}

	/**
	*	@brief	Returns a device command that will insert data into the vector in an empty slot.
	*			If needed, memory will be bound sprasely to the buffer.
	*
	*	@param	data		Data to insert
	*	@param	location	Outputs the insertion index
	*/
	auto insert_cmd(const T &data,
					std::uint64_t &location) {
		return insert_cmd(lib::vector<T>{ data }, location);
	}

	/**
	*	@brief	Returns a device command that will overwrite slot at index idx with data.
	*
	*	@param	idx		Slot index to overwrite
	*	@param	data	New data to overwrite
	*/
	auto overwrite_cmd(std::uint64_t idx,
					   const lib::vector<T> &data) {
		assert(idx + data.size() <= size());

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
	*	@brief	Returns a device command that will push back data into the vector.
	*			If needed, memory will be bound sprasely to the buffer.
	*
	*	@param	data	Data to push back
	*/
	auto push_back_cmd(const lib::vector<T> &data) {
		const auto location = elements.fetch_add(data.size());

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
		const auto location = elements.fetch_add(-count_to_pop);
		assert(static_cast<std::int64_t>(location) > 0);

		return unbind_cmd_t(location, count_to_pop, this);
	}

	/**
	*	@brief	Returns a device command that will resize the vector.
	*			Memory will be bound or unbound sprasely from the buffer, as needed.
	*
	*	@param	new_size		New vector size
	*/
	auto resize_cmd(std::uint64_t new_size) {
		const auto old_size = elements.exchange(new_size);

		return resize_cmd_t(old_size,
							new_size,
							this);
	}

	auto size() const { return elements.load(); }

	auto &get() { return buffer; }
	auto &get() const { return buffer; }
};

}
}
