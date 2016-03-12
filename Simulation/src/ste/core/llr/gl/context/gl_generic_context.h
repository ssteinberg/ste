// StE
// � Shlomi Steinberg, 2015

#pragma once

#include <array>
#include <unordered_map>

#include <type_traits>

#include "context_state.h"
#include "context_state_key.h"

#include "gl_current_context.h"

namespace StE {
namespace LLR {

class gl_generic_context {
private:
	bool dummy;
	
protected:
	std::size_t total_state_changes{ 0 };
	std::size_t total_buffer_changes{ 0 };
	std::size_t total_texture_changes{ 0 };
	std::size_t total_shader_changes{ 0 };
	std::size_t total_fbo_changes{ 0 };
	std::size_t total_va_changes{ 0 };
	
	mutable std::unordered_map<context_state_name, context_basic_state> basic_states;
	mutable std::unordered_map<void*, context_state> states;
	mutable std::unordered_map<context_state_key, context_state> resources;

private:
	template <typename FuncT, typename K, typename V, typename... Args>
	void set_context_server_state(std::size_t &counter, std::unordered_map<K, V> &m, K &&k, FuncT *func, Args... args) const {
		if (m[k].compare(args...))
			return;
		
		m[k].push([=](){ func(args...); }, args...);
		++counter;
		
		if (!dummy)
			func(args...);
	}
	template <typename FuncT, typename K, typename V, typename... Args>
	void set_context_server_state(std::size_t &counter, std::unordered_map<K, V> &m, FuncT *func, Args... args) const {
		set_context_server_state(counter, m, reinterpret_cast<K>(func), func, args...);
	}
	
	template <typename K, typename V>
	auto &get_context_server_state(std::unordered_map<K, V> &m, K &&k) const {
		return m[k];
	}
	template <typename FuncT, typename K, typename V>
	auto &get_context_server_state(std::unordered_map<K, V> &m, FuncT *func) const {
		return get_context_server_state(m, reinterpret_cast<K>(func));
	}
	
	void set_context_server_state(context_state_name &&name, bool enabled) const {
		if (basic_states[name] == enabled)
			return;
		
		basic_states[name].push(enabled);
		++total_state_changes;
		
		if (!dummy) {
			GLenum glname = static_cast<typename std::underlying_type<context_state_name>::type>(name);
			enabled ?
				glEnable(glname) :
				glDisable(glname);
		}
	}
	
	bool get_context_server_state(context_state_name &&name) const {
		return basic_states[name].compare(true);
	}

public:
	gl_generic_context(bool dummy = false) : dummy(dummy), basic_states({{ DITHER, { true, none }  }, { MULTISAMPLE, { true, none } }}) {}
	virtual ~gl_generic_context() noexcept {}

	void viewport(std::int32_t x, std::int32_t y, std::uint32_t w, std::uint32_t h) const {
		set_context_server_state(total_state_changes, 
								 states,
								 glViewport, 
								 x, y, w, h);
	}
	auto viewport() const {
		return get_context_server_state(states, glViewport)
					.get<std::int32_t, std::int32_t, std::uint32_t, std::uint32_t>();
	}

	void color_mask(bool r, bool b, bool g, bool a) const {
		set_context_server_state(total_state_changes, 
								 states,
								 glColorMask, 
								 r, g, b, a);
	}
	void depth_mask(bool mask) const {
		set_context_server_state(total_state_changes, 
								 states,
								 glDepthMask, 
								 mask);
	}

	void cull_face(GLenum face) const {
		set_context_server_state(total_state_changes, 
								 states,
								 glCullFace, 
								 face);
	}
	void front_face(GLenum face) const {
		set_context_server_state(total_state_changes, 
								 states,
								 glFrontFace, 
								 face);
	}

	void blend_func(GLenum src, GLenum dst) const {
		set_context_server_state(total_state_changes, 
								 states,
								 glBlendFunc, 
								 src, dst);
	}
	void blend_func_separate(GLenum src_rgb, GLenum dst_rgb, GLenum src_a, GLenum dst_a) const {
		set_context_server_state(total_state_changes, 
								 states, 
								 glBlendFuncSeparate, 
								 src_rgb, dst_rgb, src_a, dst_a);
	}
	void blend_color(float r, float g, float b, float a) const {
		set_context_server_state(total_state_changes, 
								 states, 
								 glBlendColor, 
								 r, g, b, a);
	}
	void blend_equation(GLenum mode) const {
		set_context_server_state(total_state_changes, 
								 states, 
								 glBlendEquation, 
								 mode);
	}

