// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_shader_stage_variable_type.hpp>
#include <ste_shader_stage_variable.hpp>

#include <block_layout.hpp>

#include <lib/string.hpp>
#include <lib/unique_ptr.hpp>
#include <lib/vector.hpp>

namespace ste {
namespace gl {

namespace _detail {

template <std::size_t a, typename... Ts>
auto ste_shader_stage_variable_for_blocks(const block_layout<a, Ts...> &,
										  const lib::string &name,
										  std::uint16_t offset);

// Variable class type from ste_shader_stage_variable_type
template <ste_shader_stage_variable_type var_type>
struct ste_shader_stage_variable_for_variable_type {};
template <>
struct ste_shader_stage_variable_for_variable_type<ste_shader_stage_variable_type::bool_t> {
	using type = ste_shader_stage_variable_scalar;
};
template <>
struct ste_shader_stage_variable_for_variable_type<ste_shader_stage_variable_type::int_t> {
	using type = ste_shader_stage_variable_scalar;
};
template <>
struct ste_shader_stage_variable_for_variable_type<ste_shader_stage_variable_type::uint_t> {
	using type = ste_shader_stage_variable_scalar;
};
template <>
struct ste_shader_stage_variable_for_variable_type<ste_shader_stage_variable_type::float_t> {
	using type = ste_shader_stage_variable_scalar;
};
template <>
struct ste_shader_stage_variable_for_variable_type<ste_shader_stage_variable_type::image_t> {
	using type = ste_shader_stage_variable_opaque;
};
template <>
struct ste_shader_stage_variable_for_variable_type<ste_shader_stage_variable_type::storage_image_t> {
	using type = ste_shader_stage_variable_opaque;
};
template <>
struct ste_shader_stage_variable_for_variable_type<ste_shader_stage_variable_type::sampler_t> {
	using type = ste_shader_stage_variable_opaque;
};
template <>
struct ste_shader_stage_variable_for_variable_type<ste_shader_stage_variable_type::texture_t> {
	using type = ste_shader_stage_variable_opaque;
};
template <>
struct ste_shader_stage_variable_for_variable_type<ste_shader_stage_variable_type::opaque_t> {
	using type = ste_shader_stage_variable_opaque;
};
template <>
struct ste_shader_stage_variable_for_variable_type<ste_shader_stage_variable_type::struct_t> {
	using type = ste_shader_stage_variable_struct;
};
template <ste_shader_stage_variable_type var_type>
using ste_shader_stage_variable_for_variable_type_t = typename ste_shader_stage_variable_for_variable_type<var_type>::type;

// Is opaque variable type trait
template <ste_shader_stage_variable_type var_type>
struct ste_shader_stage_variable_type_is_opaque : std::false_type {};
template <>
struct ste_shader_stage_variable_type_is_opaque<ste_shader_stage_variable_type::image_t> : std::true_type {};
template <>
struct ste_shader_stage_variable_type_is_opaque<ste_shader_stage_variable_type::storage_image_t> : std::true_type {};
template <>
struct ste_shader_stage_variable_type_is_opaque<ste_shader_stage_variable_type::sampler_t> : std::true_type {};
template <>
struct ste_shader_stage_variable_type_is_opaque<ste_shader_stage_variable_type::texture_t> : std::true_type {};
template <>
struct ste_shader_stage_variable_type_is_opaque<ste_shader_stage_variable_type::opaque_t> : std::true_type {};
template <ste_shader_stage_variable_type var_type>
static constexpr auto ste_shader_stage_variable_type_is_opaque_v = ste_shader_stage_variable_type_is_opaque<var_type>::value;

// Variable constructor
template <ste_shader_stage_variable_type var_type>
struct ste_shader_stage_variable_from_type_impl {
	static_assert(var_type != ste_shader_stage_variable_type::unknown, "Unknown type");
	static_assert(var_type != ste_shader_stage_variable_type::opaque_t, "Unknown opaque type");

