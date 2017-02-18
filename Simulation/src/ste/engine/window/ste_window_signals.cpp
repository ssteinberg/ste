
#include <stdafx.hpp>
#include <ste_window_signals.hpp>

#include <GLFW/glfw3.h>

#include <ste_window.hpp>

using namespace StE;

ste_window_signals::ste_window_signals(const ste_window *win) {
	GLFWwindow* winptr = win->get_window_handle();
	glfwSetWindowUserPointer(winptr, this);


	glfwSetFramebufferSizeCallback(winptr, [](GLFWwindow* winptr, int w, int h) {
		auto *ptr = reinterpret_cast<ste_window_signals*>(glfwGetWindowUserPointer(winptr));
		ptr->framebuffer_resize_signal.emit({ w, h });
	});


	glfwSetCursorPosCallback(winptr, [](GLFWwindow* winptr, double xpos, double ypos) {
		auto *ptr = reinterpret_cast<ste_window_signals*>(glfwGetWindowUserPointer(winptr));
		ptr->hid_pointer_movement_signal.emit({ xpos, ypos });
	});

	glfwSetScrollCallback(winptr, [](GLFWwindow* winptr, double xoffset, double yoffset) {
		auto *ptr = reinterpret_cast<ste_window_signals*>(glfwGetWindowUserPointer(winptr));
		ptr->hid_scroll_signal.emit({ xoffset, yoffset });
	});

	glfwSetMouseButtonCallback(winptr, [](GLFWwindow* winptr, int button, int action, int mods) {
		auto *ptr = reinterpret_cast<ste_window_signals*>(glfwGetWindowUserPointer(winptr));
		ptr->hid_pointer_button_signal.emit(HID::pointer::convert_button(button), HID::convert_status(action), static_cast<HID::ModifierBits>(mods));
	});

	glfwSetKeyCallback(winptr, [](GLFWwindow* winptr, int key, int scancode, int action, int mods) {
		auto *ptr = reinterpret_cast<ste_window_signals*>(glfwGetWindowUserPointer(winptr));

		auto k = HID::keyboard::convert_key(key);
		ptr->hid_keyboard_signal.emit(k, scancode, HID::convert_status(action), static_cast<HID::ModifierBits>(mods));
	});
}
