//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_resource.hpp>
#include <data_structure_common.hpp>

#include <device_buffer.hpp>
#include <device_resource_allocation_policy.hpp>

#include <vk_cmd_update_buffer.hpp>

#include <vector>

namespace StE {
namespace GL {

template <typename T>
class array : ste_resource_deferred_create_trait {
	static_assert(sizeof(T) % 4 == 0, "T size must be a multiple of 4");

private:
	using buffer_t = device_buffer<T, device_resource_allocation_policy_device>;

	static constexpr VkBufferUsageFlags buffer_usage_additional_flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT;

private:
	const ste_context &ctx;
	buffer_t buffer;

public:
	array(const ste_context &ctx,
		  std::uint64_t count,
		  const VkBufferUsageFlags &usage)
		: ctx(ctx),
		buffer(ctx,
			   make_data_queue_selector(),
			   count,
			   usage | buffer_usage_additional_flags)
	{}
	array(const ste_context &ctx,
		  std::uint64_t count,
		  const std::vector<T> &initial_data,
		  const VkBufferUsageFlags &usage)
		: array(ctx, count, usage)
	{
		// Copy initial static data
		copy_initial_data(ctx, buffer, initial_data);
	}
	array(const ste_context &ctx,
		  const std::vector<T> &initial_data,
		  const VkBufferUsageFlags &usage)
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
		return vk_cmd_update_buffer(buffer.get(), data.size(), data.data(), offset);
	}

	auto size() const { return buffer.get().get_elements_count(); }

	auto& get_buffer() { return buffer; }
	auto& get_buffer() const { return buffer; }
	operator VkBuffer() const { return *get_buffer(); }
};

}
}
