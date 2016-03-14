// StE
// � Shlomi Steinberg, 2015

#pragma once

#include <array>
#include <unordered_map>

#include <functional>

#include <type_traits>
#include "gl_type_traits.h"

#include "context_state.h"
#include "context_state_key.h"

#include "gl_current_context.h"

#include "tuple_call.h"

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
	
	mutable std::unordered_map<context_state_name, context_state> states;
	mutable std::unordered_map<context_state_key, context_state> resources;

private:
	template <typename FuncT, typename K, typename V, typename... Ts, typename... Args>
	void set_context_server_state(std::size_t &counter, std::unordered_map<K, V> &m, K &&k, std::tuple<Ts...> &&v, FuncT *func, Args... &&args) const {
		if (m[k].compare(v))
			return;
		
		m[k].set([=](const tuple_type_erause &t) {
					tuple_call(popper_func, t);
				}, std::move(v));
		++counter;
		
		if (!dummy)
			func(std::forward<Args>(args)...);
	}
	template <typename FuncT, typename K, typename V, typename... Args>
	void set_context_server_state(std::size_t &counter, std::unordered_map<K, V> &m, K &&k, FuncT *func, Args... args) const {
		set_context_server_state(counter, m, std::move(k, std::make_tuple<Args...>(args...), func, std::forward<Args>(args)...);
	}
	
	template <typename K, typename V>
	auto &get_context_server_state(std::unordered_map<K, V> &m, K &&k) const {
		return m[k];
	}
	
	template <typename K, typename V>
	void push_context_server_state(std::unordered_map<K, V> &m, K &&k) const {
		auto it = m.find(k);
		assert(it != m.end() && "State wasn't previously set or can't be pushed.");
		if (it == m.end())
			return;
			
		it->push();
	}
	
	template <typename K, typename V>
	void pop_context_server_state(std::unordered_map<K, V> &m, K &&k) const {
		auto it = m.find(k);
		assert(it != m.end() && "State wasn't previously set or can't be popped.");
		if (it == m.end())
			return;
			
		auto t = it->pop();
		assert(!!t && "State wasn't previously pushed.");
		if (!t)
			return;
		
		if (!dummy)
			t.second()(t.first);
	}

public:
	void viewport(std::int32_t x, std::int32_t y, std::uint32_t w, std::uint32_t h) const {
		set_context_server_state(total_state_changes, 
								 states,
								 VIEWPORT_STATE,
								 glViewport, 
								 x, y, w, h);
	}
	auto viewport() const {
		return get_context_server_state(states, VIEWPORT_STATE).get<std::int32_t, std::int32_t, std::uint32_t, std::uint32_t>();
	}

	void color_mask(bool r, bool b, bool g, bool a) const {
		set_context_server_state(total_state_changes, 
								 states,
								 COLOR_MASK_STATE,
								 glColorMask, 
								 r, g, b, a);
	}
	auto color_mask() const {
		return get_context_server_state(states, COLOR_MASK_STATE).get<bool, bool, bool, bool>();
	}
	
	void depth_mask(bool mask) const {
		set_context_server_state(total_state_changes, 
								 states,
								 DEPTH_MASK_STATE,
								 glDepthMask, 
								 mask);
	}
	auto depth_mask() const {
		return get_context_server_state(states, DEPTH_MASK_STATE).get<bool>();
	}

	void cull_face(GLenum face) const {
		set_context_server_state(total_state_changes, 
								 states,
								 CULL_FACE_STATE,
								 glCullFace, 
								 face);
	}
	auto cull_face() const {
		return get_context_server_state(states, CULL_FACE_STATE).get<GLenum>();
	}
	
	void front_face(GLenum face) const {
		set_context_server_state(total_state_changes, 
								 states,
								 FRONT_FACE_STATE,
								 glFrontFace, 
								 face);
	}
	auto front_face() const {
		return get_context_server_state(states, FRONT_FACE_STATE).get<GLenum>();
	}

	void blend_func(GLenum src, GLenum dst) const {
		set_context_server_state(total_state_changes, 
								 states,
								 BLEND_FUNC_STATE,
								 glBlendFunc, 
								 src, dst);
	}
	auto blend_func() const {
		return get_context_server_state(states, BLEND_FUNC_STATE).get<GLenum, GLenum>();
	}
	
	void blend_func_separate(GLenum src_rgb, GLenum dst_rgb, GLenum src_a, GLenum dst_a) const {
		set_context_server_state(total_state_changes, 
								 states, 
								 BLEND_FUNC_SEPARATE_STATE,
								 glBlendFuncSeparate, 
								 src_rgb, dst_rgb, src_a, dst_a);
	}
	auto blend_func_separate() const {
		return get_context_server_state(states, BLEND_FUNC_SEPARATE_STATE).get<GLenum, GLenum, GLenum, GLenum>();
	}
	
	void blend_color(float r, float g, float b, float a) const {
		set_context_server_state(total_state_changes, 
								 states, 
								 BLEND_COLOR_STATE,
								 glBlendColor, 
								 r, g, b, a);
	}
	auto blend_color() const {
		return get_context_server_state(states, BLEND_COLOR_STATE).get<float, float, float, float>();
	}
	
	void blend_equation(GLenum mode) const {
		set_context_server_state(total_state_changes, 
								 states, 
								 BLEND_EQUATION_STATE,
								 glBlendEquation, 
								 mode);
	}
	auto blend_equation() const {
		return get_context_server_state(states, BLEND_EQUATION_STATE).get<GLenum>();
	}

public:
	void bind_buffer(GLenum target, std::uint32_t id) const {
		set_context_server_state(total_buffer_changes, 
								 resources,
								 { BUFFER_OBJECT, target, 0 },
								 std::make_tuple<std::uin32_t,std::uin32_t,int,std::size_t>(0, id, 0, -1),
								 glBindBuffer,
								 target, id);
	}
	void bind_buffer_base(GLenum target, std::uint32_t index, std::uint32_t id) const {
		set_context_server_state(total_buffer_changes, 
								 resources,
								 { BUFFER_OBJECT, target, index },
								 std::make_tuple<std::uin32_t,std::uin32_t,int,std::size_t>(index, id, 0, -1),
								 glBindBufferBase,
								 target, index, id);
	}
	void bind_buffer_range(GLenum target, std::uint32_t index, std::uint32_t id, int offset, std::size_t size) const {
		set_context_server_state(total_buffer_changes, 
								 resources,
								 { BUFFER_OBJECT, target, index },
								 std::make_tuple<std::uin32_t,std::uin32_t,int,std::size_t>(index, id, offset, size),
								 glBindBufferRange,
								 target, index, id, offset, size);
	}
	void push_buffer_state(GLenum target, std::uint32_t index = 0) const {
		push_context_server_state(resources, { BUFFER_OBJECT, target, index });
	}
	void pop_buffer_state(GLenum target, std::uint32_t index = 0) const {
		pop_context_server_state(resources, { BUFFER_OBJECT, target, index });
	}
	
	void bind_texture_unit(std::uint32_t unit, std::uint32_t id) const {
		set_context_server_state(total_texture_changes, 
								 resources,
								 { TEXTURE_OBJECT, unit },
								 std::make_tuple(id),
								 glBindTextureUnit,
								 unit, id);
	}
	void push_texture_unit_state(std::uint32_t unit) const {
		push_context_server_state(resources, { TEXTURE_OBJECT, unit });
	}
	void pop_texture_unit_state(std::uint32_t unit) const {
		pop_context_server_state(resources, { TEXTURE_OBJECT, unit });
	}
	
	void bind_framebuffer(GLenum target, std::uint32_t id) const {
		set_context_server_state(total_fbo_changes, 
								 resources,
								 { FRAMEBUFFER_OBJECT, target },
								 std::make_tuple(id),
								 glBindFramebuffer,
								 target, id);
	}
	void push_framebuffer_state(GLenum target) const {
		push_context_server_state(resources, { FRAMEBUFFER_OBJECT, target });
	}
	void pop_framebuffer_state(GLenum target) const {
		pop_context_server_state(resources, { FRAMEBUFFER_OBJECT, target });
	}
	
	void bind_image_texture(std::uint32_t unit, std::uint32_t texture, int level, bool layered, int layer, GLenum access, GLenum format) const {
		set_context_server_state(total_state_changes, 
								 resources,
								 { IMAGE_OBJECT, unit },
								 std::make_tuple(texture, level, layered, layer, access, format),
								 glBindImageTexture,
								 unit, texture, level, layered, layer, access, format);
	}
	void push_image_texture_state(std::uint32_t unit) const {
		push_context_server_state(resources, { IMAGE_OBJECT, unit });
	}
	void pop_image_texture_state(std::uint32_t unit) const {
		pop_context_server_state(resources, { IMAGE_OBJECT, unit });
	}
	
	void bind_renderbuffer(GLenum target, std::uint32_t id) const {
		set_context_server_state(total_fbo_changes, 
								 resources,
								 { RENDERBUFFER_OBJECT, target },
								 std::make_tuple(id),
								 glBindRenderbuffer,
								 target, id);
	}
	void push_renderbuffer_state(GLenum target) const {
		push_context_server_state(resources, { RENDERBUFFER_OBJECT, target });
	}
	void pop_renderbuffer_state(GLenum target) const {
		pop_context_server_state(resources, { RENDERBUFFER_OBJECT, target });
	}
	
	void bind_sampler(std::uint32_t unit, std::uint32_t id) const {
		set_context_server_state(total_state_changes, 
								 resources,
								 { SAMPLER_OBJECT, unit },
								 std::make_tuple(id),
								 glBindSampler,
								 unit, id);
	}
	void push_sampler_state(std::uint32_t unit) const {
		push_context_server_state(resources, { SAMPLER_OBJECT, unit });
	}
	void pop_sampler_state(std::uint32_t unit) const {
		pop_context_server_state(resources, { SAMPLER_OBJECT, unit });
	}
	
	void bind_transform_feedback(GLenum target, std::uint32_t id) const {
		set_context_server_state(total_state_changes, 
								 resources,
								 { TRANSFORM_FEEDBACK_OBJECT, target },
								 std::make_tuple(id),
								 glBindTransformFeedback,
								 target, id);
	}
	void push_transform_feedback_state(GLenum target) const {
		push_context_server_state(resources, { TRANSFORM_FEEDBACK_OBJECT, target });
	}
	void pop_transform_feedback_state(GLenum target) const {
		pop_context_server_state(resources, { TRANSFORM_FEEDBACK_OBJECT, target });
	}
	
	void bind_vertex_array(std::uint32_t id) const {
		set_context_server_state(total_va_changes, 
								 resources,
								 { VERTEX_ARRAY_OBJECT },
								 std::make_tuple(id),
								 glBindVertexArray,
								 id);
	}
	void push_vertex_array_state() const {
		push_context_server_state(resources, { VERTEX_ARRAY_OBJECT });
	}
	void pop_vertex_array_state() const {
		pop_context_server_state(resources, { VERTEX_ARRAY_OBJECT });
	}
	
	void bind_shader_program(std::uint32_t id) const {
		set_context_server_state(total_shader_changes, 
								 resources,
								 { GLSL_PROGRAM_OBJECT },
								 std::make_tuple(id),
								 glUseProgram,
								 id);
	}
	void push_shader_program_state(GLenum target) const {
		push_context_server_state(resources, { GLSL_PROGRAM_OBJECT });
	}
	void pop_shader_program_state(GLenum target) const {
		pop_context_server_state(resources, { GLSL_PROGRAM_OBJECT });
	}

public:
	void enable_depth_test() const { enable_state(DEPTH_TEST); }
	void disable_depth_test() const { disable_state(DEPTH_TEST); }

	void enable_state(context_state_name state) const {
		set_context_server_state(total_state_changes, 
								 states,
								 state,
								 std::make_tuple(true),
								 glEnable);
	}
	void disable_state(context_state_name state) const {
		set_context_server_state(total_state_changes, 
								 states,
								 state,
								 std::make_tuple(false),
								 glDisable);
	}
	void set_state(context_state_name state, bool enable) const {
		enable ? enable_state(state) : disable_state(state);
	}
	bool is_enabled(context_state_name state) const {
		return get_context_server_state(states, state).get<bool>();
	}
	
public:
	void push_state(context_state_name state) const {
		push_context_server_state(states, state);
	}
	void pop_state(context_state_name state) const {
		pop_context_server_state(states, state);
	}
	
public:
	void draw_arrays(GLenum mode, std::int32_t first, std::uint32_t count) const {
		glDrawArrays(mode, first, count);
	}
	
	template <typename T>
	void draw_elements(GLenum mode, std::uint32_t count, const void* ind) const {
		draw_elements(mode, count, gl_type_name_enum<T>::gl_enum, ind);
	}
	void draw_elements(GLenum mode, std::uint32_t count, GLenum type, const void* ind) const {
		glDrawElements(mode, count, type, ind);
	}
	
	template <typename T>
	void draw_multi_elements_indirect(GLenum mode, const void* ind, std::uint32_t drawcount, std::uint32_t stride) const {
		draw_multi_elements_indirect(mode, gl_type_name_enum<T>::gl_enum, ind);
	}
	void draw_multi_elements_indirect(GLenum mode, GLenum type, const void* ind, std::uint32_t drawcount, std::uint32_t stride) const {
		glMultiDrawElementsIndirect((mode, type, ind, drawcount, stride);
	}
	
	void dispatch_compute(std::uint32_t x, std::uint32_t y, std::uint32_t z) const {
		glDispatchCompute(x, y, z);
	} 
	
public:
	virtual void make_current() {
		gl_current_context::current = this;
	}

	void memory_barrier(GLbitfield bits) const { glMemoryBarrier(bits); }

	void clear_framebuffer(bool color = true, bool depth = true) const { glClear((color ? GL_COLOR_BUFFER_BIT : 0) | (depth ? GL_DEPTH_BUFFER_BIT : 0)); }

public:
	gl_generic_context(bool dummy = false) : dummy(dummy) {}
	virtual ~gl_generic_context() noexcept {}
};

}
}
