// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "gl_utils.hpp"
#include "gl_current_context.hpp"

#include "resource.hpp"
#include "resource_traits.hpp"
#include "bindable_resource.hpp"
#include "layout_binding.hpp"

#include "image.hpp"
#include "texture_base.hpp"
#include "render_target.hpp"
#include "texture_traits.hpp"
#include "surface_constants.hpp"


#include <memory>
#include <tuple>
#include <vector>
#include <unordered_map>

namespace StE {
namespace Core {

template<typename A>
class framebuffer_object_common;

template<typename A>
class fbo_attachment_point {
private:
	friend class framebuffer_object;

	glm::ivec2 size;
	gli::format format;

protected:
	framebuffer_object_common<A> *fbo;
	GLenum attachment_point;

	fbo_attachment_point(fbo_attachment_point &&t) = delete;
	fbo_attachment_point &operator=(fbo_attachment_point &&t) = delete;
	fbo_attachment_point(const fbo_attachment_point &t) = delete;
	fbo_attachment_point &operator=(const fbo_attachment_point &t) = delete;

	fbo_attachment_point(framebuffer_object_common<A> *fbo, GLenum attachment_point) : fbo(fbo), attachment_point(attachment_point) {}

public:
	virtual ~fbo_attachment_point() noexcept {}

	void detach() {
		glNamedFramebufferTexture(fbo->get_resource_id(), attachment_point, 0, 0);
		size = { 0,0 };
	}

	template <core_resource_type TexType>
	void attach(const image<TexType> &img, std::enable_if_t<texture_is_array<TexType>::value>* = nullptr) {
		glNamedFramebufferTextureLayer(fbo->get_resource_id(), attachment_point, img.get_resource_id(), img.get_level(), img.get_layer());
		size = img.get_image_size();
		format = img.get_format();
	}
	template <core_resource_type TexType>
	void attach(const image<TexType> &img, std::enable_if_t<!texture_is_array<TexType>::value>* = nullptr) {
		glNamedFramebufferTexture(fbo->get_resource_id(), attachment_point, img.get_resource_id(), img.get_level());
		size = img.get_image_size();
		format = img.get_format();
	}
	void attach(const render_target &rt) {
		glNamedFramebufferRenderbuffer(fbo->get_resource_id(), attachment_point, GL_RENDERBUFFER, rt.get_resource_id());
		size = rt.get_image_size();
		format = rt.get_format();
	}
	template <core_resource_type TexType>
	void attach(const texture_mipmapped<TexType> &tex, unsigned level = 0) {
		glNamedFramebufferTexture(fbo->get_resource_id(), attachment_point, tex.get_resource_id(), level);
		size = { tex.get_image_size().x, tex.get_image_size().y };
		format = tex.get_format();
	}

	template<typename T>
	void operator=(const T &target) { attach(target); }

	generic_resource::type fbo_id() const { return fbo->get_resource_id(); }
	GLenum get_attachment_point() const { return attachment_point; }

	virtual glm::ivec2 get_attachment_size() const { return size; }
	virtual gli::format get_attachment_format() const { return format; }

	bool is_attached() const { return size.x > 0; }
};

template<typename A, GLenum color_attachment_point = GL_COLOR_ATTACHMENT0>
class fbo_color_attachment_point : public fbo_attachment_point<A> {
private:
	friend class framebuffer_object;

protected:
	using Base = fbo_attachment_point<A>;
	using Base::fbo;
	fbo_color_attachment_point(framebuffer_object_common<A> *fbo, unsigned index) : Base(fbo, color_attachment_point + index) {}

public:
	virtual ~fbo_color_attachment_point() noexcept {}

	void read_pixels(void *data, int data_size, const glm::uvec2 &rect_size, const glm::uvec2 &origin = { 0, 0 }) const {
		auto gl_format = GL::gl_utils::translate_format(Base::get_attachment_format(), swizzles_rgba);

		fbo->bind_read();
		glReadBuffer(Base::get_attachment_point());

		glReadnPixels(origin.x, origin.y, rect_size.x, rect_size.y, gl_format.External, gl_format.Type, data_size, data);

		fbo->unbind_read();
		fbo->update_draw_buffers();
	}
	void read_pixels(void *data, int data_size) const { read_pixels(data, data_size, Base::get_attachment_size()); }

