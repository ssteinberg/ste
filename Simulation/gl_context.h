// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "optional.h"
#include "function_traits.h"

#include <memory>
#include <functional>
#include <map>
#include <unordered_map>

#include <gli/gli.hpp>
#include <glfw/glfw3.h>

namespace StE {

class StEngineControl;

namespace LLR {

class system_provided_framebuffer;

class gl_context {
private:
	friend class StE::StEngineControl;

	using window_type = std::unique_ptr<GLFWwindow, std::function<void(GLFWwindow*)>>;

public:
	struct context_settings {
		optional<bool> fs;
		optional<bool> vsync;
		optional<int> samples;
		optional<bool> debug_context;
	};

protected:
	void resize(const glm::i32vec2 &size);
	void setup_debug_context();

	window_type window;
	std::unique_ptr<system_provided_framebuffer> default_fb;
	context_settings ctx_settings;

	mutable std::unordered_map<GLenum, bool> states;
	mutable std::unordered_map<GLenum, bool> default_states;
	mutable std::map<std::tuple<GLenum, unsigned>, std::tuple<unsigned, int, std::size_t>> bind_buffer_map;

	window_type create_window(const char *title, const glm::i32vec2 &size, gli::format format, gli::format depth_format);
	void create_default_framebuffer(gli::format format, gli::format depth_format);
	void make_current();

private:
	template <typename T, typename... Args>
	void set_context_server_state(T *func, Args... args) const {
		using T = std::tuple<decltype(Args{})...>;

		static thread_local bool first_call{ true };
		static thread_local T val;

		T new_args = T(args...);

		if (first_call || val != new_args) {
			func(args...);
			val = new_args;
			first_call = false;
		}
	}

	template <typename T, typename K, typename V, typename... Args>
	void bind_context_resource(T *func, K &&index, V &&spec, Args... args) const {
		static thread_local std::map<std::remove_cv_t<std::remove_reference_t<K>>, std::remove_cv_t<std::remove_reference_t<V>>> val;

		auto it = val.find(index);
		if (it == val.end() || it->second != spec) {
			func(args...);
			val[std::move(index)] = std::move(spec);
		}
	}

	template <typename T, typename K, typename V, typename... Args>
	void bind_context_resource_with_map(T *func, K &&index, V &&spec, std::map<K,V> &m, Args... args) const {
		auto it = m.find(index);
		if (it == m.end() || it->second != spec) {
			func(args...);
			m[std::move(index)] = std::move(spec);
		}
	}

public:
	gl_context(const context_settings &settings, const char *title, const glm::i32vec2 &size, gli::format format = gli::FORMAT_RGBA8_SRGB, gli::format depth_format = gli::FORMAT_D24_UNORM);
	gl_context(const char * title, const glm::i32vec2 &size, gli::format format = gli::FORMAT_RGBA8_SRGB, gli::format depth_format = gli::FORMAT_D24_UNORM) : gl_context(context_settings(), title, size, format, depth_format) {}
	~gl_context() {}

	gl_context(gl_context &&m) = delete;
	gl_context(const gl_context &c) = delete;
	gl_context& operator=(gl_context &&m) = delete;
	gl_context& operator=(const gl_context &c) = delete;

