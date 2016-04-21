// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <string>
#include <functional>

#include "gl_type_traits.hpp"

#include "context_state.hpp"
#include "context_basic_state.hpp"
#include "context_state_key.hpp"
#include "context_state_name.hpp"

#include "gl_context_state_log.hpp"

#include "gl_current_context.hpp"

#include "tuple_type_erasure.hpp"
#include "tuple_call.hpp"

#include "optional.hpp"

#include <map>
#include <type_traits>

#include <boost/container/flat_map.hpp>

namespace StE {
namespace Core {
namespace GL {

class gl_generic_context {
private:
	template <typename K, typename V>
	using container = boost::container::flat_map<K, V>;

protected:
	mutable container<BasicStateName, context_basic_state> basic_states;
	mutable container<context_state_key<StateName>, context_state> states;
	mutable container<context_state_key<StateName>, context_state> resources;

	mutable gl_context_state_log *logger{ nullptr };

private:
	template <typename K, typename V>
	void set_context_server_state(container<K, V> &m, const context_state_key<StateName> &k, const context_state::state_type &state) const {
		auto it = m.find(k);
		if (it == m.end() || it->second.compare(state))
			return;

		if (it == m.end())
			it = m.emplace(std::make_pair(k, context_state())).first;

		it->second.set(state);

		apply_context_server_state(k, state);
	}
	template <typename FuncT, typename K, typename V, typename... Ts, typename... Args>
	void set_context_server_state(container<K, V> &m, const K &k, std::tuple<Ts...> &&v, FuncT *func, Args&&... args) const {
		auto it = m.find(k);
		auto args_tuple = std::tuple<std::remove_reference_t<Args>...>(args...);
		bool execute = it == m.end() || !it->second.compare(v);

		if (logger) {
			if (&m == &states) {
				optional<context_state::state_type> old_state = none;
				if (execute && it->second.exists())
					old_state = it->second.get_state();
				logger->log_state_change(execute, std::make_pair(k, old_state));
			}
			else if (&m == &resources) 	logger->log_resource_change(execute, k);
			else 						assert(false);
		}

		if (!execute)
			return;

		if (it == m.end())
			it = m.emplace(std::make_pair(k, context_state())).first;

		it->second.set([this, &m, k, func](const tuple_type_erasure &t) {
						   std::tuple<std::remove_reference_t<Args>...> params = t.get_weak<std::remove_reference_t<Args>...>();
						   apply_context_server_state(func, params);
					   },
					   std::move(v),
					   args_tuple);

		apply_context_server_state(func, args_tuple);
	}
	template <typename FuncT, typename K, typename V, typename... Args>
	void set_context_server_state(container<K, V> &m, const K &k, FuncT *func, Args... args) const {
		set_context_server_state(m, k, std::tuple<Args...>(args...), func, std::forward<Args>(args)...);
	}
	template <typename K>
	void set_context_server_state(container<K, context_basic_state> &m, const K &k, const context_basic_state::state_type &state) const {
		auto it = m.find(k);
		bool execute = it == m.end() || !it->second.compare(state);

		if (logger)
			logger->log_basic_state_change(execute, k);

		if (!execute)
			return;

		if (it == m.end())
			it = m.emplace(std::make_pair(k, context_basic_state())).first;

		it->second.set(state);

		apply_context_server_state(k, state);
	}

	template <typename K, typename V>
	auto &get_context_server_state(container<K, V> &m, const K &k) const {
		return m[k];
	}

	template <typename K, typename V>
	void push_context_server_state(container<K, V> &m, const K &k) const {
		auto it = m.find(k);
		assert(it != m.end() && "State wasn't previously set or can't be pushed.");
		if (it == m.end())
			return;

		it->second.push();
	}

	template <typename K, typename V>
	void pop_context_server_state(container<K, V> &m, const K &k) const {
		auto it = m.find(k);
		assert(it != m.end() && "State wasn't previously set");
		if (it == m.end())
			return;

		auto opt = it->second.pop();
		assert(opt && "State wasn't previously pushed");
		if (!opt)
			return;
		auto &state = opt.get();

		apply_context_server_state(k, state);
	}

private:
	template <typename K>
	static void apply_context_server_state(const K &k, const context_state::state_type &state) {
		static_cast<std::function<void(const tuple_type_erasure&)>>(state.setter)(state.args);
	}
	template <typename FuncT, typename... Args>
	static void apply_context_server_state(FuncT *func, const std::tuple<Args...> &params) {
		tuple_call(func, params);
	}

