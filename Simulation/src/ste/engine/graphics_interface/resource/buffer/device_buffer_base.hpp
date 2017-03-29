//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <device_resource_queue_transferable.hpp>

namespace StE {
namespace GL {

class device_buffer_base 
	: public device_resource_queue_transferable {
protected:
	device_buffer_base(const device_resource_queue_ownership::family_t &family)
		: device_resource_queue_transferable(family)
	{}

public:
	virtual ~device_buffer_base() noexcept {}
	virtual VkBuffer get_buffer_handle() const = 0;

	device_buffer_base(device_buffer_base&&) = default;
	device_buffer_base &operator=(device_buffer_base&&) = default;

	virtual std::uint64_t get_elements_count() const = 0;
	virtual std::uint32_t get_element_size_bytes() const = 0;
	virtual bool is_sparse() const = 0;
};

}
}
