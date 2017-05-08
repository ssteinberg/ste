//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vk_buffer.hpp>

namespace ste {
namespace gl {

class device_buffer_base {
protected:
	device_buffer_base() = default;

public:
	virtual ~device_buffer_base() noexcept {}

	virtual const vk::vk_buffer& get_buffer_handle() const = 0;

	virtual std::uint64_t get_elements_count() const = 0;
	virtual std::uint32_t get_element_size_bytes() const = 0;
	virtual bool is_sparse() const = 0;
};

}
}
