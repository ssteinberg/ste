
#include <stdafx.hpp>
#include <ste_engine_hid.hpp>
#include <ste_engine.hpp>

using namespace StE;

ste_engine_hid::ste_engine_hid(GLFWwindow* winptr) {
	glfwSetCursorPosCallback(winptr, [](GLFWwindow* winptr, double xpos, double ypos) {
		ste_engine *engine_ptr = reinterpret_cast<ste_engine*>(glfwGetWindowUserPointer(winptr));
		engine_ptr->hid_pointer_movement_signal.emit({ xpos, ypos });
	});

	glfwSetScrollCallback(winptr, [](GLFWwindow* winptr, double xoffset, double yoffset) {
		ste_engine *engine_ptr = reinterpret_cast<ste_engine*>(glfwGetWindowUserPointer(winptr));
		engine_ptr->hid_scroll_signal.emit({ xoffset, yoffset });
	});

	glfwSetMouseButtonCallback(winptr, [](GLFWwindow* winptr, int button, int action, int mods) {
		ste_engine *engine_ptr = reinterpret_cast<ste_engine*>(glfwGetWindowUserPointer(winptr));
		engine_ptr->hid_pointer_button_signal.emit(HID::pointer::convert_button(button), HID::convert_status(action), static_cast<HID::ModifierBits>(mods));
	});

	glfwSetKeyCallback(winptr, [](GLFWwindow* winptr, int key, int scancode, int action, int mods) {
		ste_engine *engine_ptr = reinterpret_cast<ste_engine*>(glfwGetWindowUserPointer(winptr));

		auto k = HID::keyboard::convert_key(key);
		engine_ptr->hid_keyboard_signal.emit(k, scancode, HID::convert_status(action), static_cast<HID::ModifierBits>(mods));
	});
}
