//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <GLFW/glfw3.h>
#include <log.hpp>
#include <ste_engine_exceptions.hpp>

namespace ste {

class ste_glfw_handle {
public:
	ste_glfw_handle() {
		glfwInit();
		glfwSetErrorCallback([](int err, const char* description) {
			ste_log_error() << "GLFW reported an error (" << err << "): " << description;
			throw ste_engine_glfw_exception(description);
		});
	}
	~ste_glfw_handle() noexcept { glfwTerminate(); }
};

}
