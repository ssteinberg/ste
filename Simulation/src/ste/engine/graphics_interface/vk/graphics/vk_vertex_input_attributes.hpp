// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

namespace StE {
namespace GL {

class vk_vertex_input_attributes {
public:
	std::size_t offset_to_attrib(int attrib) const noexcept {
		int offset, i;
		for (offset = i = 0; i < attrib; offset += attrib_size(i), ++i) {}
		return offset;
	}

public:
	virtual ~vk_vertex_input_attributes() noexcept {}

	virtual std::size_t attrib_count() const noexcept = 0;
	virtual std::size_t stride() const noexcept = 0;
	virtual std::size_t attrib_size(int attrib) const noexcept = 0;
	virtual VkFormat	attrib_format(int attrib) const noexcept = 0;
};

}
}
