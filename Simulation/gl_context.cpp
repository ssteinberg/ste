
#include "stdafx.h"
#include <GL/glew.h>

#include "gl_extensions.h"

#include "gl_context.h"
#include "gl_current_context.h"
#include "Log.h"

#include "AttributedString.h"

#include <functional>
#include <stdexcept>

using namespace StE::LLR;

class ste_context_intializer {
private:
	friend class gl_context;

	ste_context_intializer() { glfwInit(); }

	std::atomic<bool> initialized{ false };
	bool init_extensions() { 
		bool f = false; 
		if (initialized.compare_exchange_strong(f, true)) {
			glewExperimental = true; 
			return glewInit() == GLEW_OK && init_glext();
		}
		return true;
	}
public:
	~ste_context_intializer() {
		glfwTerminate();
	}
};

gl_context::gl_context(const context_settings &settings, const char *title, const glm::i32vec2 &size, gli::format format, gli::format depth_format) : ctx_settings(settings) {
	static ste_context_intializer ste_global_context_initializer;

	this->window = create_window(title, size, format, depth_format);
	make_current();

	ste_log() << "Context created and made current.";

	if (settings.vsync)
		 glfwSwapInterval(settings.vsync.get() ? 1 : 0);

 	if (!ste_global_context_initializer.init_extensions()) {
 		ste_log_fatal() << "Couldn't fetch OpenGL function pointers." << std::endl;
 		throw std::runtime_error("Couldn't fetch OpenGL function pointers.");
 	}
	if (!glCreateTextures || !glNamedFramebufferDrawBuffers) {
		ste_log_fatal() << "Not a valid 4.5 OpenGL context." << std::endl;
		throw std::runtime_error("Not a valid 4.5 OpenGL context.");
	}
	
	// Mandatory extensions
	if (!this->is_extension_supported("GL_ARB_sparse_buffer")) {
		ste_log_fatal() << "Mandatory extension \"GL_ARB_sparse_buffer\" missing." << std::endl;
		throw std::runtime_error("Mandatory extension \"GL_ARB_sparse_buffer\" missing.");
	}
	if (!this->is_extension_supported("GL_ARB_bindless_texture")) {
		ste_log_fatal() << "Mandatory extension \"GL_ARB_bindless_texture\" missing." << std::endl;
		throw std::runtime_error("Mandatory extension \"GL_ARB_bindless_texture\" missing.");
	}
	if (!this->is_extension_supported("GL_ARB_shader_storage_buffer_object")) {
		ste_log_fatal() << "Mandatory extension \"GL_ARB_shader_storage_buffer_object\" missing." << std::endl;
		throw std::runtime_error("Mandatory extension \"GL_ARB_shader_storage_buffer_object\" missing.");
	}
	if (!this->is_extension_supported("GL_NV_gpu_shader5")) {
		ste_log_fatal() << "Mandatory extension \"GL_NV_gpu_shader5\" missing." << std::endl;
		throw std::runtime_error("Mandatory extension \"GL_NV_gpu_shader5\" missing.");
	}
	if (!this->is_extension_supported("GL_EXT_texture_filter_anisotropic")) {
		ste_log_fatal() << "Mandatory extension \"GL_EXT_texture_filter_anisotropic\" missing." << std::endl;
		throw std::runtime_error("Mandatory extension \"GL_EXT_texture_filter_anisotropic\" missing.");
	}

	if (is_debug_context())
		setup_debug_context();

	create_default_framebuffer(format, depth_format);
}

