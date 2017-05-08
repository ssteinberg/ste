// StE 
// © Shlomi Steinberg, 2015-2017

#pragma once 

#include <stdexcept> 

namespace ste {
namespace gl {

class framebuffer_exception : public std::runtime_error {
	using Base = std::runtime_error;

public:
	using Base::Base;
};

class framebuffer_layout_multiple_depth_attachments_exception : public framebuffer_exception {
	using Base = framebuffer_exception;

public:
	using Base::Base;
};

class framebuffer_attachment_mismatch_exception : public framebuffer_exception {
	using Base = framebuffer_exception;

public:
	using Base::Base;
};

class framebuffer_invalid_attachment_location_exception : public framebuffer_attachment_mismatch_exception {
	using Base = framebuffer_attachment_mismatch_exception;

public:
	using Base::Base;
};

class framebuffer_attachment_format_mismatch_exception : public framebuffer_attachment_mismatch_exception {
	using Base = framebuffer_attachment_mismatch_exception;

public:
	using Base::Base;
};

}
}