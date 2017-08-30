//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <resource_exceptions.hpp>

namespace ste {
namespace resource {

class surface_error : public resource_exception {
public:
	using resource_exception::resource_exception;
};

class surface_opaque_storage_mismatch_error : public surface_error {
public:
	using surface_error::surface_error;
};

}
}
