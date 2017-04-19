//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <pipeline_binding_set_layout_impl.hpp>
#include <pipeline_external_binding_layout.hpp>

namespace StE {
namespace GL {

using pipeline_external_binding_set_layout = _internal::pipeline_binding_set_layout_impl<pipeline_external_binding_layout>;

}
}
