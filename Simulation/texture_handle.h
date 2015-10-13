// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

namespace StE {
namespace LLR {

class texture_handle {
private:
	std::uint64_t handle;

public:
	texture_handle() : handle(0) {}
	texture_handle(std::uint64_t handle) : handle(handle) {}

	void make_resident() const { glMakeTextureHandleResidentARB(handle); }
	void make_nonresident() const { glMakeTextureHandleNonResidentARB(handle); }
	bool is_resident() const { return glIsTextureHandleResidentARB(handle); }

	operator std::uint64_t() const { return handle; }
	std::uint64_t get_handle() const { return handle; }
};

}
}
