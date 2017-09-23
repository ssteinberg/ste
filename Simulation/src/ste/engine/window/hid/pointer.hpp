//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <glfw.hpp>

#include <hid.hpp>
#include <ste_window.hpp>

namespace ste {
namespace hid {

class pointer {
public:
	/**
	*	@brief	Queries pointer button status
	*
	*	@param	window	Window
	*	@param	b		Button name
	*/
	static hid::status button_status(const ste_window &window, button b) {
		return static_cast<hid::status>(glfwGetMouseButton(window.get_window_handle(),
														   static_cast<int>(b)));
	}
};

}
}
