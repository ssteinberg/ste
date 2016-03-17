// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "query_object.hpp"

namespace StE {
namespace Core {

class OcclusionQueryAnySamples : public query_object<core_resource_type::QueryObjectAnySamples> {};
class OcclusionQueryAnySamplesConservative : public query_object<core_resource_type::QueryObjectAnySamplesConservative> {};

}
}
