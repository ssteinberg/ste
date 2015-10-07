// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <memory>
#include <atomic>
#include <future>
#include <functional>
#include <type_traits>

#include "optional.h"
#include "function_traits.h"

namespace StE {

class task_scheduler;

template <typename R>
class task {
private:
	using SchedArg = optional<task_scheduler*>;
	using F = std::function<R(SchedArg)>;

	F f;

public:
	template <typename L>
	task(const L &lambda,
		 std::enable_if_t<function_traits<L>::arity == 1>* = 0) :
		f(lambda) {
		static_assert(std::is_constructible<function_traits<L>::arg<0>::t, SchedArg>::value, "Lambda argument must be constructible with SchedArg");
	}
	template <typename L>
	task(L &&lambda,
		 std::enable_if_t<function_traits<L>::arity == 0>* = 0) :
		f([func = std::forward<L>(lambda)](SchedArg sched) -> R { return func(); }) {
	}

	task(const task &) = default;
	task &operator=(const task &) = default;
	task(task &&) = default;
	task &operator=(task &&) = default;

	template <typename L>
	task<function_traits<L>::result_t> then(L &&lambda,
											std::enable_if_t<function_traits<L>::arity == 1>* = 0) {
		static_assert(std::is_constructible<function_traits<L>::arg<0>::t, R>::value, "Lambda argument must be constructible with R");

		return [func = std::forward<L>(lambda), thisf = this->f](SchedArg sched) {
			auto r = thisf(sched);
			return (!sched) ? func(r) : (*sched).schedule_now([=]() { return func(r); }).get();
		};
	}
	template <typename L>
	task<function_traits<L>::result_t> then(L &&lambda,
											std::enable_if_t<function_traits<L>::arity == 2> * = 0) {
		static_assert(std::is_constructible<function_traits<L>::arg<0>::t, SchedArg>::value, "Lambda argument 0 must be constructible with SchedArg");
		static_assert(std::is_constructible<function_traits<L>::arg<1>::t, R>::value, "Lambda argument 1 must be constructible with R");

		return [func = std::forward<L>(lambda), thisf = this->f](SchedArg sched) {
			auto r = thisf(sched);
			return (!sched) ? func(sched, r) : (*sched)->schedule_now([=]() { return func(sched, r); }).get();
		};
	}
	template <typename L>
	task<function_traits<L>::result_t> then_on_main_thread(L &&lambda,
														   std::enable_if_t<function_traits<L>::arity == 2>* = 0) {
		static_assert(std::is_constructible<function_traits<L>::arg<0>::t, SchedArg>::value, "Lambda argument 0 must be constructible with SchedArg");
		static_assert(std::is_constructible<function_traits<L>::arg<1>::t, R>::value, "Lambda argument 1 must be constructible with R");

		return [func = std::forward<L>(lambda), thisf = this->f](SchedArg sched) {
			auto r = thisf(sched);
			return (!sched) ? func(sched, r) : (*sched)->schedule_now_on_main_thread([=]() { return func(sched, r); }).get();
		};
	}

	R operator()(SchedArg sched = none) const { return f(sched); }
};

}

#include "task_scheduler.h"
