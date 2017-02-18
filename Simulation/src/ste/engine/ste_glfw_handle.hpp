//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <GLFW/glfw3.h>

namespace StE {

class ste_glfw_handle {
public:
	ste_glfw_handle() { glfwInit(); }
	~ste_glfw_handle() noexcept { glfwTerminate(); }
};

}