	void bind_buffer(GLenum target, std::uint32_t id) const {
		set_context_server_state(total_buffer_changes, 
								 resources,
								 { BUFFER_OBJECT, target },
								 glBindBuffer,
								 target, id);
	}
	void bind_buffer_base(GLenum target, std::uint32_t index, std::uint32_t id) const {
		set_context_server_state(total_buffer_changes, 
								 resources,
								 { BUFFER_OBJECT, target, index },
								 glBindBufferBase,
								 target, index, id);
	}
	void bind_buffer_range(GLenum target, std::uint32_t index, std::uint32_t id, int offset, std::size_t size) const {
		set_context_server_state(total_buffer_changes, 
								 resources,
								 { BUFFER_OBJECT, target, index },
								 glBindBufferRange,
								 target, index, id, offset, size);
	}
	void bind_texture_unit(std::uint32_t unit, std::uint32_t id) const {
		set_context_server_state(total_texture_changes, 
								 resources,
								 { TEXTURE_OBJECT, unit },
								 glBindTextureUnit,
								 unit, id);
	}
	void bind_framebuffer(GLenum target, std::uint32_t id) const {
		set_context_server_state(total_fbo_changes, 
								 resources,
								 { FRAMEBUFFER_OBJECT, target },
								 glBindFramebuffer,
								 target, id);
	}
	void bind_image_texture(std::uint32_t unit, std::uint32_t texture, int level, bool layered, int layer, GLenum access, GLenum format) const {
		set_context_server_state(total_state_changes, 
								 resources,
								 { IMAGE_OBJECT, unit },
								 glBindImageTexture,
								 unit, texture, level, layered, layer, access, format);
	}
	void bind_renderbuffer(GLenum target, std::uint32_t id) const {
		set_context_server_state(total_fbo_changes, 
								 resources,
								 { RENDERBUFFER_OBJECT, target },
								 glBindRenderbuffer,
								 target, id);
	}
	void bind_sampler(std::uint32_t unit, std::uint32_t id) const {
		set_context_server_state(total_state_changes, 
								 resources,
								 { SAMPLER_OBJECT, unit },
								 glBindSampler,
								 unit, id);
	}
	void bind_transform_feedback(GLenum target, std::uint32_t id) const {
		set_context_server_state(total_state_changes, 
								 resources,
								 { TRANSFORM_FEEDBACK_OBJECT, target },
								 glBindTransformFeedback,
								 target, id);
	}
	void bind_vertex_array(std::uint32_t id) const {
		set_context_server_state(total_va_changes, 
								 resources,
								 { VERTEX_ARRAY_OBJECT },
								 glBindVertexArray,
								 id);
	}
	void bind_shader_program(std::uint32_t id) const {
		set_context_server_state(total_shader_changes, 
								 resources,
								 { GLSL_PROGRAM_OBJECT },
								 glUseProgram,
								 id);
	}

	void enable_depth_test() const { enable_state(GL_DEPTH_TEST); }
	void disable_depth_test() const { disable_state(GL_DEPTH_TEST); }

	void enable_state(GLenum state) const {
		set_context_server_state(static_cast<context_state_name>(state), true);
	}
	void disable_state(GLenum state) const {
		set_context_server_state(static_cast<context_state_name>(state), false);
	}
	void set_state(GLenum state, bool enable) const {
		enable ? enable_state(state) : disable_state(state);
	}
	bool is_enabled(GLenum state) const {
		return get_context_server_state(static_cast<context_state_name>(state));
	} 
	
	virtual void make_current() {
		gl_current_context::current = this;
	}

	void memory_barrier(GLbitfield bits) const { glMemoryBarrier(bits); }

	void clear_framebuffer(bool color = true, bool depth = true) const { glClear((color ? GL_COLOR_BUFFER_BIT : 0) | (depth ? GL_DEPTH_BUFFER_BIT : 0)); }
};

}
}
