//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <vk_buffer_dense.hpp>
#include <device_buffer_base.hpp>
#include <device_resource.hpp>
#include <device_resource_allocation_policy.hpp>

#include <buffer_usage.hpp>

namespace ste {
namespace gl {

template <typename T, class allocation_policy = device_resource_allocation_policy_device>
class device_buffer
	: public device_buffer_base,
	public device_resource<vk::vk_buffer_dense<>, allocation_policy>
{
public:
	device_buffer(const ste_context &ctx,
				  std::uint64_t count,
				  const buffer_usage &usage)
		: device_resource(ctx,
						  static_cast<std::uint32_t>(sizeof(T)),
						  count,
						  static_cast<VkBufferUsageFlags>(usage))
	{}
	~device_buffer() noexcept {}
	
	const vk::vk_buffer<>& get_buffer_handle() const override final { return *this; }

	device_buffer(device_buffer&&) = default;
	device_buffer &operator=(device_buffer&&) = default;

	std::uint64_t get_elements_count() const override final { return this->get().get_elements_count(); }
	std::uint32_t get_element_size_bytes() const override final { return sizeof(T); };
	bool is_sparse() const override final { return false; };
};

}
}