	template <typename K>
	static void apply_context_server_state(const K &k, const context_basic_state::state_type &state) {
		state ?
			glEnable(static_cast<GLenum>(k)) :
			glDisable(static_cast<GLenum>(k));
	}

public:
	void attach_logger(gl_context_state_log *l) const { logger = l; }
	void detach_logger() const { logger = nullptr; }

	void restore_states_from_logger(const gl_context_state_log &l) const {
		for (auto &state : l.get_basic_states())
			restore_default(state);
		for (auto &state : l.get_states()) {
			if (state.first.get_name() == StateName::SAMPLER_OBJECT)
				bind_sampler(state.first.get_index0(), 0);
			else if (state.first.get_name() == StateName::TRANSFORM_FEEDBACK_OBJECT)
				bind_transform_feedback(state.first.get_index0(), 0);
			else if (state.second) {
				const context_state::state_type &s = state.second.get();
				set_context_server_state(states, state.first, s);
			}
		}
	}

public:
	void viewport(std::int32_t x, std::int32_t y, std::uint32_t w, std::uint32_t h) const {
		set_context_server_state(states,
								 { StateName::VIEWPORT_STATE },
								 glViewport,
								 x, y, w, h);
	}
	auto viewport() const {
		return get_context_server_state(states, { StateName::VIEWPORT_STATE }).get_value<std::int32_t, std::int32_t, std::uint32_t, std::uint32_t>();
	}

	void color_mask(bool r, bool b, bool g, bool a) const {
		set_context_server_state(states,
								 { StateName::COLOR_MASK_STATE },
								 glColorMask,
								 r, g, b, a);
	}
	auto color_mask() const {
		return get_context_server_state(states, { StateName::COLOR_MASK_STATE }).get_value<bool, bool, bool, bool>();
	}

	void depth_mask(bool mask) const {
		set_context_server_state(states,
								 { StateName::DEPTH_MASK_STATE },
								 glDepthMask,
								 mask);
	}
	auto depth_mask() const {
		return get_context_server_state(states, { StateName::DEPTH_MASK_STATE }).get_value<bool>();
	}

	void cull_face(GLenum face) const {
		set_context_server_state(states,
								 { StateName::CULL_FACE_STATE },
								 glCullFace,
								 face);
	}
	auto cull_face() const {
		return get_context_server_state(states, { StateName::CULL_FACE_STATE }).get_value<GLenum>();
	}

	void front_face(GLenum face) const {
		set_context_server_state(states,
								 { StateName::FRONT_FACE_STATE },
								 glFrontFace,
								 face);
	}
	auto front_face() const {
		return get_context_server_state(states, { StateName::FRONT_FACE_STATE }).get_value<GLenum>();
	}

	void blend_func(GLenum src, GLenum dst) const {
		set_context_server_state(states,
								 { StateName::BLEND_FUNC_STATE },
								 glBlendFunc,
								 src, dst);
	}
	auto blend_func() const {
		return get_context_server_state(states, { StateName::BLEND_FUNC_STATE }).get_value<GLenum, GLenum>();
	}

	void blend_func_separate(GLenum src_rgb, GLenum dst_rgb, GLenum src_a, GLenum dst_a) const {
		set_context_server_state(states,
								 { StateName::BLEND_FUNC_SEPARATE_STATE },
								 glBlendFuncSeparate,
								 src_rgb, dst_rgb, src_a, dst_a);
	}
	auto blend_func_separate() const {
		return get_context_server_state(states, { StateName::BLEND_FUNC_SEPARATE_STATE }).get_value<GLenum, GLenum, GLenum, GLenum>();
	}

	void blend_color(float r, float g, float b, float a) const {
		set_context_server_state(states,
								 { StateName::BLEND_COLOR_STATE },
								 glBlendColor,
								 r, g, b, a);
	}
	auto blend_color() const {
		return get_context_server_state(states, { StateName::BLEND_COLOR_STATE }).get_value<float, float, float, float>();
	}

	void blend_equation(GLenum mode) const {
		set_context_server_state(states,
								 { StateName::BLEND_EQUATION_STATE },
								 glBlendEquation,
								 mode);
	}
	auto blend_equation() const {
		return get_context_server_state(states, { StateName::BLEND_EQUATION_STATE }).get_value<GLenum>();
	}

