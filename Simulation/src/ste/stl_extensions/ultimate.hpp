// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <functional>
#include <optional.hpp>

#include <function_traits.hpp>

namespace StE {

/**
 *	@brief	Ultimate stores an expression and invokes it upon destruction.
 */
template <typename L>
class ultimate {
	static_assert(function_traits<L>::arity == 0, "L can not take any arguments");

private:
	optional<L> expr;

public:
	ultimate(L&& expr) : expr(std::move(expr)) {}

	ultimate(ultimate&&) = default;
	ultimate &operator=(ultimate&&) = default;

	ultimate(const ultimate&) = delete;
	ultimate &operator=(const ultimate&) = delete;

	~ultimate() noexcept {
		if (expr)
			expr.get()();
	}
};

/**
 *	@brief	Type-erased ultimate
 */
class ultimate_type_erasure {
private:
	std::shared_ptr<void> capture;

public:
	template <typename F>
	ultimate_type_erasure(ultimate<F> &&u)
		: capture(std::make_shared<ultimate<F>>(std::move(u))) 
	{}
};

/**
*	@brief	Creates an ultimate object with a callable expression
*/
template <typename F>
auto ultimately(F&& f) {
	return ultimate<F>(std::forward<F>(f));
}

}
