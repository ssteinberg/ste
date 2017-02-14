// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <GLFW/glfw3.h>

#include <hid.hpp>

namespace StE {
namespace HID {

class pointer {
public:
	enum class B {
		Unknown = -1,
		Left = GLFW_MOUSE_BUTTON_LEFT,
		Right = GLFW_MOUSE_BUTTON_RIGHT,
		Middle = GLFW_MOUSE_BUTTON_MIDDLE,
		A = GLFW_MOUSE_BUTTON_1,
		B = GLFW_MOUSE_BUTTON_2,
		C = GLFW_MOUSE_BUTTON_3,
		D = GLFW_MOUSE_BUTTON_4,
		E = GLFW_MOUSE_BUTTON_5,
		F = GLFW_MOUSE_BUTTON_6,
		G = GLFW_MOUSE_BUTTON_7,
		H = GLFW_MOUSE_BUTTON_8,
	};
	
	static B convert_button(int button) { return static_cast<B>(button); }

	static HID::Status button_status(GLFWwindow *window, B b) { return HID::convert_status(glfwGetMouseButton(window, static_cast<int>(b))); }
};

}
}
