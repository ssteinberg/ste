// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "gl_generic_context.hpp"

#include "optional.hpp"
#include "function_traits.hpp"

#include <memory>
#include <functional>

#include <array>
#include <map>
#include <unordered_map>

#include <gli/gli.hpp>
#include <GLFW/glfw3.h>

namespace StE {

class StEngineControl;

namespace Core {

class context_framebuffer;

namespace GL {

class gl_context : public gl_generic_context {
	using Base = gl_generic_context;

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
	void set_defaults();

	mutable window_type window;
	std::unique_ptr<context_framebuffer> default_fb;
	context_settings ctx_settings;

	window_type create_window(const char *title, const glm::i32vec2 &size, gli::format format, gli::format depth_format);
	void create_default_framebuffer(gli::format format, gli::format depth_format);
	void make_current() override;

public:
	gl_context(const context_settings &settings, const char *title, const glm::i32vec2 &size, gli::format format = gli::FORMAT_RGBA8_SRGB_PACK32, gli::format depth_format = gli::FORMAT_D24_UNORM_PACK32);
	gl_context(const char * title, const glm::i32vec2 &size, gli::format format = gli::FORMAT_RGBA8_SRGB_PACK32, gli::format depth_format = gli::FORMAT_D24_UNORM_PACK32) : gl_context(context_settings(), title, size, format, depth_format) {}
	~gl_context() {}

	gl_context(gl_context &&m) = delete;
	gl_context(const gl_context &c) = delete;
	gl_context& operator=(gl_context &&m) = delete;
	gl_context& operator=(const gl_context &c) = delete;

	bool is_extension_supported(const char *ext) const { return glfwExtensionSupported(ext); }

	int meminfo_total_dedicated_vram() const { int ret; glGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &ret); return ret; }
	int meminfo_total_available_vram() const { int ret; glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &ret); return ret; }
	int meminfo_free_vram() const { int ret; glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &ret); return ret; }

	gli::format framebuffer_format() const;
	glm::ivec2 framebuffer_size() const;
	context_framebuffer &defaut_framebuffer() const;

	auto *get_window() const { return window.get(); }

	bool is_debug_context() const;
	const context_settings &get_contex_settings() const { return ctx_settings; }
};

}
}
}

#include "context_framebuffer.hpp"

namespace StE {
namespace Core {
namespace GL {

gli::format inline gl_context::framebuffer_format() const { return default_fb->front_buffer().get_attachment_format(); }
glm::ivec2 inline gl_context::framebuffer_size() const { return default_fb->front_buffer().get_attachment_size(); }
context_framebuffer inline &gl_context::defaut_framebuffer() const { return *default_fb; }

}
}
}