gl_context::window_type gl_context::create_window(const char * title, const glm::i32vec2 &size, gli::format format, gli::format depth_format) {
	auto format_flags = gli::detail::get_format_info(format).Flags;
	auto depth_format_flags = gli::detail::get_format_info(depth_format).Flags;

	bool is_depth = depth_format_flags & gli::detail::CAP_DEPTH_BIT;

	bool compressed = format_flags & gli::detail::CAP_COMPRESSED_BIT;
	bool srgb = format_flags & gli::detail::CAP_COLORSPACE_SRGB_BIT;
	bool integer = format_flags & gli::detail::CAP_UNSIGNED_BIT;
	bool normalized = format_flags & gli::detail::CAP_NORMALIZED_BIT;

	auto bpp = gli::detail::bits_per_pixel(format);
	auto components = gli::component_count(format);

	if (compressed) {
		ste_log_fatal() << "Creating context failed: Compressed formats are unsupported." << std::endl;
		throw std::runtime_error("Couldn't create context.");
	}
	if (!integer || !normalized) {
		ste_log_fatal() << "Creating context failed: Format must be an integer normalized format." << std::endl;
		throw std::runtime_error("Couldn't create context.");
	}
	if (!is_depth) {
		ste_log_fatal() << "Creating context failed: Invalid depth format." << std::endl;
		throw std::runtime_error("Couldn't create context.");
	}

	auto bpc = bpp / components;
	auto red_bits = bpc;
	auto green_bits = components > 1 ? bpc : 0;
	auto blue_bits = components > 2 ? bpc : 0;
	auto alpha_bits = components > 3 ? bpc : 0;
	auto depth_bits = depth_format == gli::format::FORMAT_D24_UNORM_PACK32 ? 24 : gli::detail::bits_per_pixel(depth_format);
	bool debug = is_debug_context();

	ste_log() << "Creating OpenGL" << (debug ? " DEBUG " : " ") << "rendering context (" << size.x << "px x " << size.y << "px - " << bpp << " bits/pixel (" << red_bits << "," << green_bits << "," << blue_bits << "," << alpha_bits << ") - " << depth_bits << " depth bits - " << ")" << std::endl;

	glfwWindowHint(GLFW_DOUBLEBUFFER, true);
	glfwWindowHint(GLFW_RESIZABLE, true);
	glfwWindowHint(GLFW_VISIBLE, true);

	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	if (debug)
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	glfwWindowHint(GLFW_RED_BITS, red_bits);
	glfwWindowHint(GLFW_GREEN_BITS, green_bits);
	glfwWindowHint(GLFW_BLUE_BITS, blue_bits);
	glfwWindowHint(GLFW_ALPHA_BITS, alpha_bits);
	glfwWindowHint(GLFW_DEPTH_BITS, depth_bits);
	if (ctx_settings.samples)
		glfwWindowHint(GLFW_SAMPLES, ctx_settings.samples.get());
	if (srgb)
		glfwWindowHint(GLFW_SRGB_CAPABLE, true);

	window_type win = window_type(glfwCreateWindow(size.x, size.y, title, ctx_settings.fs && ctx_settings.fs.get() ? glfwGetPrimaryMonitor() : nullptr, nullptr), [](GLFWwindow *win) { glfwDestroyWindow(win); });
	if (win == nullptr) {
		ste_log_fatal() << "Window creation failed!" << std::endl;
		throw std::runtime_error("Couldn't create window.");
		return nullptr;
	}

	cull_face(GL_BACK);
	front_face(GL_CCW);

	return win;
}

void gl_context::create_default_framebuffer(gli::format format, gli::format depth_format) {
	auto format_flags = gli::detail::get_format_info(format).Flags;
	bool srgb = format_flags & gli::detail::CAP_COLORSPACE_SRGB_BIT;

	glm::i32vec2 ret;
	glfwGetFramebufferSize(window.get(), &ret.x, &ret.y);
	viewport(0, 0, ret.x, ret.y);
	default_fb = std::make_unique<context_framebuffer>(ret, format);
	if (srgb)
		enable_state(GL_FRAMEBUFFER_SRGB);
}

void gl_context::make_current() {
	glfwMakeContextCurrent(window.get());
	gl_current_context::current = this;
}

bool gl_context::is_debug_context() const {
	if (ctx_settings.debug_context)
		return ctx_settings.debug_context.get();
#ifdef DEBUG
	return true;
#else
	return false;
#endif
}

void gl_context::setup_debug_context() {
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback([](GLenum source,
							  GLenum type,
							  GLuint id,
							  GLenum severity,
							  GLsizei length,
							  const GLchar* message,
							  const void* userParam) {
		using namespace StE::Text::Attributes;

		Text::AttributedString attr_str = b("OpenGL Debug Output: ") + "Object - " + std::to_string(id) + " " + std::string(message, length);
		switch (severity) {
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			if (type == GL_DEBUG_TYPE_OTHER) {
				ste_log_every(1000) << attr_str << std::endl;
			}
			else {
				ste_log() << attr_str << std::endl;
			}
			break;
		case GL_DEBUG_SEVERITY_LOW:
		case GL_DEBUG_SEVERITY_MEDIUM:
			ste_log_warn() << attr_str << std::endl;
			break;
		case GL_DEBUG_SEVERITY_HIGH:
			ste_log_error() << attr_str << std::endl;
			break;
		default:
			break;
		}
	}, nullptr);
}

void gl_context::resize(const glm::i32vec2 &size) {
	viewport(0, 0, size.x, size.y);
	default_fb = std::unique_ptr<context_framebuffer>(new context_framebuffer(size, framebuffer_format()));
}
