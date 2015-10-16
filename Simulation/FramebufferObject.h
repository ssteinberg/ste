// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "gl_utils.h"

#include "resource.h"
#include "resource_traits.h"
#include "bindable_resource.h"
#include "layout_binding.h"

#include "image.h"
#include "RenderTarget.h"
#include "texture_traits.h"

#include <memory>
#include <tuple>
#include <vector>
#include <unordered_map>

namespace StE {
namespace LLR {

template<typename A>
class frame_buffer_object;

template<typename A>
class fbo_attachment_point {
private:
	friend class FramebufferObject;

	glm::tvec2<std::size_t> size;
	gli::format format;

protected:
	frame_buffer_object<A> *fbo;
	GLenum attachment_point;

	fbo_attachment_point(fbo_attachment_point &&t) = delete;
	fbo_attachment_point &operator=(fbo_attachment_point &&t) = delete;
	fbo_attachment_point(const fbo_attachment_point &t) = delete;
	fbo_attachment_point &operator=(const fbo_attachment_point &t) = delete;

	fbo_attachment_point(frame_buffer_object<A> *fbo, GLenum attachment_point) : fbo(fbo), attachment_point(attachment_point) {}

public:
	virtual ~fbo_attachment_point() noexcept {}

	void detach() { 
		glNamedFramebufferTexture(fbo->get_resource_id(), attachment_point, 0, 0);
		size = { 0,0 };
	}

	template <llr_resource_type TexType>
	void attach(const image<TexType> &img, std::enable_if_t<texture_is_array<TexType>::value>* = 0) {
		glNamedFramebufferTextureLayer(fbo->get_resource_id(), attachment_point, img.get_resource_id(), img.get_level(), img.get_layer());
		size = img.get_image_size();
		format = img.get_format();
	}
	template <llr_resource_type TexType>
	void attach(const image<TexType> &img, std::enable_if_t<!texture_is_array<TexType>::value>* = 0) {
		glNamedFramebufferTexture(fbo->get_resource_id(), attachment_point, img.get_resource_id(), img.get_level());
		size = img.get_image_size();
		format = img.get_format();
	}
 	void attach(const RenderTarget &rt) {
		glNamedFramebufferRenderbuffer(fbo->get_resource_id(), attachment_point, GL_RENDERBUFFER, rt.get_resource_id());
		size = rt.get_image_size();
		format = rt.get_format();
 	}

	template<typename T>
	void operator=(const T &target) { attach(target); }

	unsigned int fbo_id() const { return fbo->get_resource_id(); }
	GLenum get_attachment_point() const { return attachment_point; }

	virtual glm::tvec2<std::size_t> get_attachment_size() const { return size; }
	virtual gli::format get_attachment_format() const { return format; }

	bool is_attached() const { return size.x > 0; }
};

template<typename A, GLenum color_attachment_point = GL_COLOR_ATTACHMENT0>
class fbo_color_attachment_point : public fbo_attachment_point<A> {
private:
	friend class FramebufferObject;

protected:
	using Base = fbo_attachment_point<A>;
	using Base::fbo_attachment_point;
	fbo_color_attachment_point(frame_buffer_object<A> *fbo, int index) : Base(fbo, color_attachment_point + index) {}

public:
	virtual ~fbo_color_attachment_point() noexcept {}

	void read_pixels(void *data, int data_size, const glm::uvec2 &rect_size, const glm::uvec2 &origin = { 0, 0 }) const {
		auto &gl_format = gl_utils::translate_format(get_attachment_format());

		fbo->bind_read();
		glReadBuffer(get_attachment_point());

		glReadnPixels(origin.x, origin.y, rect_size.x, rect_size.y, gl_format.External, gl_format.Type, data_size, data);

		fbo->unbind_read();
		fbo->update_draw_buffers();
	}
	void read_pixels(void *data, int data_size) const { read_pixels(data, data_size, get_attachment_size()); }

	void write_pixels(void *data, const glm::uvec2 &rect_size, const glm::uvec2 &origin = { 0, 0 }) {
		auto &gl_format = gl_utils::translate_format(get_attachment_format());

		fbo->bind_write();
		glWriteBuffer(get_attachment_point());

		if (origin.x || origin.y) glRasterPos2f(origin.x, origin.y);
		glDrawPixels(rect_size.x, rect_size.y, gl_format.External, gl_format.Type, data);
		if (origin.x || origin.y) glRasterPos2f(.0f, .0f);

		fbo->unbind_write();
		fbo->update_draw_buffers();
	}
	void write_pixels(void *data) { write_pixels(data, get_attachment_size()); }

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
	friend class FramebufferObject;
	struct emplace_helper {};

	int index;

	void unbind(const LayoutLocationType &) const final override {};

protected:
	using fbo_color_attachment_point<A>::fbo_color_attachment_point;
	fbo_layout_bindable_color_attachment_point(frame_buffer_object<A> *fbo, int index) : index(index), fbo_color_attachment_point(fbo, index) {}

public:
	virtual ~fbo_layout_bindable_color_attachment_point() noexcept {}

	template <typename ... Ts>
	fbo_layout_bindable_color_attachment_point(const emplace_helper &, Ts ... args) : fbo_layout_bindable_color_attachment_point(std::forward<Ts>(args)...) {}

	void bind(const LayoutLocationType &binding) const final override {
		fbo->set_color_output_binding_index(index, binding);
	}

