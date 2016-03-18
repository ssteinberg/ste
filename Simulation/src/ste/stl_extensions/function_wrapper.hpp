// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <memory>

namespace StE {

class function_wrapper {
private:
	struct base {
		virtual void operator()() = 0;
		virtual ~base() {}
	};
	template <typename F>
	struct impl : base {
		F f;
		impl(F &&f) : f(std::move(f)) {}
		void operator()() { f(); }
	};

	std::shared_ptr<base> functor;

public:
	function_wrapper() = default;
	template <typename F>
	function_wrapper(F &&f) : functor(new impl<F>(std::move(f))) {}

	void operator()() { (*functor)(); }
};

}
