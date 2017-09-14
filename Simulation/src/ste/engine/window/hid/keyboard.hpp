//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <GLFW/glfw3.h>

#include <hid.hpp>
#include <ste_window.hpp>

namespace ste {
namespace hid {

class keyboard {
public:
	/**
	*	@brief	Queries keyboard key status
	*
	*	@param	window	Window
	*	@param	k		Key name
	*/
	static hid::status key_status(const ste_window &window, key k) {
		return static_cast<hid::status>(glfwGetKey(window.get_window_handle(),
												   static_cast<int>(k)));
	}

	/**
	*	@brief	Queries keyboard key status
	*
	*	@param	window		Window
	*	@param	scancode	Key scancode
	*/
	static hid::status key_status(const ste_window &window, key_scancode scancode) {
		return static_cast<hid::status>(glfwGetKey(window.get_window_handle(),
												   scancode));
	}
};

}
}
