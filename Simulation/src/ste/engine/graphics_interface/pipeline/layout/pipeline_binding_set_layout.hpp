//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <pipeline_binding_set_layout_impl.hpp>
#include <pipeline_binding_layout.hpp>

namespace ste {
namespace gl {

using pipeline_binding_set_layout = _internal::pipeline_binding_set_layout_impl<const pipeline_binding_layout*>;

}
}
