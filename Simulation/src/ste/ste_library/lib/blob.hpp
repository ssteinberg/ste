//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <lib/allocator.hpp>

#include <blob.hpp>

namespace ste {
namespace lib {

using blob = ::ste::blob<allocator<char>>;

}
}
