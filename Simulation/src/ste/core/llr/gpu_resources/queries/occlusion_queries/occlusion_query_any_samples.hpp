// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "query_object.hpp"

namespace StE {
namespace LLR {

class OcclusionQueryAnySamples : public query_object<llr_resource_type::LLRQueryObjectAnySamples> {};
class OcclusionQueryAnySamplesConservative : public query_object<llr_resource_type::LLRQueryObjectAnySamplesConservative> {};

}
}
