//	StE
// � Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_resource.hpp>
#include <host_write_buffer.hpp>
#include <vector_common.hpp>

#include <buffer_usage.hpp>
#include <device_buffer.hpp>
#include <device_resource_allocation_policy.hpp>

#include <task.hpp>

#include <array>
#include <lib/vector.hpp>
#include <allow_type_decay.hpp>
#include <lib/blob.hpp>

namespace ste {
namespace gl {

template <
	typename T,
	class allocation_policy = device_resource_allocation_policy_device
>
class array : ste_resource_deferred_create_trait, public allow_type_decay<array<T, allocation_policy>, device_buffer<T, allocation_policy>> {
//	static_assert(sizeof(T) % 4 == 0, "T size must be a multiple of 4");

private:
	using buffer_t = device_buffer<T, allocation_policy>;

	static constexpr auto buffer_usage_additional_flags = buffer_usage::transfer_dst;

public:
	using value_type = T;

	using update_cmd_t = _internal::vector_cmd_update_buffer<array<T, allocation_policy>>;

private:
	buffer_t buffer;

public:
	array(const ste_context &ctx,
		  std::uint64_t count,
		  const buffer_usage &usage,
		  const char *name)
		: buffer(ctx,
				 count,
				 usage | buffer_usage_additional_flags,
				 name) {}

	array(const ste_context &ctx,
		  std::uint64_t count,
		  const lib::vector<T> &initial_data,
		  const buffer_usage &usage,
		  const char *name)
		: array(ctx, count, usage, name) {
		// Copy initial static data
		_internal::host_write_buffer(ctx,
									 *this,
									 initial_data.data(),
									 initial_data.size());
	}

	array(const ste_context &ctx,
		  const lib::vector<T> &initial_data,
		  const buffer_usage &usage,
		  const char *name)
		: array(ctx, initial_data.size(), initial_data, usage, name) {}

	template <std::size_t N>
	array(const ste_context &ctx,
		  std::uint64_t count,
		  const std::array<T, N> &initial_data,
		  const buffer_usage &usage,
		  const char *name)
		: array(ctx, count, usage, name) {
		// Copy initial static data
		_internal::host_write_buffer(ctx,
									 *this,
									 initial_data.data(),
									 initial_data.size());
	}

	template <std::size_t N>
	array(const ste_context &ctx,
		  const std::array<T, N> &initial_data,
		  const buffer_usage &usage,
		  const char *name)
		: array(ctx, initial_data.size(), initial_data, usage, name) {}

	~array() noexcept {}

	array(array &&o) = default;
	array &operator=(array &&) = default;

	/**
	*	@brief	Returns a device command that will overwrite slot at index idx with data.
	*
	*	@param	idx		Slot index to overwrite
	*	@param	data	New data to overwrite
	*	@param	size	Element count of new data
	*/
	auto overwrite_cmd(std::uint64_t idx,
					   const T *data,
					   std::size_t size) {
		assert(idx + size <= this->size());

		return update_cmd_t(data,
							size,
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
					   const lib::vector<T> &data) {
		return overwrite_cmd(idx,
							 data.data(),
							 data.size());
	}

	/**
	*	@brief	Returns a device command that will overwrite slot at index idx with data.
	*
	*	@param	idx		Slot index to overwrite
	*	@param	data	New data to overwrite
	*/
	auto overwrite_cmd(std::uint64_t idx,
					   const T &data) {
		return overwrite_cmd(idx, &data, 1);
	}

	auto size() const { return buffer.get().get_elements_count(); }

	auto &get() { return buffer; }
	auto &get() const { return buffer; }
};

}
}
