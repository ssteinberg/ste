//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <block_layout_element.hpp>

namespace ste {
namespace gl {

namespace _detail {

template <int N, int Elements, typename Block>
struct block_operator_equal_impl {
    bool operator()(const Block &lhs, const Block &rhs) {
        return
            lhs.template get<N>() == rhs.template get<N>() &&
            block_operator_equal_impl<N + 1, Elements, Block>()(lhs, rhs);
    }
};
template <int Elements, typename Block>
struct block_operator_equal_impl<Elements, Elements, Block> {
    bool operator()(const Block &lhs, const Block &rhs) {
        return true;
    }
};

}

template <std::size_t base_alignment, typename... Ts>
struct block_layout {
	using tuple_t = std::tuple<Ts...>;
	using _element_t = _detail::layout_element<base_alignment, 0, Ts...>;

	/*
	*	@brief	Requested block base alignment
	*/
	static constexpr byte_t block_base_alignment = byte_t(base_alignment);

	static constexpr auto _round_up(byte_t s, byte_t alignment) {
		return ((s + byte_t(alignment) - 1_B) / static_cast<std::size_t>(alignment)) * static_cast<std::size_t>(alignment);
	}

	/*
	*	@brief	Block alignment of this struct
	*/
	static constexpr byte_t struct_base_alignment = _round_up(block_layout_struct_base_alignment<Ts...>::value,
															  block_base_alignment);

	/*
	*	@brief	Sizeof this block layout
	*/
	static constexpr byte_t sizeof_block_element = _round_up(byte_t(sizeof(_element_t)), struct_base_alignment);
	static constexpr byte_t block_element_padding_bytes = sizeof_block_element - byte_t(sizeof(_element_t));

	_detail::block_layout_front<_element_t, static_cast<std::size_t>(block_element_padding_bytes)> _front;

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

    bool operator==(const block_layout &rhs) const {
        return _detail::block_operator_equal_impl<0, elements_count, block_layout<base_alignment, Ts...>>()(*this, rhs);
	}
    bool operator!=(const block_layout &rhs) const {
        return !(*this == rhs);
	}
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
	static_assert(byte_t(sizeof(Block)) == Block::sizeof_block_element);

	return byte_t(offsetof(Block, _front.block)) + _detail::block_layout_member_offset<N>::template offset<decltype(Block::_front.block)>();
}

}
}
