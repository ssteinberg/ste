// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "query_object.hpp"

namespace StE {
namespace Core {

class occlusion_query_any_samples : public query_object<core_resource_type::QueryObjectAnySamples> {};
class occlusion_query_any_samples_conservative : public query_object<core_resource_type::QueryObjectAnySamplesConservative> {};

}
}
