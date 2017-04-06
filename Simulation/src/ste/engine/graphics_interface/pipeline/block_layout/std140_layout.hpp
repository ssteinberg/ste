//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <block_layout.hpp>

namespace StE {
namespace GL {

template <typename... Ts>
struct std140_layout : block_layout<16, Ts...> {
};

}
}
