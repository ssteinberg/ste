//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_resource.hpp>
#include <data_structure_common.hpp>

#include <buffer_usage.hpp>
#include <buffer_view.hpp>
#include <device_buffer.hpp>
#include <device_resource_allocation_policy.hpp>

#include <cmd_update_buffer.hpp>

#include <vector>
#include <allow_type_decay.hpp>

namespace ste {
namespace gl {

template <typename T>
class array : ste_resource_deferred_create_trait, public allow_type_decay<array<T>, device_buffer<T, device_resource_allocation_policy_device>> {
	static_assert(sizeof(T) % 4 == 0, "T size must be a multiple of 4");

private:
	using buffer_t = device_buffer<T, device_resource_allocation_policy_device>;

	static constexpr auto buffer_usage_additional_flags = buffer_usage::transfer_dst;

private:
	const ste_context &ctx;
	buffer_t buffer;

public:
	array(const ste_context &ctx,
		  std::uint64_t count,
		  const buffer_usage &usage)
		: ctx(ctx),
		buffer(ctx,
			   count,
			   usage | buffer_usage_additional_flags)
	{}
	array(const ste_context &ctx,
		  std::uint64_t count,
		  const std::vector<T> &initial_data,
		  const buffer_usage &usage)
		: array(ctx, count, usage)
	{
		// Copy initial static data
		copy_initial_data(ctx, buffer, initial_data);
	}
	array(const ste_context &ctx,
		  const std::vector<T> &initial_data,
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
	auto update_cmd(const std::vector<T> &data, 
					std::uint64_t offset = 0) {
		assert(data.size() + offset <= size() && "Out-of-bounds");
		return cmd_update_buffer(buffer_view(buffer, 
											 offset, 
											 data.size()), 
								 data.size() * sizeof(T), data.data());
	}

	auto size() const { return buffer.get().get_elements_count(); }

	auto& get() { return buffer; }
	auto& get() const { return buffer; }
};

}
}