	void clear_color(float r, float g, float b, float a) const {
		set_context_server_state(states,
								 { StateName::CLEAR_COLOR_STATE },
								 glClearColor,
								 r, g, b, a);
	}
	auto clear_color() const {
		return get_context_server_state(states, { StateName::CLEAR_COLOR_STATE }).get_value<float, float, float, float>();
	}

	void clear_depth(float d) const {
		set_context_server_state(states,
								 { StateName::CLEAR_DEPTH_STATE },
								 glClearDepth,
								 d);
	}
	auto clear_depth() const {
		return get_context_server_state(states, { StateName::CLEAR_DEPTH_STATE }).get_value<float>();
	}

public:
	void bind_buffer(GLenum target, std::uint32_t id) const {
		set_context_server_state(resources,
								 { StateName::BUFFER_OBJECT, target, 0 },
								 std::tuple<std::uint32_t,std::uint32_t,int,std::size_t>(0, id, 0, std::numeric_limits<std::size_t>::max()),
								 glBindBuffer,
								 target, id);
	}
	void bind_buffer_base(GLenum target, std::uint32_t index, std::uint32_t id) const {
		set_context_server_state(resources,
								 { StateName::BUFFER_OBJECT, target, index },
								 std::tuple<std::uint32_t,std::uint32_t,int,std::size_t>(index, id, 0, std::numeric_limits<std::size_t>::max()),
								 glBindBufferBase,
								 target, index, id);
	}
	void bind_buffer_range(GLenum target, std::uint32_t index, std::uint32_t id, int offset, std::size_t size) const {
		set_context_server_state(resources,
								 { StateName::BUFFER_OBJECT, target, index },
								 std::tuple<std::uint32_t,std::uint32_t,int,std::size_t>(index, id, offset, size),
								 glBindBufferRange,
								 target, index, id, offset, size);
	}
	void push_buffer_state(GLenum target, std::uint32_t index = 0) const {
		push_context_server_state(resources, { StateName::BUFFER_OBJECT, target, index });
	}
	void pop_buffer_state(GLenum target, std::uint32_t index = 0) const {
		pop_context_server_state(resources, { StateName::BUFFER_OBJECT, target, index });
	}

	void bind_texture_unit(std::uint32_t unit, std::uint32_t id) const {
		set_context_server_state(resources,
								 { StateName::TEXTURE_OBJECT, unit },
								 std::make_tuple(id),
								 glBindTextureUnit,
								 unit, id);
	}
	void push_texture_unit_state(std::uint32_t unit) const {
		push_context_server_state(resources, { StateName::TEXTURE_OBJECT, unit });
	}
	void pop_texture_unit_state(std::uint32_t unit) const {
		pop_context_server_state(resources, { StateName::TEXTURE_OBJECT, unit });
	}

	void bind_framebuffer(GLenum target, std::uint32_t id) const {
		set_context_server_state(resources,
								 { StateName::FRAMEBUFFER_OBJECT, target },
								 std::make_tuple(id),
								 glBindFramebuffer,
								 target, id);
	}
	void push_framebuffer_state(GLenum target) const {
		push_context_server_state(resources, { StateName::FRAMEBUFFER_OBJECT, target });
	}
	void pop_framebuffer_state(GLenum target) const {
		pop_context_server_state(resources, { StateName::FRAMEBUFFER_OBJECT, target });
	}

	void bind_image_texture(std::uint32_t unit, std::uint32_t texture, int level, bool layered, int layer, GLenum access, GLenum format) const {
		set_context_server_state(resources,
								 { StateName::IMAGE_OBJECT, unit },
								 std::make_tuple(texture, level, layered, layer, access, format),
								 glBindImageTexture,
								 unit, texture, level, layered, layer, access, format);
	}
	void push_image_texture_state(std::uint32_t unit) const {
		push_context_server_state(resources, { StateName::IMAGE_OBJECT, unit });
	}
	void pop_image_texture_state(std::uint32_t unit) const {
		pop_context_server_state(resources, { StateName::IMAGE_OBJECT, unit });
	}

	void bind_renderbuffer(GLenum target, std::uint32_t id) const {
		set_context_server_state(resources,
								 { StateName::RENDERBUFFER_OBJECT, target },
								 std::make_tuple(id),
								 glBindRenderbuffer,
								 target, id);
	}
	void push_renderbuffer_state(GLenum target) const {
		push_context_server_state(resources, { StateName::RENDERBUFFER_OBJECT, target });
	}
	void pop_renderbuffer_state(GLenum target) const {
		pop_context_server_state(resources, { StateName::RENDERBUFFER_OBJECT, target });
	}