	int meminfo_total_dedicated_vram() const { int ret; glGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &ret); return ret; }
	int meminfo_total_available_vram() const { int ret; glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &ret); return ret; }
	int meminfo_free_vram() const { int ret; glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &ret); return ret; }

	void memory_barrier(GLbitfield bits) const { glMemoryBarrier(bits); }

	void clear_framebuffer(bool color = true, bool depth = true) const { glClear((color ? GL_COLOR_BUFFER_BIT : 0) | (depth ? GL_DEPTH_BUFFER_BIT : 0)); }

	void color_mask(bool r, bool b, bool g, bool a) const { set_context_server_state(glColorMask, r, g, b, a); }
	void depth_mask(bool mask) const { set_context_server_state(glDepthMask, mask); }

	void cull_face(GLenum face) const { set_context_server_state(glCullFace, face); }
	void front_face(GLenum face) const { set_context_server_state(glFrontFace, face); }

	void blend_func(GLenum src, GLenum dst) const { set_context_server_state(glBlendFunc, src, dst); }
	void blend_func_separate(GLenum src_rgb, GLenum dst_rgb, GLenum src_a, GLenum dst_a) const { set_context_server_state(glBlendFuncSeparate, src_rgb, dst_rgb, src_a, dst_a); }
	void blend_color(float r, float g, float b, float a) const { set_context_server_state(glBlendColor, r, g, b, a); }
	void blend_equation(GLenum mode) const { set_context_server_state(glBlendEquation, mode); }

	void bind_buffer(GLenum target, unsigned id) const {
		bind_context_resource_with_map(glBindBuffer,
									   std::make_tuple(target, static_cast<unsigned>(0)), std::make_tuple(id, -1, static_cast<std::size_t>(-1)),
									   bind_buffer_map,
									   target, id);
	}
	void bind_buffer_base(GLenum target, unsigned index, unsigned id) const {
		bind_context_resource_with_map(glBindBufferBase,
									   std::make_tuple(target, index), std::make_tuple(id, -1, static_cast<std::size_t>(-1)),
									   bind_buffer_map,
									   target, index, id);
	}
	void bind_buffer_range(GLenum target, unsigned index, unsigned id, int offset, std::size_t size) const {
		bind_context_resource_with_map(glBindBufferRange,
									   std::make_tuple(target, index), std::make_tuple(id, offset, size),
									   bind_buffer_map,
									   target, index, id, offset, size);
	}
	void bind_texture_unit(unsigned unit, unsigned id) const {
		bind_context_resource(glBindTextureUnit, 
							  unit, id,
							  unit, id);
	}
	void bind_framebuffer(GLenum target, unsigned id) const {
		bind_context_resource(glBindFramebuffer, 
							  target, id,
							  target, id);
	}
	void bind_image_texture(unsigned unit, unsigned texture, int level, bool layered, int layer, GLenum access, GLenum format) const {
		bind_context_resource(glBindImageTexture, 
							  unit, std::make_tuple(texture, level, layer, layered, access, format),
							  unit, texture, level, layered, layer, access, format);
	}
	void bind_renderbuffer(GLenum target, unsigned id) const {
		bind_context_resource(glBindRenderbuffer, 
							  target, id,
							  target, id);
	}
	void bind_sampler(unsigned unit, unsigned id) const {
		bind_context_resource(glBindSampler, 
							  unit, id,
							  unit, id);
	}
	void bind_transform_feedback(GLenum target, unsigned id) const {
		bind_context_resource(glBindTransformFeedback,
							  target, id,
							  target, id);
	}
	void bind_vertex_array(unsigned id) const {
		bind_context_resource(glBindVertexArray, 
							  0, id,
							  id);
	}
	void bind_vertex_buffer(unsigned bindingindex, unsigned buffer, int offset, int stride) const {
		bind_context_resource(glBindVertexBuffer, 
							  bindingindex, std::make_tuple(buffer, offset, stride),
							  bindingindex, buffer, offset, stride);
	}
	void bind_shader_program(unsigned id) const {
		bind_context_resource(glUseProgram, 
							  0, id,
							  id);
	}

	void enable_state(GLenum state) const {
		auto emplace_result = states.try_emplace(state, false);
		if (emplace_result.second)
			default_states[state] = glIsEnabled(state);
		if (!emplace_result.first->second) {
			glEnable(state);
			emplace_result.first->second = true;
		}
	}
	void disable_state(GLenum state) const {
		auto emplace_result = states.try_emplace(state, false);
		if (emplace_result.second)
			default_states[state] = glIsEnabled(state);
		if (emplace_result.first->second) {
			glDisable(state);
			emplace_result.first->second = false;
		}
	}
	void set_state(GLenum state, bool enable) const {
		enable ? enable_state(state) : disable_state(state);
	}
	void restore_default_state(GLenum state) const {
		auto it = default_states.find(state);
		if (it != default_states.end()) {
			if (it->second)
				enable_state(state);
			else
				disable_state(state);
		}
	}

	void enable_depth_test() const { enable_state(GL_DEPTH_TEST); }
	void disable_depth_test() const { disable_state(GL_DEPTH_TEST); }

	void enable_vertex_attrib_array(unsigned index) const { set_context_server_state(glEnableVertexAttribArray, index); }

	gli::format framebuffer_format() const;
	glm::tvec2<std::size_t> framebuffer_size() const;
	system_provided_framebuffer &defaut_framebuffer() const;

	bool is_debug_context() const;
	const context_settings &get_contex_settings() const { return ctx_settings; }
};

}
}

#include "system_provided_framebuffer.h"

namespace StE {
namespace LLR {

gli::format inline gl_context::framebuffer_format() const { return default_fb->front_buffer().get_attachment_format(); }
glm::tvec2<std::size_t> inline gl_context::framebuffer_size() const { return default_fb->front_buffer().get_attachment_size(); }
system_provided_framebuffer inline &gl_context::defaut_framebuffer() const { return *default_fb; }

} 
}
