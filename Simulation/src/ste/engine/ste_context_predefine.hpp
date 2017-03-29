//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

namespace StE {

struct ste_engine_types;
template <typename Types>
class ste_context_impl;

using ste_context = ste_context_impl<ste_engine_types>;

}
