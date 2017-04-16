// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <memory>
#include <functional>

namespace StE {

namespace _detail {

template <typename ... Params>
struct function_wrapper_type_erasure_base {
	virtual void operator()(Params&&... params) = 0;
	virtual ~function_wrapper_type_erasure_base() {}
};
template <typename F, typename ... Params>
struct function_wrapper_type_erasure_impl : function_wrapper_type_erasure_base<Params...> {
	F f;
	function_wrapper_type_erasure_impl(F &&f) : f(std::move(f)) {}
	void operator()(Params&&... params) override final { f(std::forward<Params>(params)...); }
};

}

template <typename ... Params>
class shared_function_wrapper {
private:
	std::shared_ptr<_detail::function_wrapper_type_erasure_base<Params...>> functor;

public:
	shared_function_wrapper() = default;
	template <typename F>
	shared_function_wrapper(F &&f) : functor(std::make_shared<_detail::function_wrapper_type_erasure_impl<F, Params...>>(std::move(f))) {}

	void operator()(Params&&... params) const { (*functor)(std::forward<Params>(params)...); }
};

template <typename ... Params>
class unique_function_wrapper {
private:
	std::unique_ptr<_detail::function_wrapper_type_erasure_base<Params...>> functor;

public:
	unique_function_wrapper() = default;
	template <typename F>
	unique_function_wrapper(F &&f) : functor(std::make_unique<_detail::function_wrapper_type_erasure_impl<F, Params...>>(std::move(f))) {}

	void operator()(Params&&... params) const { (*functor)(std::forward<Params>(params)...); }
};

}
