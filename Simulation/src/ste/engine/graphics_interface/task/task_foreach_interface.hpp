//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <task_interface.hpp>

#include <type_traits>
#include <tuple>

namespace ste {
namespace gl {

namespace _internal {

template <typename Task, typename Tuple, int Size, int N>
struct task_foreach_interface_impl_selector;

template <typename Task, typename Tuple, int N, bool inherits_from_interface>
struct task_foreach_interface_impl {};
template <typename Task, typename Tuple, int N>
struct task_foreach_interface_impl<Task, Tuple, N, false> {
	template <typename T, typename F>
	void operator()(T&& t,
					F&& f) {
		task_foreach_interface_impl_selector<Task, Tuple, std::tuple_size_v<Tuple>, N + 1>::type()(std::forward<T>(t), std::forward<F>(f));
	}
};
template <typename Task, typename Tuple, int N>
struct task_foreach_interface_impl<Task, Tuple, N, true> {
	using Interface = std::tuple_element_t<N, Tuple>;

	template <typename F>
	void operator()(Task *task,
					F&& f) {
		Interface *interface = task;
		f(interface);

		task_foreach_interface_impl_selector<Task, Tuple, std::tuple_size_v<Tuple>, N + 1>::type()(task, std::forward<F>(f));
	}
	template <typename F>
	void operator()(const Task *task,
					F&& f) {
		const Interface *interface = task;
		f(interface);

		task_foreach_interface_impl_selector<Task, Tuple, std::tuple_size_v<Tuple>, N + 1>::type()(task, std::forward<F>(f));
	}
};
template <typename Task>
struct task_foreach_interface_impl_noop {
	template <typename T, typename F>
	void operator()(T&& t, F&& f) {}
};

template <typename Task, typename Tuple, int Size, int N>
struct task_foreach_interface_impl_selector {
	using Interface = std::tuple_element_t<N, Tuple>;
	using type = task_foreach_interface_impl<Task, Tuple, N, std::is_base_of_v<Interface, Task>>;
};
template <typename Task, typename Tuple, int Size>
struct task_foreach_interface_impl_selector<Task, Tuple, Size, Size> {
	using type = task_foreach_interface_impl_noop<Task>;
};

template <typename Task, typename = std::void_t<>>
struct task_foreach_interface {
	template <typename F>
	void operator()(Task *, F&&) {}
	template <typename F>
	void operator()(const Task *, F&&) {}
};
template <typename Task>
struct task_foreach_interface<Task, std::void_t<typename Task::policy::interface_types>> {
	using Interfaces = typename Task::policy::interface_types;
	using selector = typename task_foreach_interface_impl_selector<Task, Interfaces, std::tuple_size_v<Interfaces>, 0>::type;

	template <typename F>
	void operator()(Task *task, F&& f) {
		selector()(task, std::forward<F>(f));
	}
	template <typename F>
	void operator()(const Task *task, F&& f) {
		selector()(task, std::forward<F>(f));
	}
};

}

}
}
