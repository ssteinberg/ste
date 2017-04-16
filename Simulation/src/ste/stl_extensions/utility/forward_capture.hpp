// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <functional>
#include <tuple>

namespace StE {

namespace _detail {

template <typename T>
class forward_capture_impl_by_value {
private:
	T val;

public:
	template <typename S>
	forward_capture_impl_by_value(S&& val) : val(std::forward<S>(val)) {}

	decltype(auto) get() & { return val; }
	decltype(auto) get() && { return std::move(val); }
	decltype(auto) get() const& { return val; }
};

template <typename T>
class forward_capture_impl_by_ref {
private:
	std::reference_wrapper<T> ref;

public:
	forward_capture_impl_by_ref(T& val) : ref(val) {}

	decltype(auto) get() & { return ref.get(); }
	decltype(auto) get() && { return std::move(ref).get(); }
	decltype(auto) get() const& { return ref.get(); }
};

template <typename T>
struct forward_capture_impl : public forward_capture_impl_by_value<T> {
	using Base = forward_capture_impl_by_value<T>;
	using Base::Base;
};
template <typename T>
struct forward_capture_impl<T&> : public forward_capture_impl_by_ref<T> {
	using Base = forward_capture_impl_by_ref<T>;
	using Base::Base;
};

}

template <typename T>
using forward_capture_t = _detail::forward_capture_impl<T>;
template <typename ... Ts>
using forward_capture_pack_t = std::tuple<forward_capture_t<Ts>...>;

template <typename T>
auto forward_capture(T&& val) {
	return _detail::forward_capture_impl<T>(std::forward<T>(val));
}

template <typename ... Ts>
auto forward_capture_pack(Ts&&... params) {
	return std::make_tuple(_detail::forward_capture_impl<Ts>(std::forward<Ts>(params))...);
}

}
