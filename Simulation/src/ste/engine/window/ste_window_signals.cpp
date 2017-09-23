
#include <stdafx.hpp>
#include <ste_window_signals.hpp>

#include <glfw.hpp>

#include <ste_window.hpp>

using namespace ste;

ste_window_signals::ste_window_signals(const ste_window *win) {
	GLFWwindow* winptr = win->get_window_handle();
	glfwSetWindowUserPointer(winptr, this);

	// Attach framebuffer resize signal handler
	glfwSetFramebufferSizeCallback(winptr, [](GLFWwindow* winptr, 
											  int w, 
											  int h) {
		auto *ptr = reinterpret_cast<ste_window_signals*>(glfwGetWindowUserPointer(winptr));
		ptr->framebuffer_resize_signal.emit({ w, h });
	});
	
	// Attach pointer position change signal handler
	glfwSetCursorPosCallback(winptr, [](GLFWwindow* winptr, 
										double xpos, 
										double ypos) {
		auto *ptr = reinterpret_cast<ste_window_signals*>(glfwGetWindowUserPointer(winptr));
		ptr->hid_pointer_movement_signal.emit({ xpos, ypos });
	});

	// Attach scrolling signal handler
	glfwSetScrollCallback(winptr, [](GLFWwindow* winptr, 
									 double xoffset, 
									 double yoffset) {
		auto *ptr = reinterpret_cast<ste_window_signals*>(glfwGetWindowUserPointer(winptr));
		ptr->hid_scroll_signal.emit({ xoffset, yoffset });
	});

	// Attach pointer button signal handler
	glfwSetMouseButtonCallback(winptr, [](GLFWwindow* winptr, 
										  int button, 
										  int action, 
										  int mods) {
		auto *ptr = reinterpret_cast<ste_window_signals*>(glfwGetWindowUserPointer(winptr));

		auto b = static_cast<hid::button>(button);
		auto status = static_cast<hid::status>(action);
		auto modifiers = static_cast<hid::modifier_bits>(mods);
		ptr->hid_pointer_button_signal.emit(b,
											status, 
											modifiers);
	});

	// Attach keyboard key signal handler
	glfwSetKeyCallback(winptr, [](GLFWwindow* winptr, 
								  int key, 
								  int scancode, 
								  int action, 
								  int mods) {
		auto *ptr = reinterpret_cast<ste_window_signals*>(glfwGetWindowUserPointer(winptr));

		auto k =static_cast<hid::key>(key);
		auto key_scancode = static_cast<hid::key_scancode>(scancode);
		auto status = static_cast<hid::status>(action);
		auto modifiers = static_cast<hid::modifier_bits>(mods);
		ptr->hid_keyboard_signal.emit(k,
									  key_scancode,
									  status,
									  modifiers);
	});

	// Attach text input signal handler
	glfwSetCharCallback(winptr, [](GLFWwindow *winptr,
								   std::uint32_t codepoint) {
		auto *ptr = reinterpret_cast<ste_window_signals*>(glfwGetWindowUserPointer(winptr));

		ptr->hid_text_input_signal.emit(codepoint);
	});
}