	void write_pixels(void *data, const glm::uvec2 &rect_size, const glm::uvec2 &origin = { 0, 0 }) {
		auto gl_format = GL::gl_utils::translate_format(Base::get_attachment_format(), swizzles_rgba);

		fbo->bind_write();
		glWriteBuffer(Base::get_attachment_point());

		if (origin.x || origin.y) glRasterPos2f(origin.x, origin.y);
		glDrawPixels(rect_size.x, rect_size.y, gl_format.External, gl_format.Type, data);
		if (origin.x || origin.y) glRasterPos2f(.0f, .0f);

		fbo->unbind_write();
		fbo->update_draw_buffers();
	}
	void write_pixels(void *data) { write_pixels(data, Base::get_attachment_size()); }

	template <typename ... Ts>
	void attach(Ts&&...args) { Base::attach(std::forward<Ts>(args)...); }
	template<typename T>
	void operator=(const T &target) { *(static_cast<fbo_attachment_point<A>*>(this)) = target; }
};

class fbo_color_attachment_layout_binding_type {};
using color_layout_binding = layout_binding<fbo_color_attachment_layout_binding_type>;
color_layout_binding inline operator "" _color_idx(unsigned long long int i) { return color_layout_binding(i); }

template<typename A>
class fbo_layout_bindable_color_attachment_point : public fbo_color_attachment_point<A>, public shader_layout_bindable_resource<fbo_color_attachment_layout_binding_type> {
private:
	friend class framebuffer_object;
	struct emplace_helper {};

	using Base = fbo_color_attachment_point<A>;

private:
	unsigned index;

	void unbind(const LayoutLocationType &) const final override {};

protected:
	using fbo_color_attachment_point<A>::fbo_color_attachment_point;
	fbo_layout_bindable_color_attachment_point(framebuffer_object_common<A> *fbo, unsigned index) : fbo_color_attachment_point<A>(fbo, index), index(index) {}

public:
	template <typename ... Ts>
	fbo_layout_bindable_color_attachment_point(const emplace_helper &, Ts ... args) : fbo_layout_bindable_color_attachment_point(std::forward<Ts>(args)...) {}
	virtual ~fbo_layout_bindable_color_attachment_point() noexcept {}

	void bind(const LayoutLocationType &binding) const final override {
		Base::fbo->set_color_output_binding_index(index, binding);
	}

	template<typename T>
	void operator=(const T &target) {
		*(dynamic_cast<fbo_attachment_point<A>*>(this)) = target;
	}
};

class framebuffer_object_allocator : public generic_resource_allocator {
public:
	generic_resource::type allocate() override final {
		GLuint id;
		glCreateFramebuffers(1, &id);
		return id;
	}
	static void deallocate(generic_resource::type &id) {
		if (id) {
			glDeleteFramebuffers(1, &id);
			id = 0;
		}
	}
};

class framebuffer_object_binder {
public:
	static void bind(generic_resource::type id) { GL::gl_current_context::get()->bind_framebuffer(GL_FRAMEBUFFER, id); }
	static void unbind() { GL::gl_current_context::get()->bind_framebuffer(GL_FRAMEBUFFER, 0); }
};

class framebuffer_object_generic {
public:
	virtual void bind() const = 0;
	virtual void unbind() const = 0;

	virtual ~framebuffer_object_generic() noexcept {}
};

template <class Allocator>
class framebuffer_object_common : public bindable_resource<Allocator, framebuffer_object_binder>, virtual public framebuffer_object_generic {
private:
	friend class fbo_attachment_point<Allocator>;
	template<class A, GLenum t>
	friend class fbo_color_attachment_point;
	friend class fbo_layout_bindable_color_attachment_point<Allocator>;

	using Base = bindable_resource<Allocator, framebuffer_object_binder>;

private:
	std::vector<GLenum> draw_buffers;

protected:
	void set_color_output_binding_index(unsigned attachment_index, const color_layout_binding &binding) {
		unsigned color_output = binding == layout_binding_none<fbo_color_attachment_layout_binding_type>() ?
									GL_NONE :
									GL_COLOR_ATTACHMENT0 + binding.binding_index();

		if (draw_buffers.size() < attachment_index + 1) draw_buffers.resize(attachment_index + 1, GL_NONE);
		bool changed = draw_buffers[attachment_index] != color_output;
		draw_buffers[attachment_index] = color_output;

		if (changed) {
			for (unsigned i = 0; i < draw_buffers.size(); ++i) {
				if (i == attachment_index) continue;
				if (draw_buffers[i] == color_output) draw_buffers[i] = GL_NONE;
			}
			update_draw_buffers();
		}
	}

	void update_draw_buffers() {
		if (draw_buffers.size())
			glNamedFramebufferDrawBuffers(Base::get_resource_id(), draw_buffers.size(), &draw_buffers[0]);
	}

