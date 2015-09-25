// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "system_provided_framebuffer.h"

#include <memory>
#include <functional>

#include <gli/gli.hpp>
#include <glfw/glfw3.h>

namespace StE {

class StEngineControl;

namespace LLR {

class RenderContext {
private:
	friend class StE::StEngineControl;

protected:
	RenderContext(RenderContext &&m) = delete;
	RenderContext(const RenderContext &c) = delete;
	RenderContext& operator=(RenderContext &&m) = delete;
	RenderContext& operator=(const RenderContext &c) = delete;

	RenderContext(const char * title, const glm::i32vec2 &size, bool fs = false, bool vsync = true, gli::format format = gli::FORMAT_RGBA8_UNORM, int samples=0, gli::format depth_format = gli::FORMAT_D24_UNORM);

	void resize(const glm::i32vec2 &size);

	std::unique_ptr<GLFWwindow, std::function<void(GLFWwindow*)>> window;

public:
	std::unique_ptr<system_provided_framebuffer> default_framebuffer;

public:
	~RenderContext() {}

	void clear_framebuffer(bool color = true, bool depth = true) const { glClear((color ? GL_COLOR_BUFFER_BIT : 0) | (depth ? GL_DEPTH_BUFFER_BIT : 0)); }
	void enable_depth_test() const { glEnable(GL_DEPTH_TEST); }
	void disable_depth_test() const { glDisable(GL_DEPTH_TEST); }

	gli::format framebuffer_format() const { return default_framebuffer->front_buffer().get_attachment_format(); }
	glm::tvec2<std::size_t> framebuffer_size() const { return default_framebuffer->front_buffer().get_attachment_size(); }
};

}
}
