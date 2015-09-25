
#include "stdafx.h"
#include "StEngineControl.h"
#include "Log.h"

#include <chrono>

using namespace StE;

bool StEngineControl::init_render_context(const char *title, const glm::i32vec2 &size, bool fs, bool vsync, gli::format format, int samples, gli::format depth_format) {
	ste_log() << "Creating window " << size.x << "px x " << size.y << "px";

	glfwSetErrorCallback([](int err, const char* description) { ste_log_error() << "GLFW reported an error (" << err << "): " << description; });

	context = std::unique_ptr<LLR::RenderContext>(new LLR::RenderContext(title, size, fs, vsync, format, samples, depth_format));
	if (context->window == nullptr) {
		return false;
	}

	LLR::opengl::dump_gl_info(false);

	glfwSetWindowUserPointer(context->window.get(), this);
	glfwSetFramebufferSizeCallback(context->window.get(), [](GLFWwindow* winptr, int w, int h) {
		StEngineControl *ec_this = reinterpret_cast<StEngineControl*>(glfwGetWindowUserPointer(winptr));
		ec_this->context->resize({ w, h });
		ec_this->set_projection_dirty();

		ec_this->framebuffer_resize_signal.emit({ w, h });
	});

	return true;
}

void StEngineControl::run_loop(std::function<bool()> process) {
	if (context == nullptr) {
		ste_log_fatal() << "run_loop called without context." << std::endl;
		throw std::exception("run_loop called without context.");
	}

	glfwSetCursorPosCallback(context->window.get(), [](GLFWwindow* winptr, double xpos, double ypos) {
		StEngineControl *ec_this = reinterpret_cast<StEngineControl*>(glfwGetWindowUserPointer(winptr));
		ec_this->hid_pointer_movement_signal.emit({ xpos, ypos });
	});

	glfwSetScrollCallback(context->window.get(), [](GLFWwindow* winptr, double xoffset, double yoffset) {
		StEngineControl *ec_this = reinterpret_cast<StEngineControl*>(glfwGetWindowUserPointer(winptr));
		ec_this->hid_scroll_signal.emit({ xoffset, yoffset });
	});

	glfwSetMouseButtonCallback(context->window.get(), [](GLFWwindow* winptr, int button, int action, int mods) {
		StEngineControl *ec_this = reinterpret_cast<StEngineControl*>(glfwGetWindowUserPointer(winptr));
		ec_this->hid_pointer_button_signal.emit(HID::pointer::convert_button(button), HID::convert_status(action), static_cast<HID::ModifierBits>(mods));
	});

	glfwSetKeyCallback(context->window.get(), [](GLFWwindow* winptr, int key, int scancode, int action, int mods) {
		StEngineControl *ec_this = reinterpret_cast<StEngineControl*>(glfwGetWindowUserPointer(winptr));

		auto k = HID::keyboard::convert_key(key);
		ec_this->hid_keyboard_signal.emit(k, scancode, HID::convert_status(action), static_cast<HID::ModifierBits>(mods));
	});

	auto time = std::chrono::high_resolution_clock::now();
	float total_time = .0f;
	int frames = 0;
	bool running = true;
	while (running && !glfwWindowShouldClose(context->window.get())) {
		auto now = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> delta = now - time;
		tpf = delta;
		time = now;
		++frames;
		if ((total_time += delta.count()) > .5f) {
			fps = static_cast<float>(frames) / total_time;
			total_time = .0f;
			frames = 0;
		}

		glfwPollEvents();

		running &= process();

		glfwSwapBuffers(context->window.get());
	}
}
