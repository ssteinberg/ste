//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <block_layout_element.hpp>

namespace ste {
namespace gl {

template <std::size_t base_alignment, typename... Ts>
struct block_layout {
	using tuple_t = std::tuple<Ts...>;
	using _element_t = _detail::layout_element<base_alignment, 0, Ts...>;

	/*
	*	@brief	Requested block base alignment
	*/
	static constexpr std::size_t block_base_alignment = base_alignment;

	static constexpr std::size_t _round_up(std::size_t s, std::size_t alignment) {
		return ((s + alignment - 1) / alignment) * alignment;
	}

	/*
	*	@brief	Block alignment of this struct
	*/
	static constexpr std::size_t struct_base_alignment = _round_up(block_layout_struct_base_alignment<Ts...>::value, base_alignment);

	/*
	*	@brief	Sizeof this block layout
	*/
	static constexpr std::size_t sizeof_block_element = _round_up(sizeof(_element_t), struct_base_alignment);
	static constexpr std::size_t block_element_padding_bytes = sizeof_block_element - sizeof(_element_t);

	_detail::block_layout_front<_element_t, block_element_padding_bytes> _front;

	template <int N>
	auto& get() {
		static_assert(N < sizeof...(Ts), "N out of range");
		return _detail::block_layout_getter<N>()(_front.block);
	}
	template <int N>
	const auto& get() const {
		static_assert(N < sizeof...(Ts), "N out of range");
		return _detail::block_layout_getter<N>()(_front.block);
	}

	static constexpr std::size_t elements_count = sizeof...(Ts);
	template <int N>
	using type_at = decltype(_detail::block_layout_getter<N>()(_front.block));

	block_layout() = default;
	block_layout(tuple_t &&tuple) {
		using initializer = _detail::block_layout_initialize_block_layout_with_tuple<0, elements_count, tuple_t, block_layout<base_alignment, Ts...>>;
		initializer()(*this, std::move(tuple));
	}
	block_layout(Ts&&... args) : block_layout(tuple_t(std::forward<Ts>(args)...)) {}
};

template <int N, std::size_t base_alignment, typename... Ts>
auto block_offset_of(const block_layout<base_alignment, Ts...> &layout) {
	using block_t = std::remove_cv_t<std::remove_reference_t<decltype(layout)>>;
	return block_offset_of<N, block_t>();
}

template <int N, typename Block>
auto block_offset_of() {
	static_assert(is_block_layout_v<Block>, "Block is not a block_layout");
	// Use this opportunity to assert the generated block layout size
	static_assert(sizeof(Block) == Block::sizeof_block_element);

	return offsetof(Block, _front.block) + _detail::block_layout_member_offset<N>::template offset<decltype(Block::_front.block)>();
}

}
}
