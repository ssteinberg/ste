//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <pipeline_binding_set_layout.hpp>
#include <pipeline_binding_set_impl.hpp>

namespace StE {
namespace GL {

using pipeline_binding_set = _internal::pipeline_binding_set_impl<pipeline_binding_set_layout>;

}
}