	template<typename T>
	void operator=(const T &target) {
		*(static_cast<fbo_attachment_point<A>*>(this)) = target;
	}
};

class FramebufferObjectAllocator : public generic_resource_allocator {
public:
	unsigned allocate() override final { GLuint id;  glCreateFramebuffers(1, &id); return id; }
	static void deallocate(unsigned &id) { glDeleteFramebuffers(1, &id); id = 0; }
};

class FramebufferObjectBinder {
public:
	static void bind(unsigned int id) { glBindFramebuffer(GL_FRAMEBUFFER, id); }
	static void unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
};

template <class Allocator>
class frame_buffer_object : public bindable_resource<Allocator, FramebufferObjectBinder> {
private:
	friend class fbo_attachment_point<Allocator>;
	template<class A, GLenum t>
	friend class fbo_color_attachment_point;
	friend class fbo_layout_bindable_color_attachment_point<Allocator>;

private:
	std::vector<GLenum> draw_buffers;

protected:
	void set_color_output_binding_index(int attachment_index, const color_layout_binding &binding) {
		int color_output = binding == layout_binding_none<fbo_color_attachment_layout_binding_type>() ? 
			GL_NONE : 
			GL_COLOR_ATTACHMENT0 + binding.binding_index();

		if (draw_buffers.size() < attachment_index + 1) draw_buffers.resize(attachment_index + 1, GL_NONE);
		bool changed = draw_buffers[attachment_index] != color_output;
		draw_buffers[attachment_index] = color_output;

		if (changed) {
			for (int i = 0; i < draw_buffers.size(); ++i) {
				if (i == attachment_index) continue;
				if (draw_buffers[i] == color_output) draw_buffers[i] = GL_NONE;
			}
			update_draw_buffers();
		}
	}

	void update_draw_buffers() { 
		if (draw_buffers.size()) 
			glNamedFramebufferDrawBuffers(get_resource_id(), draw_buffers.size(), &draw_buffers[0]);
	}

	void bind_draw() const { glBindFramebuffer(GL_DRAW_FRAMEBUFFER, get_resource_id()); }
	void unbind_draw() const { glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); }
	void bind_read() const { glBindFramebuffer(GL_READ_FRAMEBUFFER, get_resource_id()); }
	void unbind_read() const { glBindFramebuffer(GL_READ_FRAMEBUFFER, 0); }

public:
	virtual ~frame_buffer_object() noexcept {}

	frame_buffer_object() {}
	template <typename ... Ts>
	frame_buffer_object(Ts ... draw_buffer_args) : draw_buffers(std::forward<Ts>(draw_buffer_args)...) {}

	frame_buffer_object(frame_buffer_object &&t) = default;
	frame_buffer_object &operator=(frame_buffer_object &&t) = default;

	bool is_fbo_complete() const { bind(); return is_valid() && get_status_code() == GL_FRAMEBUFFER_COMPLETE; }
	GLenum get_status_code() const { return glCheckNamedFramebufferStatus(get_resource_id(), GL_FRAMEBUFFER); }

	template <class A>
	void blit_to(frame_buffer_object<A> &fbo, const glm::ivec2 &src_size, const glm::ivec2 dst_size, const glm::ivec2 &src_origin = { 0,0 }, const glm::ivec2 dst_origin = { 0,0 }, bool linear_filter = true, bool blit_color = true, bool blit_depth = false) const {
		auto src_xy1 = src_origin + src_size;
		auto dst_xy1 = dst_origin + dst_size;
		int mask = 0;
		if (blit_depth) mask |= GL_DEPTH_BUFFER_BIT;
		if (blit_color) mask |= GL_COLOR_BUFFER_BIT;
		glBlitNamedFramebuffer(get_resource_id(), fbo.get_resource_id(), src_origin.x, src_origin.y, src_xy1.x, src_xy1.y, dst_origin.x, dst_origin.y, dst_xy1.x, dst_xy1.y, mask, linear_filter ? GL_LINEAR : GL_NEAREST);
	}

	static unsigned int max_framebuffer_bindings() {
		GLuint maxbuffers;
		glGetIntergeri(GL_MAX_DRAW_BUFFERS, &maxbuffers);
		return maxbuffers;
	}

	llr_resource_type resource_type() const override { return llr_resource_type::LLRFramebufferObject; }
};

class FramebufferObject : public frame_buffer_object<FramebufferObjectAllocator> {
private:
	using Allocator = FramebufferObjectAllocator;

	template<class A>
	friend class fbo_layout_bindable_color_attachment_point;

	using Base = frame_buffer_object<FramebufferObjectAllocator>;
	using depth_attachment_binding_type = fbo_attachment_point<Allocator>;
	using color_attachment_binding_type = fbo_layout_bindable_color_attachment_point<Allocator>;
	using color_attachments_binding_map_type = std::unordered_map<int, color_attachment_binding_type>;

private:
	depth_attachment_binding_type depth_attachment_binding_point{ this, GL_DEPTH_ATTACHMENT };
	color_attachments_binding_map_type color_attachments_binding_points;

	color_attachment_binding_type &color_binding_point(int index, bool * created = nullptr) {
		auto emplace_result = color_attachments_binding_points.emplace(std::piecewise_construct,
																	   std::forward_as_tuple(index),
																	   std::forward_as_tuple(color_attachment_binding_type::emplace_helper(), this, index));
		auto &it = emplace_result.first;
		if (created) *created = emplace_result.second;
		return it->second;
	}

public:
	FramebufferObject() {}

	FramebufferObject(FramebufferObject &&t) = default;
	FramebufferObject &operator=(FramebufferObject &&t) = default;

	depth_attachment_binding_type &depth_binding_point() { return depth_attachment_binding_point; }
	const depth_attachment_binding_type &depth_binding_point() const { return depth_attachment_binding_point; }

	color_attachment_binding_type &operator[](int index) {
		bool created = false;
		auto &ret = color_binding_point(index, &created);
		if (created)
			set_color_output_binding_index(index, color_layout_binding(index));
		return ret;
	}
};

}
}
