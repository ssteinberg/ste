//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <optional.hpp>

namespace ste {
namespace gl {

namespace _internal {

template <typename T>
struct composite_resource_component_storage {
	optional<T> object;
	const T *pointer{ nullptr };

	composite_resource_component_storage() = delete;
	composite_resource_component_storage(const T *ptr) : pointer(ptr) {}
	composite_resource_component_storage(T &&obj) : object(std::move(obj)) {}

	composite_resource_component_storage(composite_resource_component_storage&&) = default;
	composite_resource_component_storage &operator=(composite_resource_component_storage&&) = default;

	const T &get() const {
		if (object)
			return object.get();
		return *pointer;
	}
};

}

}
}
