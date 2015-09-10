// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <memory>

#include <SFML/Window.hpp>

namespace StE {
namespace LLR {

class RenderControl;
class RenderContext {
	friend class RenderControl;

protected:
	RenderContext(RenderContext &&m) = delete;
	RenderContext(const RenderContext &c) = delete;
	RenderContext& operator=(RenderContext &&m) = delete;
	RenderContext& operator=(const RenderContext &c) = delete;

	RenderContext() {}

	std::unique_ptr<sf::Window> create_window(const char *title, int w, int h, bool fs);

public:
	virtual ~RenderContext() {}

	void set_view_port(int x, int y, int w, int h) const { glViewport(x, y, w, h); }
	void clear_framebuffer(bool color = true, bool depth = true) const { glClear((color ? GL_COLOR_BUFFER_BIT : 0) | (depth ? GL_DEPTH_BUFFER_BIT : 0)); }
	void enable_depth_test() const { glEnable(GL_DEPTH_TEST); }
	void disable_depth_test() const { glDisable(GL_DEPTH_TEST); }
};

}
}
