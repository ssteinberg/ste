// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"

#include "buffer_object.hpp"

#include <type_traits>
#include <functional>

namespace StE {
namespace LLR {

template <typename BufferTypeTo, typename BufferTypeFrom>
BufferTypeTo buffer_object_cast(BufferTypeFrom &&s) {
	using S = std::remove_reference_t<std::remove_cv_t<BufferTypeFrom>>;
	using T = std::remove_reference_t<std::remove_cv_t<BufferTypeTo>>;

	static_assert(std::is_base_of<buffer_object<typename T::T, T::access_usage>, T>::value, "BufferTypeTo must be a buffer object");
	static_assert(std::is_base_of<buffer_object<typename S::T, S::access_usage>, S>::value, "BufferTypeFrom must be a buffer object");
	static_assert(T::access_usage == S::access_usage, "Access usage flags must match");

	return BufferTypeTo(std::forward<BufferTypeFrom>(s));
};

}
}
