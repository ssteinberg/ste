// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "query_object.h"

namespace StE {
namespace LLR {

class OcclusionQueryAnySamples : public query_object<llr_resource_type::LLRQueryObjectAnySamples> {};
class OcclusionQueryAnySamplesConservative : public query_object<llr_resource_type::LLRQueryObjectAnySamplesConservative> {};

}
}
