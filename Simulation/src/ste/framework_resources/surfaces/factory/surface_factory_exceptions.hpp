//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <surface_exceptions.hpp>

namespace ste {
namespace resource {

class surface_unsupported_format_error : public surface_error {
public:
	using surface_error::surface_error;
};

class surface_format_exception : public surface_error {
public:
	using surface_error::surface_error;
};

}
}
