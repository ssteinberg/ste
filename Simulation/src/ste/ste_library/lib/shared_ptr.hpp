//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <lib/allocator.hpp>
#include <memory>

namespace ste {
namespace lib {

template <typename T>
using shared_ptr = std::shared_ptr<T>;

template <typename T, typename... Args>
shared_ptr<T> allocate_shared(Args&&... args) {
	return std::allocate_shared<T>(allocator<T>(), std::forward<Args>(args)...);
}

// Shared pointer casts

template <class T, class U>
shared_ptr<T> static_pointer_cast(const shared_ptr<U>& r) noexcept {
	auto p = static_cast<typename shared_ptr<T>::element_type*>(r.get());
	return shared_ptr<T>(r, p);
}

template <class T, class U>
shared_ptr<T> dynamic_pointer_cast(const shared_ptr<U>& r) noexcept {
	if (auto p = dynamic_cast<typename shared_ptr<T>::element_type*>(r.get())) {
		return shared_ptr<T>(r, p);
	}
	return shared_ptr<T>();
}

template <class T, class U>
shared_ptr<T> const_pointer_cast(const shared_ptr<U>& r) noexcept {
	auto p = const_cast<typename shared_ptr<T>::element_type*>(r.get());
	return shared_ptr<T>(r, p);
}

template <class T, class U>
shared_ptr<T> reinterpret_pointer_cast(const shared_ptr<U>& r) noexcept {
	auto p = reinterpret_cast<typename shared_ptr<T>::element_type*>(r.get());
	return shared_ptr<T>(r, p);
}

}
}
