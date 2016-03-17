// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "gl_utils.hpp"
#include "surface_constants.hpp"

#include "resource.hpp"
#include "render_target_allocator.hpp"

#include <gli/gli.hpp>

namespace StE {
namespace LLR {

class RenderTargetGeneric {
public:
	virtual glm::ivec2 get_image_size() const = 0;
	virtual gli::format get_format() const = 0;

	virtual ~RenderTargetGeneric() noexcept {}
};

class RenderTarget : public resource<RenderTargetAllocator>, virtual public RenderTargetGeneric {
protected:
	glm::ivec2 size;
	gli::format format;

public:
	RenderTarget(RenderTarget &&m) = default;
	RenderTarget& operator=(RenderTarget &&m) = default;

	RenderTarget(gli::format format, const glm::ivec2 &size, int samples = 0) : size(size), format(format) {
		std::size_t block = gli::block_size(format);
		auto block_extend = gli::block_extent(format);
		std::size_t bytes = (samples == 0 ? 1 : samples) * size.x * size.y * block / (block_extend.x * block_extend.y);
		
		auto swizzle = swizzles_rgba;
		auto glformat = gl_utils::translate_format(format, swizzle);
		allocator.allocate_storage(get_resource_id(), samples, glformat, size, bytes);
	}

	glm::ivec2 get_image_size() const final override { return size; }
	gli::format get_format() const final override { return format; }

	llr_resource_type resource_type() const override { return llr_resource_type::LLRRenderbufferObject; }
};

}
}
