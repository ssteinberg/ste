// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <GLFW/glfw3.h>

namespace StE {
namespace HID {

enum ModifierBits {
	ModShift = GLFW_MOD_SHIFT,
	ModCtrl = GLFW_MOD_CONTROL,
	ModAlt = GLFW_MOD_ALT,
	ModSuper = GLFW_MOD_SUPER,
};

enum class Status {
	KeyDown = GLFW_PRESS,
	KeyUp = GLFW_RELEASE,
	KeyRepeat = GLFW_REPEAT,
};

static Status convert_status(int status) { return static_cast<Status>(status); }

}
}

#include "Keyboard.hpp"
#include "Pointer.hpp"
