
#include "stdafx.h"
#include <gl/glew.h>

#include "RenderContext.h"
#include "Log.h"

#include <functional>

using namespace StE::LLR;

class ste_context_intializer {
private:
	friend class RenderContext;

	ste_context_intializer() { glfwInit(); }

	std::atomic<bool> glew_initialized{ false };
	bool init_glew() { 
		bool f = false; 
		if (glew_initialized.compare_exchange_strong(f, true)) {
			glewExperimental = true; 
			return glewInit() == GLEW_OK;
		}
		return true;
	}
public:
	~ste_context_intializer() {
		glfwTerminate();
	}
};

RenderContext::RenderContext(const char * title, const glm::i32vec2 &size, bool fs, bool vsync, gli::format format, int samples, gli::format depth_format) {
	static ste_context_intializer ste_global_context_initializer;

	auto format_flags = gli::detail::getFormatInfo(format).Flags;
	auto depth_format_flags = gli::detail::getFormatInfo(depth_format).Flags;

	bool is_depth = depth_format_flags & gli::detail::CAP_DEPTH_BIT;

	bool compressed = format_flags & gli::detail::CAP_COMPRESSED_BIT;
	bool packed = format_flags & gli::detail::CAP_PACKED_BIT;
	bool srgb = format_flags & gli::detail::CAP_COLORSPACE_SRGB_BIT;
	bool integer = format_flags & gli::detail::CAP_UNSIGNED_BIT;
	bool normalized = format_flags & gli::detail::CAP_NORMALIZED_BIT;

	auto bpp = gli::detail::bits_per_pixel(format);
	auto components = gli::component_count(format);

	if (packed || compressed) {
		ste_log_fatal() << "Creating context failed: Packed or compressed formats are unsupported." << std::endl;
		throw std::exception("Couldn't create context.");
	}
	if (!integer || !normalized) {
		ste_log_fatal() << "Creating context failed: Format must be an integer normalized format." << std::endl;
		throw std::exception("Couldn't create context.");
	}
	if (!is_depth) {
		ste_log_fatal() << "Creating context failed: Invalid depth format." << std::endl;
		throw std::exception("Couldn't create context.");
	}

	auto bpc = bpp / components;
	auto red_bits = bpc;
	auto green_bits = components > 1 ? bpc : 0;
	auto blue_bits = components > 2 ? bpc : 0;
	auto alpha_bits = components > 3 ? bpc : 0;
	auto depth_bits = depth_format == gli::format::FORMAT_D24_UNORM ? 24 : gli::detail::bits_per_pixel(depth_format);

	ste_log() << "Creating OpenGL rendering context (" << size.x << "px x " << size.y << "px - " << bpp << " bits/pixel (" << red_bits << "," << green_bits << "," << blue_bits << "," << alpha_bits << ") - " << depth_bits << " depth bits - " << samples << " samples)";

	glfwWindowHint(GLFW_DOUBLEBUFFER, true);
	glfwWindowHint(GLFW_RESIZABLE, true);
	glfwWindowHint(GLFW_VISIBLE, true);

	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_RED_BITS, red_bits);
	glfwWindowHint(GLFW_GREEN_BITS, green_bits);
	glfwWindowHint(GLFW_BLUE_BITS, blue_bits);
	glfwWindowHint(GLFW_ALPHA_BITS, alpha_bits);
	glfwWindowHint(GLFW_DEPTH_BITS, depth_bits);
	glfwWindowHint(GLFW_SAMPLES, samples);
	if (srgb)
		glfwWindowHint(GLFW_SRGB_CAPABLE, true);

	window = decltype(window)(glfwCreateWindow(size.x, size.y, title, fs ? glfwGetPrimaryMonitor() : nullptr, nullptr), [](GLFWwindow *win) { glfwDestroyWindow(win); });
	if (window == nullptr) {
		ste_log_fatal() << "Window creation failed!" << std::endl;
		throw std::exception("Couldn't create window.");
	}
	glfwMakeContextCurrent(window.get());
	vsync ? glfwSwapInterval(1) : glfwSwapInterval(0);

	ste_log() << "Context created and made current.";

 	if (!ste_global_context_initializer.init_glew()) {
 		ste_log_fatal() << "Couldn't init GLEW." << std::endl;
 		throw std::exception("Couldn't init GLEW.");
 	}
	if (!glCreateTextures || !glNamedFramebufferDrawBuffers) {
		ste_log_fatal() << "Not a valid 4.5 OpenGL context." << std::endl;
		throw std::exception("Not a valid 4.5 OpenGL context.");
	}

	glm::i32vec2 ret;
	glfwGetFramebufferSize(window.get(), &ret.x, &ret.y);
	glViewport(0, 0, ret.x, ret.y);
	default_fb = std::make_unique<system_provided_framebuffer>(ret, format);

	ste_log_query_and_log_gl_errors();
}

void RenderContext::resize(const glm::i32vec2 &size) {
	glViewport(0, 0, size.x, size.y);
	default_fb = std::unique_ptr<system_provided_framebuffer>(new system_provided_framebuffer(size, framebuffer_format()));
}
