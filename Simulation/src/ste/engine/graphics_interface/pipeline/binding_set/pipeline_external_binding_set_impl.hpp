//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <pipeline_external_binding_set_layout.hpp>
#include <pipeline_binding_set_impl.hpp>

namespace ste {
namespace gl {

namespace _internal {

using pipeline_external_binding_set_impl = pipeline_binding_set_impl<pipeline_external_binding_set_layout>;

}

}
}
