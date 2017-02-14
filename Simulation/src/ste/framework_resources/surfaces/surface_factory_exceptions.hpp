// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <resource_loading_exception.hpp>

namespace StE {
namespace Resource {

class surface_error : public resource_loading_exception {
public:
	using resource_loading_exception::resource_loading_exception;
};

class surface_unsupported_format_error : public surface_error {
public:
	using surface_error::surface_error;
};

}
}
