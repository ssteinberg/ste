// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <gl_utils.hpp>
#include <surface_constants.hpp>

#include <resource.hpp>
#include <render_target_allocator.hpp>

namespace StE {
namespace Core {

class render_target_generic {
public:
	virtual glm::ivec2 get_image_size() const = 0;
	virtual gli::format get_format() const = 0;

	virtual ~render_target_generic() noexcept {}
};

class render_target : public resource<render_target_allocator>, virtual public render_target_generic {
protected:
	glm::ivec2 size;
	gli::format format;

public:
	render_target(render_target &&m) = default;
	render_target& operator=(render_target &&m) = default;

	render_target(gli::format format, const glm::ivec2 &size, int samples = 0) : size(size), format(format) {
		std::size_t block = gli::block_size(format);
		auto block_extend = gli::block_extent(format);
		std::size_t bytes = (samples == 0 ? 1 : samples) * size.x * size.y * block / (block_extend.x * block_extend.y);

		auto swizzle = swizzles_rgba;
		auto glformat = GL::gl_utils::translate_format(format, swizzle);
		allocator.allocate_storage(get_resource_id(), samples, glformat, size, bytes);
	}

	glm::ivec2 get_image_size() const final override { return size; }
	gli::format get_format() const final override { return format; }

	core_resource_type resource_type() const override { return core_resource_type::RenderbufferObject; }
};

}
}
