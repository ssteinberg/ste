// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "FramebufferObject.hpp"

#include <gli/gli.hpp>

namespace StE {
namespace Core {

class context_framebuffer;

template<typename A, GLenum color_attachment_point>
class context_fbo_color_attachment_point : public fbo_color_attachment_point<A, color_attachment_point> {
private:
	friend class context_framebuffer;

	using Base = fbo_color_attachment_point<A, color_attachment_point>;

	glm::ivec2 size;
	gli::format format;

	using Base::attach;
	using Base::detach;

protected:
	context_fbo_color_attachment_point(context_framebuffer *fbo, const glm::ivec2 &size, const gli::format &format);

public:
	using Base::fbo_id;
	using Base::attachment_point;

	virtual glm::ivec2 get_attachment_size() const override { return size; }
	virtual gli::format get_attachment_format() const override { return format; }
};

class context_framebuffer_dummy_allocator : public generic_resource_allocator {
public:
	GenericResource::type allocate() override final { return 0; }
	static void deallocate(GenericResource::type &id) {}
};

class context_framebuffer : public frame_buffer_object<context_framebuffer_dummy_allocator> {
private:
	template<typename A, GLenum color_attachment_point>
	friend class context_fbo_color_attachment_point;

	using Base = frame_buffer_object<context_framebuffer_dummy_allocator>;
	using front_attachment_point = context_fbo_color_attachment_point<context_framebuffer_dummy_allocator, GL_FRONT>;
	using back_attachment_point = context_fbo_color_attachment_point<context_framebuffer_dummy_allocator, GL_BACK>;

private:
	using Base::unbind;

	front_attachment_point front_color_attchment;
	back_attachment_point back_color_attchment;

public:
	context_framebuffer(const glm::ivec2 &size, const gli::format &format) : 
		Base(), front_color_attchment(this, size, format), back_color_attchment(this, size, format) {}

	context_framebuffer(context_framebuffer &&t) = delete;
	context_framebuffer &operator=(context_framebuffer &&t) = delete;
	context_framebuffer(const context_framebuffer &t) = delete;
	context_framebuffer &operator=(const context_framebuffer &t) = delete;

	front_attachment_point &front_buffer() { return front_color_attchment; }
	back_attachment_point &back_buffer() { return back_color_attchment; }
	const front_attachment_point &front_buffer() const { return front_color_attchment; }
	const back_attachment_point &back_buffer() const { return back_color_attchment; }

	using Base::bind;
	using Base::blit_to;
};

template<typename A, GLenum color_attachment_point>
context_fbo_color_attachment_point<A, color_attachment_point>::context_fbo_color_attachment_point(context_framebuffer *fbo, const glm::ivec2 &size, const gli::format &format) : 
	fbo_color_attachment_point<A, color_attachment_point>(fbo, 0), size(size), format(format) {}

}
}