	void bind_vertex_array(std::uint32_t id) const {
		set_context_server_state(resources,
								 { StateName::VERTEX_ARRAY_OBJECT },
								 std::make_tuple(id),
								 glBindVertexArray,
								 id);
	}
	void push_vertex_array_state() const {
		push_context_server_state(resources, { StateName::VERTEX_ARRAY_OBJECT });
	}
	void pop_vertex_array_state() const {
		pop_context_server_state(resources, { StateName::VERTEX_ARRAY_OBJECT });
	}

	void bind_shader_program(std::uint32_t id) const {
		set_context_server_state(resources,
								 { StateName::GLSL_PROGRAM_OBJECT },
								 std::make_tuple(id),
								 glUseProgram,
								 id);
	}
	void push_shader_program_state(GLenum target) const {
		push_context_server_state(resources, { StateName::GLSL_PROGRAM_OBJECT });
	}
	void pop_shader_program_state(GLenum target) const {
		pop_context_server_state(resources, { StateName::GLSL_PROGRAM_OBJECT });
	}

public:
	void bind_sampler(std::uint32_t unit, std::uint32_t id) const {
		set_context_server_state(states,
								 { StateName::SAMPLER_OBJECT, unit },
								 std::make_tuple(id),
								 glBindSampler,
								 unit, id);
	}
	void push_sampler_state(std::uint32_t unit) const {
		push_context_server_state(states, { StateName::SAMPLER_OBJECT, unit });
	}
	void pop_sampler_state(std::uint32_t unit) const {
		pop_context_server_state(states, { StateName::SAMPLER_OBJECT, unit });
	}

	void bind_transform_feedback(GLenum target, std::uint32_t id) const {
		set_context_server_state(states,
								 { StateName::TRANSFORM_FEEDBACK_OBJECT, target },
								 std::make_tuple(id),
								 glBindTransformFeedback,
								 target, id);
	}
	void push_transform_feedback_state(GLenum target) const {
		push_context_server_state(states, { StateName::TRANSFORM_FEEDBACK_OBJECT, target });
	}
	void pop_transform_feedback_state(GLenum target) const {
		pop_context_server_state(states, { StateName::TRANSFORM_FEEDBACK_OBJECT, target });
	}

public:
	void enable_depth_test() const { enable_state(BasicStateName::DEPTH_TEST); }
	void disable_depth_test() const { disable_state(BasicStateName::DEPTH_TEST); }

	void enable_state(BasicStateName state) const {
		set_context_server_state(basic_states,
								 state,
								 true);
	}
	void disable_state(BasicStateName state) const {
		set_context_server_state(basic_states,
								 state,
								 false);
	}
	void set_state(BasicStateName state, bool enable) const {
		enable ? enable_state(state) : disable_state(state);
	}
	bool is_enabled(BasicStateName state) const {
		return get_context_server_state(basic_states, state).get_state();
	}

public:
	void push_state(BasicStateName state) const {
		push_context_server_state(basic_states, state);
	}
	void pop_state(BasicStateName state) const {
		pop_context_server_state(basic_states, state);
	}
	void restore_default(BasicStateName state) const {
		if (state == BasicStateName::DITHER ||
			state == BasicStateName::MULTISAMPLE)
			set_context_server_state(basic_states,
									 state,
									 true);
		else
			set_context_server_state(basic_states,
									 state,
									 false);
	}

	void push_state(const context_state_key<StateName> &state) const {
		push_context_server_state(states, state);
	}
	void pop_state(const context_state_key<StateName> &state) const {
		pop_context_server_state(states, state);
	}

public:
	void draw_arrays(GLenum mode, std::int32_t first, std::uint32_t count) const {
		glDrawArrays(mode, first, count);
	}

	void draw_arrays_instanced(GLenum mode, std::int32_t first, std::uint32_t count, std::uint32_t instances) const {
		glDrawArraysInstanced(mode, first, count, instances);
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
		draw_multi_elements_indirect(mode, gl_type_name_enum<T>::gl_enum, ind, drawcount, stride);
	}
	void draw_multi_elements_indirect(GLenum mode, GLenum type, const void* ind, std::uint32_t drawcount, std::uint32_t stride) const {
		glMultiDrawElementsIndirect(mode, type, ind, drawcount, stride);
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
	gl_generic_context() {}
	virtual ~gl_generic_context() noexcept {}
};

}
}
}
