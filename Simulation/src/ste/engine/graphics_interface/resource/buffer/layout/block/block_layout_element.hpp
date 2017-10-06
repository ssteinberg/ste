//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_type_traits.hpp>
#include <block_layout_common.hpp>

namespace ste {
namespace gl {

namespace _detail {

// Disable alignment warnings
#pragma warning (push)
#pragma warning (disable : 4359)

// Force packing
// VC++ specific
#ifdef _MSC_VER
#pragma pack (push, 1)
#else
static_assert(false, "TODO");
#endif

template <typename T, int padding_size>
struct alignas(1) layout_element_padder { alignas(1) T pad[padding_size]; };
template <typename T>
struct alignas(1) layout_element_padder<T, 0> {};

template <typename T, int padding_size>
struct alignas(1) layout_element_member {
	alignas(1) T member;
	alignas(1) layout_element_padder<std::int8_t, padding_size> _padder;
};
template <typename T>
struct alignas(1) layout_element_member<T, 0> {
	alignas(1) T member;
};


template <std::size_t base_alignment, typename T, bool is_block>
struct layout_element_asserter_impl {};
template <std::size_t base_alignment, typename T>
struct layout_element_asserter_impl<base_alignment, T, false> {
	static_assert(!is_matrix<T>::value || matrix_rows_count<T>::value != 3,
				  "Matrix columns must be 2 or 4 component vectors.");
	static_assert(!std::is_array_v<T> || 
				  block_layout_type_base_alignment<T>::value == byte_t(sizeof(std::remove_all_extents_t<T>)),
				  "Arrays in block layout must be manually aligned.");
};
template <std::size_t base_alignment, typename T>
struct layout_element_asserter_impl<base_alignment, T, true> {
	static_assert(byte_t(base_alignment) >= T::block_base_alignment,
				  "T is a block layout of greater base alignment than parent. This happens when mixing blocks of different layouts, "
				  "e.g. std430 inside a std140 layout.");
};
template <std::size_t base_alignment, typename T>
using layout_element_asserter = layout_element_asserter_impl<base_alignment, T, is_block_layout_v<T>>;

template <std::size_t base_alignment, std::size_t accumulated, typename... Ts>
struct alignas(1) layout_element {};

template <std::size_t base_alignment, std::size_t accumulated, typename T0, typename T1, typename... Ts>
struct alignas(1) layout_element<base_alignment, accumulated, T0, T1, Ts...> : layout_element_asserter<base_alignment, T0> {
	static constexpr std::size_t padding_size = static_cast<std::size_t>(block_layout_member_padding<accumulated, T0, T1>::value);
	alignas(1) layout_element_member<T0, padding_size> data;
	alignas(1) layout_element<base_alignment, accumulated + sizeof(T0) + padding_size, T1, Ts...> next;

	static_assert(sizeof(decltype(data)) == sizeof(T0) + padding_size, "Extra padding was inserted!");
};
template <std::size_t base_alignment, std::size_t accumulated, typename T0, typename T1>
struct alignas(1) layout_element<base_alignment, accumulated, T0, T1> : layout_element_asserter<base_alignment, T0> {
	static constexpr std::size_t padding_size = static_cast<std::size_t>(block_layout_member_padding<accumulated, T0, T1>::value);
	alignas(1) layout_element_member<T0, padding_size> data;
	alignas(1) layout_element<base_alignment, accumulated + sizeof(T0) + padding_size, T1> next;

