// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "gl_utils.h"

#include "resource.h"
#include "render_target_allocator.h"

#include <gli/gli.hpp>

namespace StE {
namespace LLR {

class RenderTargetGeneric {
public:
	virtual glm::tvec2<std::size_t> get_image_size() const = 0;
	virtual gli::format get_format() const = 0;
};

class RenderTarget : public resource<RenderTargetAllocator>, virtual public RenderTargetGeneric {
protected:
	glm::tvec2<std::size_t> size;
	gli::format format;

public:
	RenderTarget(RenderTarget &&m) = default;
	RenderTarget& operator=(RenderTarget &&m) = default;

	RenderTarget(gli::format format, const glm::tvec2<std::size_t> &size, int samples = 0) : size(size), format(format) {
		auto glformat = gl_utils::translate_format(format);
		allocator.allocate_storage(get_resource_id(), samples, glformat, size);
	}

	glm::tvec2<std::size_t> get_image_size() const final override { return size; }
	gli::format get_format() const final override { return format; }

	llr_resource_type resource_type() const override { return llr_resource_type::LLRRenderbufferObject; }
};

}
}
