// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "optional.h"
#include "function_traits.h"

#include "optional.h"

#include <memory>
#include <functional>

#include <array>
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

	window_type create_window(const char *title, const glm::i32vec2 &size, gli::format format, gli::format depth_format);
	void create_default_framebuffer(gli::format format, gli::format depth_format);
	void make_current();

private:
	template <typename dummy, typename FuncT, typename... Args>
	void set_context_server_state(FuncT *func, Args... args) const {
		using T = std::tuple<decltype(Args{})...>;
		static thread_local optional<T> state = none;
		T new_args = T(args...);

		if (!state || *state != new_args) {
			func(args...);
			state = std::move(new_args);
		}
	}

	template <typename T, typename K, typename V, typename M, typename... Args>
	void bind_context_resource(T *func, const K &index, const V &spec, M &m, Args... args) const {
		auto it = m.find(index);
		if (it == m.end() || it->second != spec) {
			func(args...);
			m[index] = spec;
		}
	}

	template <typename T, typename V, typename... Args>
	void bind_context_resource_val(T *func, const V &spec, optional<V> &o, Args... args) const {
		if (!o || *o != spec) {
			func(args...);
			o = spec;
		}
	}

	template <typename T, typename K, typename V, typename M, int N, typename... Args>
	void bind_context_resource_multiple(T *func, const std::array<std::pair<K, V>, N> &vals, M &m, Args... args) const {
		bool exec = false;
		for (auto &p : vals) {
			auto &index = p.first;
			auto &spec = p.second;

			auto it = m.find(index);
			if (it == m.end() || it->second != spec) {
				exec = true;
				m[index] = spec;
			}
		}

		if (exec)
			func(args...);
	}

	mutable std::map<std::tuple<GLenum, unsigned>, std::tuple<unsigned, int, std::size_t>> bind_buffers_map;
	mutable std::unordered_map<unsigned, unsigned> bind_textures_map, bind_framebuffer_map, bind_renderbuffer_map, bind_sampler_map, bind_transfrom_feedback_map;
	mutable std::unordered_map<unsigned, std::tuple<unsigned, int, int, bool, GLenum, GLenum>> bind_images_map;
	mutable std::unordered_map<unsigned, std::tuple<unsigned, int, int>> bind_vertex_buffer_map;
	mutable optional<unsigned> bind_program_val, bind_vertex_array_val;

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

	void color_mask(bool r, bool b, bool g, bool a) const {
		struct dummy {};
		set_context_server_state<dummy>(glColorMask, r, g, b, a);
	}
	void depth_mask(bool mask) const {
		struct dummy {};
		set_context_server_state<dummy>(glDepthMask, mask);
	}

	void cull_face(GLenum face) const {
		struct dummy {};
		set_context_server_state<dummy>(glCullFace, face);
	}
	void front_face(GLenum face) const {
		struct dummy {};
		set_context_server_state<dummy>(glFrontFace, face);
	}

	void blend_func(GLenum src, GLenum dst) const {
		struct dummy {};
		set_context_server_state<dummy>(glBlendFunc, src, dst);
	}
	void blend_func_separate(GLenum src_rgb, GLenum dst_rgb, GLenum src_a, GLenum dst_a) const {
		struct dummy {};
		set_context_server_state<dummy>(glBlendFuncSeparate, src_rgb, dst_rgb, src_a, dst_a);
	}
	void blend_color(float r, float g, float b, float a) const {
		struct dummy {};
		set_context_server_state<dummy>(glBlendColor, r, g, b, a);
	}
	void blend_equation(GLenum mode) const {
		struct dummy {};
		set_context_server_state<dummy>(glBlendEquation, mode);
	}

	void enable_vertex_attrib_array(unsigned index) const {
		struct dummy {};
		set_context_server_state<dummy>(glEnableVertexAttribArray, index);
	}

	void bind_buffer(GLenum target, unsigned id) const {
		bind_context_resource(glBindBuffer,
							  std::make_tuple(target, static_cast<unsigned>(0)), std::make_tuple(id, -1, static_cast<std::size_t>(-1)),
							  bind_buffers_map,
							  target, id);
	}
	void bind_buffer_base(GLenum target, unsigned index, unsigned id) const {
		bind_context_resource(glBindBufferBase,
							  std::make_tuple(target, index), std::make_tuple(id, -1, static_cast<std::size_t>(-1)),
							  bind_buffers_map,
							  target, index, id);
	}
	template <int first, int N>
	void bind_buffers_base(GLenum target, const std::array<unsigned, N> &ids) const {
		std::array<std::pair<std::tuple<GLenum, unsigned>, std::tuple<unsigned, int, std::size_t>>>, N> vals;
		for (int i = 0; i < N; ++i) vals[i] = std::make_pair(std::make_tuple(target, static_cast<unsigned>(i + first)),
															 std::make_tuple(units[i], -1, static_cast<std::size_t>(-1)));

		bind_context_resource_multiple(glBindBuffersBase,
									   vals,
									   bind_buffers_map,
									   target, first, N, &ids[0]);
	}
	void bind_buffer_range(GLenum target, unsigned index, unsigned id, int offset, std::size_t size) const {
		bind_context_resource(glBindBufferRange,
							  std::make_tuple(target, index), std::make_tuple(id, offset, size),
							  bind_buffers_map,
							  target, index, id, offset, size);
	}
	void bind_texture_unit(unsigned unit, unsigned id) const {
		bind_context_resource(glBindTextureUnit,
							  unit, id,
							  bind_textures_map,
							  unit, id);
	}
	template <int first, int N>
	void bind_texture_units(const std::array<unsigned, N> &units) const {
		std::array<std::pair<unsigned, unsigned>, N> vals;
		for (int i = 0; i < N; ++i) vals[i] = std::make_pair(static_cast<unsigned>(i + first), units[i]);

		bind_context_resource_multiple(glBindTextures, 
									   vals, 
									   bind_textures_map, 
									   first, N, &units[0]);
	}
	void bind_framebuffer(GLenum target, unsigned id) const {
		bind_context_resource(glBindFramebuffer,
							  target, id,
							  bind_framebuffer_map,
							  target, id);
	}
	void bind_image_texture(unsigned unit, unsigned texture, int level, bool layered, int layer, GLenum access, GLenum format) const {
		bind_context_resource(glBindImageTexture,
							  unit, std::make_tuple(texture, level, layer, layered, access, format),
							  bind_images_map,
							  unit, texture, level, layered, layer, access, format);
	}
	void bind_renderbuffer(GLenum target, unsigned id) const {
		bind_context_resource(glBindRenderbuffer,
							  target, id,
							  bind_renderbuffer_map,
							  target, id);
	}
	void bind_sampler(unsigned unit, unsigned id) const {
		bind_context_resource(glBindSampler,
							  unit, id,
							  bind_sampler_map,
							  unit, id);
	}
	void bind_transform_feedback(GLenum target, unsigned id) const {
		bind_context_resource(glBindTransformFeedback,
							  target, id,
							  bind_transfrom_feedback_map,
							  target, id);
	}
	void bind_vertex_array(unsigned id) const {
		bind_context_resource_val(glBindVertexArray,
								  id,
								  bind_vertex_array_val,
								  id);
	}
	void bind_vertex_buffer(unsigned bindingindex, unsigned buffer, int offset, int stride) const {
		bind_context_resource(glBindVertexBuffer,
							  bindingindex, std::make_tuple(buffer, offset, stride),
							  bind_vertex_buffer_map,
							  bindingindex, buffer, offset, stride);
	}
	void bind_shader_program(unsigned id) const {
		bind_context_resource_val(glUseProgram,
								  id,
								  bind_program_val,
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
