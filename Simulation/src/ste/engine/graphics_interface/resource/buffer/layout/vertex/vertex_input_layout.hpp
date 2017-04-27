//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <typelist.hpp>
#include <generate_array.hpp>

namespace StE {
namespace GL {

template <typename... Ts>
struct vertex_input_layout {
	using tuple_t = std::tuple<Ts...>;
	static constexpr char _vertex_input_tag = 0;

	static constexpr std::size_t count = sizeof...(Ts);
	template <int N>
	using type_at = typename typelist_type_at<N, Ts...>::type;

	alignas(1) tuple_t data;

	template <int N>
	auto& get() {
		static_assert(N < sizeof...(Ts), "N out of range");
		return std::get<N>(data);
	}
	template <int N>
	const auto& get() const {
		static_assert(N < sizeof...(Ts), "N out of range");
		return std::get<N>(data);
	}

	vertex_input_layout() = default;
};

namespace _detail {

template <typename T>
decltype(T::_vertex_input_tag, std::true_type{}) _vertex_input_layout_is_vertex_input(int);
template <typename T>
std::false_type _vertex_input_layout_is_vertex_input(...) {}
template <typename T>
using _vertex_input_layout_is_vertex_input_tester = decltype(_vertex_input_layout_is_vertex_input<T>(0));
template <typename T>
struct vertex_input_layout_is_vertex_input {
	static constexpr bool value = _vertex_input_layout_is_vertex_input_tester<T>{};
};

template <int N, typename Block>
struct vertex_input_offset_of_helper {
	auto operator()(int i) {
		auto b = Block();
		auto addr = reinterpret_cast<std::size_t>(&b.template get<N>());
		auto base = reinterpret_cast<std::size_t>(&b.data);
		if (N == i)
			return addr - base;

		return vertex_input_offset_of_helper<N - 1, Block>()(i);
	}
};
template <typename Block>
struct vertex_input_offset_of_helper<-1, Block> {
	std::size_t operator()(int i) {
		assert(false && "i out of range");
		return 0;
	}
};

}

template <typename T>
struct is_vertex_input_layout {
	static constexpr bool value = _detail::vertex_input_layout_is_vertex_input<T>::value;
};
template <typename T>
static constexpr auto is_vertex_input_layout_v = is_vertex_input_layout<T>::value;

template <typename Block>
auto vertex_input_offset_of(int element) {
	static_assert(is_vertex_input_layout_v<Block>, "Block is not a vertex_input_layout");
	assert(element < Block::count);

	return _detail::vertex_input_offset_of_helper<Block::count - 1, Block>()(element);
}
template <typename... Ts>
constexpr auto vertex_input_offset_of(int N, const vertex_input_layout<Ts...> &layout) {
	using block_t = std::remove_cv_t<std::remove_reference_t<decltype(layout)>>;
	return vertex_input_offset_of<block_t>(N);
}

}
}