	void bind_draw() const { GL::gl_current_context::get()->bind_framebuffer(GL_DRAW_FRAMEBUFFER, Base::get_resource_id()); }
	void unbind_draw() const { GL::gl_current_context::get()->bind_framebuffer(GL_DRAW_FRAMEBUFFER, 0); }
	void bind_read() const { GL::gl_current_context::get()->bind_framebuffer(GL_READ_FRAMEBUFFER, Base::get_resource_id()); }
	void unbind_read() const { GL::gl_current_context::get()->bind_framebuffer(GL_READ_FRAMEBUFFER, 0); }

public:
	virtual ~framebuffer_object_common() noexcept {}

	framebuffer_object_common(framebuffer_object_common &&t) = default;
	framebuffer_object_common &operator=(framebuffer_object_common &&t) = default;

	void bind() const override final {
		Base::bind();
	}
	void unbind() const override final { Base::unbind(); }

	framebuffer_object_common() {}
	template <typename ... Ts>
	framebuffer_object_common(Ts ... draw_buffer_args) : draw_buffers(std::forward<Ts>(draw_buffer_args)...) {}

	bool is_fbo_complete() const {
		auto status = get_status_code();
		return Base::is_valid() && status == GL_FRAMEBUFFER_COMPLETE;
	}
	GLenum get_status_code() const {
		return glCheckNamedFramebufferStatus(Base::get_resource_id(), GL_FRAMEBUFFER);
	}

	template <class A>
	void blit_to(framebuffer_object_common<A> &fbo, const glm::ivec2 &src_size, const glm::ivec2 dst_size, const glm::ivec2 &src_origin = { 0,0 }, const glm::ivec2 dst_origin = { 0,0 }, bool linear_filter = true, bool blit_color = true, bool blit_depth = false) const {
		auto src_xy1 = src_origin + src_size;
		auto dst_xy1 = dst_origin + dst_size;
		int mask = 0;
		if (blit_depth) mask |= GL_DEPTH_BUFFER_BIT;
		if (blit_color) mask |= GL_COLOR_BUFFER_BIT;
		glBlitNamedFramebuffer(Base::get_resource_id(), fbo.get_resource_id(), src_origin.x, src_origin.y, src_xy1.x, src_xy1.y, dst_origin.x, dst_origin.y, dst_xy1.x, dst_xy1.y, mask, linear_filter ? GL_LINEAR : GL_NEAREST);
	}

	static auto max_framebuffer_bindings() {
		GLuint maxbuffers;
		glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxbuffers);
		return maxbuffers;
	}

	core_resource_type resource_type() const override { return core_resource_type::FramebufferObject; }
};

class framebuffer_object : public framebuffer_object_common<framebuffer_object_allocator> {
private:
	using Allocator = framebuffer_object_allocator;

	template<class A>
	friend class fbo_layout_bindable_color_attachment_point;

	using Base = framebuffer_object_common<framebuffer_object_allocator>;
	using depth_attachment_binding_type = fbo_attachment_point<Allocator>;
	using color_attachment_binding_type = fbo_layout_bindable_color_attachment_point<Allocator>;
	using color_attachments_binding_map_type = std::unordered_map<unsigned, std::unique_ptr<color_attachment_binding_type>>;

private:
	depth_attachment_binding_type depth_attachment_binding_point{ this, GL_DEPTH_ATTACHMENT };
	color_attachments_binding_map_type color_attachments_binding_points;

	color_attachment_binding_type &color_binding_point(unsigned index, bool * created = nullptr) {
		auto emplace_result = color_attachments_binding_points.emplace(std::make_pair(index,std::make_unique<color_attachment_binding_type>(color_attachment_binding_type::emplace_helper(), this, index)));
		auto &it = emplace_result.first;
		if (created) *created = emplace_result.second;
		return *it->second;
	}

public:
	framebuffer_object() {}

	framebuffer_object(framebuffer_object &&t) = default;
	framebuffer_object &operator=(framebuffer_object &&t) = default;

	depth_attachment_binding_type &depth_binding_point() { return depth_attachment_binding_point; }
	const depth_attachment_binding_type &depth_binding_point() const { return depth_attachment_binding_point; }

	color_attachment_binding_type &operator[](unsigned index) {
		bool created = false;
		auto &ret = color_binding_point(index, &created);
		if (created)
			set_color_output_binding_index(index, color_layout_binding(index));
		return ret;
	}
};

}
}
