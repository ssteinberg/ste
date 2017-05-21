//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_resource.hpp>
#include <copy_data_buffer.hpp>

#include <buffer_usage.hpp>
#include <buffer_view.hpp>
#include <device_buffer.hpp>
#include <device_resource_allocation_policy.hpp>

#include <task.hpp>
#include <cmd_update_buffer.hpp>

#include <lib/vector.hpp>
#include <allow_type_decay.hpp>
#include <functional>
#include <lib/blob.hpp>

namespace ste {
namespace gl {

template <typename T>
class array : ste_resource_deferred_create_trait, public allow_type_decay<array<T>, device_buffer<T, device_resource_allocation_policy_device>> {
	static_assert(sizeof(T) % 4 == 0, "T size must be a multiple of 4");

private:
	using buffer_t = device_buffer<T, device_resource_allocation_policy_device>;

	static constexpr auto buffer_usage_additional_flags = buffer_usage::transfer_dst;

private:
	buffer_t buffer;

public:
	array(const ste_context &ctx,
		  std::uint64_t count,
		  const buffer_usage &usage)
		: buffer(ctx,
				 count,
				 usage | buffer_usage_additional_flags)
	{}
	array(const ste_context &ctx,
		  std::uint64_t count,
		  const lib::vector<T> &initial_data,
		  const buffer_usage &usage)
		: array(ctx, count, usage)
	{
		// Copy initial static data
		_internal::copy_data_buffer(ctx, buffer, initial_data);
	}
	array(const ste_context &ctx,
		  const lib::vector<T> &initial_data,
		  const buffer_usage &usage)
		: array(ctx, initial_data.size(), initial_data, usage)
	{}
	~array() noexcept {}

	array(array &&o) = default;
	array &operator=(array&&) = default;

	/**
	*	@brief	Returns a device command that will copy data to the array.
	*
	*	@param	data	Data to copy
	*	@param	offset	Array offset to copy to
	*/
	auto update_task(const lib::vector<T> &data,
					 std::uint64_t offset) {
		assert(data.size() + offset <= size() && "Out-of-bounds");

		// Store copy of data
		lib::blob bin(data);

		// Create the task
		auto t = task<cmd_update_buffer>();
		t.attach_dst_buffer_view(buffer_view(buffer,
											 offset,
											 data.size()));

		return std::bind(std::move(t), std::move(bin));
	}

	auto size() const { return buffer.get().get_elements_count(); }

	auto& get() { return buffer; }
	auto& get() const { return buffer; }
};

}
}