	static_assert(sizeof(decltype(data)) == sizeof(T0) + padding_size, "Extra padding was inserted!");
};
template <std::size_t base_alignment, std::size_t accumulated, typename T0>
struct alignas(1) layout_element<base_alignment, accumulated, T0> : layout_element_asserter<base_alignment, T0> {
	alignas(1) layout_element_member<T0, 0> data;
};

#pragma pack (pop)

template <std::size_t base_alignment, typename T0, typename T1, typename... Ts>
struct layout_element<base_alignment, 0, T0, T1, Ts...> : layout_element_asserter<base_alignment, T0> {
#ifdef _MSC_VER
#pragma pack (push, 1)
#else
	static_assert(false, "TODO");
#endif
	static constexpr std::size_t padding_size = static_cast<std::size_t>(block_layout_member_padding<0, T0, T1>::value);
	alignas(1) layout_element_member<T0, padding_size> data;
	alignas(1) layout_element<base_alignment, sizeof(T0) + padding_size, T1, Ts...> next;
#pragma pack (pop)

	static_assert(sizeof(decltype(data)) == sizeof(T0) + padding_size, "Extra padding was inserted!");
};
template <std::size_t base_alignment, typename T0, typename T1>
struct layout_element<base_alignment, 0, T0, T1> : layout_element_asserter<base_alignment, T0> {
#ifdef _MSC_VER
#pragma pack (push, 1)
#else
	static_assert(false, "TODO");
#endif
	static constexpr std::size_t padding_size = static_cast<std::size_t>(block_layout_member_padding<0, T0, T1>::value);
	alignas(1) layout_element_member<T0, padding_size> data;
	alignas(1) layout_element<base_alignment, sizeof(T0) + padding_size, T1> next;
#pragma pack (pop)

	static_assert(sizeof(decltype(data)) == sizeof(T0) + padding_size, "Extra padding was inserted!");
};
template <std::size_t base_alignment, typename T0>
struct layout_element<base_alignment, 0, T0> : layout_element_asserter<base_alignment, T0> {
#ifdef _MSC_VER
#pragma pack (push, 1)
#else
	static_assert(false, "TODO");
#endif
	alignas(1) layout_element_member<T0, 0> data;
#pragma pack (pop)
};

template <typename ElementT, std::size_t padding_size>
struct block_layout_front {
	ElementT block;
#ifdef _MSC_VER
#pragma pack (push, 1)
#else
	static_assert(false, "TODO");
#endif
	alignas(1) _detail::layout_element_padder<std::int8_t, padding_size> _padder;
#pragma pack (pop)
};
template <typename ElementT>
struct block_layout_front<ElementT, 0> {
	ElementT block;
};

#pragma warning (pop)

template <int N>
struct block_layout_getter {
	template <int b, int a, typename... Ts>
	auto& operator()(layout_element<b, a, Ts...> &element) { return block_layout_getter<N - 1>()(element.next); }
	template <int b, int a, typename... Ts>
	const auto& operator()(const layout_element<b, a, Ts...> &element) { return block_layout_getter<N - 1>()(element.next); }
};
template <>
struct block_layout_getter<0> {
	template <int b, int a, typename... Ts>
	auto& operator()(layout_element<b, a, Ts...> &element) { return element.data.member; }
	template <int b, int a, typename... Ts>
	const auto& operator()(const layout_element<b, a, Ts...> &element) { return element.data.member; }
};

template <int N>
struct block_layout_member_offset {
	template <int b, int a, typename... Ts>
	static constexpr auto offset(const layout_element<b, a, Ts...> &e) {
		using element_t = std::remove_cv_t<std::remove_reference_t<decltype(e)>>;
		return offset<element_t>();
	}
	template <typename ElementT>
	static constexpr auto offset() {
		static constexpr auto offset_to_next = byte_t(offsetof(ElementT, next));
		return offset_to_next + block_layout_member_offset<N - 1>::template offset<decltype(ElementT::next)>();
	}
};
template <>
struct block_layout_member_offset<0> {
	template <int b, int a, typename... Ts>
	static constexpr auto offset(const layout_element<b, a, Ts...> &e) {
		using element_t = std::remove_cv_t<std::remove_reference_t<decltype(e)>>;
		return offset<element_t>();
	}
	template <typename ElementT>
	static constexpr auto offset() {
		return byte_t(offsetof(ElementT, data.member));
	}
};

}

}
}
