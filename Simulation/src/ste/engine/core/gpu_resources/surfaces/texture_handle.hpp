// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

namespace StE {
namespace Core {

class texture_handle {
private:
	std::uint64_t handle{ 0 };

public:
	texture_handle() = default;
	texture_handle(std::uint64_t handle) : handle(handle) {}

	void make_resident() const { 
		if (!is_resident()) 
			glMakeTextureHandleResidentARB(handle);
	}
	void make_nonresident() const { 
		if (is_resident()) 
			glMakeTextureHandleNonResidentARB(handle);
	}
	bool is_resident() const { 
		return glIsTextureHandleResidentARB(handle);
	}

	operator std::uint64_t() const { return handle; }
	std::uint64_t get_handle() const { return handle; }
};

}
}
