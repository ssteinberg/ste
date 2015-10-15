// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "FramebufferObject.h"

#include <gli/gli.hpp>

namespace StE {
namespace LLR {

class system_provided_framebuffer;

template<typename A, GLenum color_attachment_point>
class fbo_system_provided_color_attachment_point : public fbo_color_attachment_point<A, color_attachment_point> {
private:
	friend class system_provided_framebuffer;

	using Base = fbo_color_attachment_point<A, color_attachment_point>;

	glm::tvec2<std::size_t> size;
	gli::format format;

	using Base::attach;
	using Base::detach;

protected:
	fbo_system_provided_color_attachment_point(system_provided_framebuffer *fbo, const glm::tvec2<std::size_t> &size, const gli::format &format);

public:
	using Base::fbo_id;
	using Base::attachment_point;

	virtual glm::tvec2<std::size_t> get_attachment_size() const override { return size; }
	virtual gli::format get_attachment_format() const override { return format; }
};

class system_provided_framebuffer_dummy_allocator : public generic_resource_allocator {
public:
	unsigned allocate() override final { return 0; }
	void deallocate(unsigned &id) override final {}
};

class system_provided_framebuffer : protected frame_buffer_object<system_provided_framebuffer_dummy_allocator> {
private:
	template<typename A, GLenum color_attachment_point>
	friend class fbo_system_provided_color_attachment_point;

	using Base = frame_buffer_object<system_provided_framebuffer_dummy_allocator>;
	using front_attachment_point = fbo_system_provided_color_attachment_point<system_provided_framebuffer_dummy_allocator, GL_FRONT>;
	using back_attachment_point = fbo_system_provided_color_attachment_point<system_provided_framebuffer_dummy_allocator, GL_BACK>;

private:
	using Base::unbind;

	front_attachment_point front_color_attchment;
	back_attachment_point back_color_attchment;

public:
	system_provided_framebuffer(const glm::tvec2<std::size_t> &size, const gli::format &format) : 
		Base(), front_color_attchment(this, size, format), back_color_attchment(this, size, format) {}

	system_provided_framebuffer(system_provided_framebuffer &&t) = delete;
	system_provided_framebuffer &operator=(system_provided_framebuffer &&t) = delete;
	system_provided_framebuffer(const system_provided_framebuffer &t) = delete;
	system_provided_framebuffer &operator=(const system_provided_framebuffer &t) = delete;

	front_attachment_point &front_buffer() { return front_color_attchment; }
	back_attachment_point &back_buffer() { return back_color_attchment; }
	const front_attachment_point &front_buffer() const { return front_color_attchment; }
	const back_attachment_point &back_buffer() const { return back_color_attchment; }

	using Base::bind;
	using Base::blit_to;
};

template<typename A, GLenum color_attachment_point>
fbo_system_provided_color_attachment_point<A, color_attachment_point>::fbo_system_provided_color_attachment_point(system_provided_framebuffer *fbo, const glm::tvec2<std::size_t> &size, const gli::format &format) : 
	fbo_color_attachment_point(fbo, 0), size(size), format(format) {}

}
}