	template <typename T>
	static auto variable(const lib::string &name,
						 std::uint16_t offset,
						 std::enable_if_t<std::is_array_v<T>>* = nullptr) {
		// Create underlying array variable
		using Underlying = std::remove_extent_t<T>;
		auto var = variable<Underlying>(lib::string(typeid(T).name()),
										0);

		static constexpr auto elements = std::extent_v<T>;
		return lib::allocate_unique<ste_shader_stage_variable_array>(std::move(var),
																		 name,
																		 offset,
																		 elements,
																		 sizeof(T));
	}
	template <typename T>
	static auto variable(const lib::string &name,
						 std::uint16_t offset,
						 std::enable_if_t<is_scalar_v<T>>* = nullptr) {
		return lib::allocate_unique<ste_shader_stage_variable_scalar>(var_type,
																		  name,
																		  offset,
																		  sizeof(T) << 3);
	}
	template <typename T>
	static auto variable(const lib::string &name,
						 std::uint16_t offset,
						 std::enable_if_t<std::conjunction_v<std::negation<is_scalar<T>>, is_arithmetic<T>>>* = nullptr) {
		// Create underlying array variable
		using Underlying = remove_extents_t<T>;
		auto var = variable<Underlying>(lib::string(typeid(T).name()), 
										0);

		return lib::allocate_unique<ste_shader_stage_variable_matrix>(std::move(var),
																		  name,
																		  offset,
																		  matrix_rows_count_v<T>,
																		  matrix_columns_count_v<T>);
	}
	template <typename T, ste_shader_stage_variable_type vt = var_type>
	static auto variable(const lib::string &name,
						 std::uint16_t offset,
						 std::enable_if_t<ste_shader_stage_variable_type_is_opaque_v<vt>>* = nullptr) {
		return lib::allocate_unique<ste_shader_stage_variable_opaque>(vt,
																		  name,
																		  offset);
	}
	template <typename T>
	static auto variable(const lib::string &name,
						 std::uint16_t offset,
						 std::enable_if_t<is_block_layout_v<T>>* = nullptr) {
		// Block layout
		return ste_shader_stage_variable_for_blocks(T(),
													name,
													offset);
	}
};

// Variable constructor for block layouts
template <typename Block, int N, typename... Ts>
struct ste_shader_stage_variable_populate_block_elements {};
template <typename Block, int N, typename T, typename... Ts>
struct ste_shader_stage_variable_populate_block_elements<Block, N, T, Ts...> {
	void operator()(lib::vector<lib::unique_ptr<ste_shader_stage_variable>> &elements) {
		// Create current element
		static constexpr auto var_type = ste_shader_stage_variable_type_from_type_v<T>;
		static std::uint16_t offset = static_cast<std::uint16_t>(block_offset_of<N, Block>());
		auto element = ste_shader_stage_variable_from_type_impl<var_type>::template variable<T>(lib::string(typeid(T).name()),
																								offset);
		elements.push_back(std::move(element));

		// Continue
		ste_shader_stage_variable_populate_block_elements<Block, N + 1, Ts...>()(elements);
	}
};
template <typename Block, int N, typename T>
struct ste_shader_stage_variable_populate_block_elements<Block, N, T> {
	void operator()(lib::vector<lib::unique_ptr<ste_shader_stage_variable>> &elements) {
		// Create last element
		static constexpr auto var_type = ste_shader_stage_variable_type_from_type_v<T>;
		static std::uint16_t offset = static_cast<std::uint16_t>(block_offset_of<N, Block>());
		auto element = ste_shader_stage_variable_from_type_impl<var_type>::template variable<T>(lib::string(typeid(T).name()),
																								offset);
		elements.push_back(std::move(element));
	}
};

template <std::size_t a, typename... Ts>
auto ste_shader_stage_variable_for_blocks(const block_layout<a, Ts...> &,
										  const lib::string &name,
										  std::uint16_t offset) {
	using block_t = block_layout<a, Ts...>;

	// Populate block layout variable elements
	lib::vector<lib::unique_ptr<ste_shader_stage_variable>> elements;
	ste_shader_stage_variable_populate_block_elements<block_t, 0, Ts...>()(elements);

	// And create the struct variable
	return lib::allocate_unique<ste_shader_stage_variable_struct>(std::move(elements),
																	  name,
																	  offset);
}

}

/**
 *	@brief	Instantiates a ste_shader_stage_variable object given a type
 */
template <typename T, bool storage_image = false>
auto ste_shader_stage_variable_from_type(const lib::string &name) {
	static constexpr auto var_type = ste_shader_stage_variable_type_from_type_v<T, storage_image>;
	return _detail::ste_shader_stage_variable_from_type_impl<var_type>::template variable<T>(name,
																							 0);
}

}
}
