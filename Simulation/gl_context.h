// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "system_provided_framebuffer.h"
#include "optional.h"

#include <memory>
#include <functional>
#include <unordered_map>

#include <gli/gli.hpp>
#include <glfw/glfw3.h>

namespace StE {

class StEngineControl;

namespace LLR {
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

	window_type create_window(const char *title, const glm::i32vec2 &size, gli::format format, gli::format depth_format);
	void create_default_framebuffer(gli::format format, gli::format depth_format);
	void make_current();

public:
	gl_context(const context_settings &settings, const char *title, const glm::i32vec2 &size, gli::format format = gli::FORMAT_RGBA8_SRGB, gli::format depth_format = gli::FORMAT_D24_UNORM);
	gl_context(const char * title, const glm::i32vec2 &size, gli::format format = gli::FORMAT_RGBA8_SRGB, gli::format depth_format = gli::FORMAT_D24_UNORM) : gl_context(context_settings(), title, size, format, depth_format) {}
	~gl_context() {}

	gl_context(gl_context &&m) = delete;
	gl_context(const gl_context &c) = delete;
	gl_context& operator=(gl_context &&m) = delete;
	gl_context& operator=(const gl_context &c) = delete;

	void clear_framebuffer(bool color = true, bool depth = true) const { glClear((color ? GL_COLOR_BUFFER_BIT : 0) | (depth ? GL_DEPTH_BUFFER_BIT : 0)); }
	void color_mask(bool r, bool b, bool g, bool a) const { glColorMask(r, g, b, a); }
	void enable_depth_test() const { enable_state(GL_DEPTH_TEST); }
	void disable_depth_test() const { disable_state(GL_DEPTH_TEST); }
	void depth_mask(bool mask) const { glDepthMask(mask); }
	void memory_barrier(GLbitfield bits) const { glMemoryBarrier(bits); }

	void enable_state(GLenum state) const {
		auto emplace_result = states.try_emplace(state, false);
		if (!emplace_result.first->second) {
			glEnable(state);
			emplace_result.first->second = true;
		}
	}
	void disable_state(GLenum state) const {
		auto emplace_result = states.try_emplace(state, false);
		if (emplace_result.first->second) {
			glDisable(state);
			emplace_result.first->second = false;
		}
	}

	gli::format framebuffer_format() const { return default_fb->front_buffer().get_attachment_format(); }
	glm::tvec2<std::size_t> framebuffer_size() const { return default_fb->front_buffer().get_attachment_size(); }
	system_provided_framebuffer &defaut_framebuffer() const { return *default_fb; }

	bool is_debug_context() const;
	const context_settings &get_contex_settings() const { return ctx_settings; }
};

}
}
