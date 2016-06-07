// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <memory>

namespace StE {

namespace _detail {

struct function_wrapper_type_erasure_base {
	virtual void operator()() = 0;
	virtual ~function_wrapper_type_erasure_base() {}
};
template <typename F>
struct function_wrapper_type_erasure_impl : function_wrapper_type_erasure_base {
	F f;
	function_wrapper_type_erasure_impl(F &&f) : f(std::move(f)) {}
	void operator()() override final { f(); }
};

}

class shared_function_wrapper {
private:
	std::shared_ptr<_detail::function_wrapper_type_erasure_base> functor;

public:
	shared_function_wrapper() = default;
	template <typename F>
	shared_function_wrapper(F &&f) : functor(std::make_shared<_detail::function_wrapper_type_erasure_impl<F>>(std::move(f))) {}

	void operator()() const { (*functor)(); }
};

class unique_function_wrapper {
private:
	std::unique_ptr<_detail::function_wrapper_type_erasure_base> functor;

public:
	unique_function_wrapper() = default;
	template <typename F>
	unique_function_wrapper(F &&f) : functor(std::make_unique<_detail::function_wrapper_type_erasure_impl<F>>(std::move(f))) {}

	void operator()() const { (*functor)(); }
};

}
