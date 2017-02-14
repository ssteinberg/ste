// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <stdafx.hpp>
#include <texture_enums.hpp>

namespace StE {
namespace Core {

class image_handle {
private:
	std::uint64_t handle{ 0 };
	image_access_mode access;

public:
	image_handle() = default;
	image_handle(std::uint64_t handle, image_access_mode access) : handle(handle), access(access) {}

	void make_resident() const { if (!is_resident()) glMakeImageHandleResidentARB(handle, static_cast<GLenum>(access)); }
	void make_nonresident() const { if (is_resident()) glMakeImageHandleNonResidentARB(handle); }
	bool is_resident() const { return glIsImageHandleResidentARB(handle); }

	operator std::uint64_t() const { return handle; }
	std::uint64_t get_handle() const { return handle; }
};

}
}
