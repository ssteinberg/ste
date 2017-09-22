//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <surface_block.hpp>

#include <immintrin.h>

#ifndef __AVX2__
#define __AVX2__
#include <avx_mathfun.h>
#undef __AVX2__
#else
#include <avx_mathfun.h>
#endif

namespace ste {
namespace resource {

namespace _detail {

// Stores from an fp32 __m256. Source size limited to 32bit.
template <block_type type, unsigned size_bits>
struct block_avx_8_fp32_store {
	static constexpr bool has_specialization = false;
};

template <>
struct block_avx_8_fp32_store<block_type::block_fp, 32> {
	static constexpr bool has_specialization = true;
	static constexpr unsigned size_bits = 32;

	template <unsigned offset_bits>
	static void store(__m256 input, std::uint8_t *data, std::size_t stride, unsigned count) {
		const auto mask = (1ull << size_bits) - 1ull;
		for (unsigned i = 0; i < count; ++i) {
			const auto r = input.m256_f32[i];

			auto m = mask << (offset_bits % 8);
			auto &val = *reinterpret_cast<std::uint64_t*>(data + i * stride + offset_bits / 8);

			// Zero out bits
			val &= ~m;
			// Write new bits
			val += (*reinterpret_cast<const std::uint64_t*>(&r) << (offset_bits % 8)) & m;
		}
	}
};

template <unsigned size_bits>
struct block_avx_8_fp32_store<block_type::block_sscaled, size_bits> {
	static_assert(size_bits <= 32);
	static constexpr bool has_specialization = true;

	template <unsigned offset_bits>
	static void store(__m256 input, std::uint8_t *data, std::size_t stride, unsigned count) {
		const auto mask = (1ull << size_bits) - 1ull;

		// Round and convert to integers
		const auto input_i256 = _mm256_cvtps_epi32(_mm256_round_ps(input, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));
		for (unsigned i = 0; i < count; ++i) {
			const auto r = input_i256.m256i_i32[i];

			auto m = mask << (offset_bits % 8);
			auto &val = *reinterpret_cast<std::uint64_t*>(data + i * stride + offset_bits / 8);

			// Zero out bits
			val &= ~m;
			// Write new bits
			val += (*reinterpret_cast<const std::uint64_t*>(&r) << (offset_bits % 8)) & m;
		}
	}
};

template <unsigned size_bits>
struct block_avx_8_fp32_store<block_type::block_unorm, size_bits> {
	static_assert(size_bits <= 32);
	static constexpr bool has_specialization = true;

	template <unsigned offset_bits>
	static void store(__m256 input, std::uint8_t *data, std::size_t stride, unsigned count) {
		const auto mask = (1ull << size_bits) - 1ull;

		// Clamp, convert to native range, round and convert to integers
		input = _mm256_max_ps(input, _mm256_set1_ps(.0f));
		input = _mm256_min_ps(input, _mm256_set1_ps(1.f));
		input = _mm256_mul_ps(input, _mm256_set1_ps(static_cast<float>(mask)));
		const auto input_i256 = _mm256_cvtps_epi32(_mm256_round_ps(input, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));

		for (unsigned i = 0; i < count; ++i) {
			const auto r = input_i256.m256i_i32[i];

			auto m = mask << (offset_bits % 8);
			auto &val = *reinterpret_cast<std::uint64_t*>(data + i * stride + offset_bits / 8);

			// Zero out bits
			val &= ~m;
			// Write new bits
			val += (*reinterpret_cast<const std::uint64_t*>(&r) << (offset_bits % 8)) & m;
		}
	}
};

template <unsigned size_bits>
struct block_avx_8_fp32_store<block_type::block_srgb, size_bits> {
	static_assert(size_bits <= 32);
	static constexpr bool has_specialization = true;

	template <unsigned offset_bits>
	static void store(__m256 input, std::uint8_t *data, std::size_t stride, unsigned count) {
		const auto mask = (1ull << size_bits) - 1ull;

		// Linear to sRGB
		static constexpr float linear_to_sRGB_cutoff = 0.00313066844250063f;
		const auto gt_cutoff = _mm256_cmp_ps(input, _mm256_set1_ps(linear_to_sRGB_cutoff), _CMP_GT_OS);
		auto srgb_cutoff = _mm256_mul_ps(input, _mm256_set1_ps(12.92f));
		auto srgb = exp256_ps(_mm256_mul_ps(_mm256_set1_ps(1.f / 2.4f), log256_ps(input)));
		srgb = _mm256_sub_ps(_mm256_mul_ps(_mm256_set1_ps(1.055f), srgb), _mm256_set1_ps(0.055f));

		std::uint32_t negate_mask = 0xFFFFFFFF;
		const auto neg_ge_cutoff = _mm256_xor_ps(gt_cutoff, _mm256_castsi256_ps(_mm256_set1_epi32(*reinterpret_cast<const std::int32_t*>(&negate_mask))));

		srgb_cutoff = _mm256_and_ps(srgb_cutoff, neg_ge_cutoff);
		srgb = _mm256_and_ps(srgb, gt_cutoff);
		input = _mm256_or_ps(srgb, srgb_cutoff);

		// Clamp, convert to native range, round and convert to integers
		input = _mm256_max_ps(input, _mm256_set1_ps(.0f));
		input = _mm256_min_ps(input, _mm256_set1_ps(1.f));
		input = _mm256_mul_ps(input, _mm256_set1_ps(static_cast<float>(mask)));
		const auto input_i256 = _mm256_cvtps_epi32(_mm256_round_ps(input, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));

		for (unsigned i = 0; i < count; ++i) {
			const auto r = input_i256.m256i_i32[i];

			auto m = mask << (offset_bits % 8);
			auto &val = *reinterpret_cast<std::uint64_t*>(data + i * stride + offset_bits / 8);

			// Zero out bits
			val &= ~m;
			// Write new bits
			val += (*reinterpret_cast<const std::uint64_t*>(&r) << (offset_bits % 8)) & m;
		}
	}
};

// Stores from an i32 __m256i. Source size limited to 32bit.
template <block_type type, unsigned size_bits>
struct block_avx_8_i32_store {
	static constexpr bool has_specialization = false;
};

template <unsigned size_bits>
struct block_avx_8_i32_store<block_type::block_sint, size_bits> {
	static_assert(size_bits <= 32);
	static constexpr bool has_specialization = true;

	template <unsigned offset_bits>
	static void store(__m256i input, std::uint8_t *data, std::size_t stride, unsigned count) {
		const auto mask = (1ull << size_bits) - 1ull;

		for (unsigned i = 0; i < count; ++i) {
			const auto r = input.m256i_i32[i];

			auto m = mask << (offset_bits % 8);
			auto &val = *reinterpret_cast<std::uint64_t*>(data + i * stride + offset_bits / 8);

			// Zero out bits
			val &= ~m;
			// Write new bits
			val += (*reinterpret_cast<const std::uint64_t*>(&r) << (offset_bits % 8)) & m;
		}
	}
};

/**
*	@brief	Encodes and stores 8 components of 8 consecutive blocks from an AVX2 __m256 register.
*
*	@param	block		Address of first output block
*	@param	data		Input AVX2 register
*	@param	count		Count blocks to store. Limited to 8.
*/
template <gl::component_swizzle component, typename Block>
void store_block_8_components_fp32(Block *block, __m256 data, unsigned count = 8) {
	static_assert(std::is_same_v<typename Block::common_type, float>);

	static constexpr auto type = Block::blocktype;
	static constexpr unsigned offset_bits = Block::template offset_for_component<component>();
	static constexpr unsigned size_bits = Block::template size_for_component<component>();
	static constexpr unsigned stride = sizeof(Block);
	auto *dst = reinterpret_cast<std::uint8_t*>(block);

	count = std::min(count, 8u);

	// If we have accelerated AVX2 path, use it, otherwise store manually.
	if constexpr (_detail::block_avx_8_fp32_store<type, size_bits>::has_specialization) {
		return _detail::block_avx_8_fp32_store<type, size_bits>::template store<offset_bits>(data, dst, stride, count);
	}
	else {
		for (unsigned i = 0; i < count; ++i)
			(block + i)->template component<Block::template index_for_component<component>()>() = static_cast<typename Block::common_type>(data.m256_f32[i]);
	}
}

/**
*	@brief	Encodes and stores 8 components of 8 consecutive blocks from an AVX2 __m256 register.
*
*	@param	block		Address of first output block
*	@param	data		Input AVX2 register
*	@param	count		Count blocks to store. Limited to 8.
*/
template <gl::component_swizzle component, typename Block>
void store_block_8_components_fp32(Block *block, __m256i data, unsigned count = 8) {
	store_block_8_components_fp32(block,
								  _mm256_cvtepi32_ps(data),
								  count);
}

/**
*	@brief	Encodes and stores 8 components of 8 consecutive blocks from buffer.
*
*	@param	block		Address of first output block
*	@param	data		Input AVX2 register
*	@param	count		Count blocks to store. Limited to 8.
*/
template <gl::component_swizzle component, typename src_type, typename Block>
void store_block_8_components_f32(Block *block, const src_type data[8], unsigned count = 8) {
	__m256 input;
	for (int i = 0; i < count; ++i)
		input.m256_f32[i] = static_cast<float>(data[i]);

	store_block_8_components_f32(block,
								 input,
								 count);
}

/**
*	@brief	Encodes and stores 8 components of 8 consecutive blocks from an AVX2 __m256i register.
*
*	@param	block		Address of first output block
*	@param	data		Input AVX2 register
*	@param	count		Count blocks to store. Limited to 8.
*/
template <gl::component_swizzle component, typename Block>
void store_block_8_components_i32(Block *block, __m256i data, unsigned count = 8) {
	static_assert(std::is_same_v<typename Block::common_type, float>);

	static constexpr auto type = Block::blocktype;
	static constexpr unsigned offset_bits = Block::template offset_for_component<component>();
	static constexpr unsigned size_bits = Block::template size_for_component<component>();
	static constexpr unsigned stride = sizeof(Block);
	auto *dst = reinterpret_cast<std::uint8_t*>(block);

	count = std::min(count, 8u);

	// If we have accelerated AVX2 path, use it, otherwise store manually.
	if constexpr (_detail::block_avx_8_i32_store<type, size_bits>::has_specialization) {
		return _detail::block_avx_8_i32_store<type, size_bits>::template store<offset_bits>(data, dst, stride, count);
	}
	else {
		for (unsigned i = 0; i < count; ++i)
			(block + i)->template component<Block::template index_for_component<component>()>() = static_cast<typename Block::common_type>(data.m256i_i32[i]);
	}
}

/**
*	@brief	Encodes and stores 8 components of 8 consecutive blocks from an AVX2 __m256i register.
*
*	@param	block		Address of first output block
*	@param	data		Input AVX2 register
*	@param	count		Count blocks to store. Limited to 8.
*/
template <gl::component_swizzle component, typename Block>
void store_block_8_components_i32(Block *block, __m256 data, unsigned count = 8) {
	store_block_8_components_i32(block,
								 _mm256_cvtps_epi32(_mm256_round_ps(data, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC)),
								 count);
}

/**
*	@brief	Encodes and stores 8 components of 8 consecutive blocks from buffer.
*
*	@param	block		Address of first output block
*	@param	data		Input AVX2 register
*	@param	count		Count blocks to store. Limited to 8.
*/
template <gl::component_swizzle component, typename src_type, typename Block>
void store_block_8_components_i32(Block *block, const src_type data[8], unsigned count = 8) {
	__m256i input;
	for (int i = 0; i < count; ++i)
		input.m256i_i32[i] = static_cast<std::int32_t>(data[i]);

	store_block_8_components_i32(block,
								 input,
								 count);
}

}

/**
*	@brief	Encodes from buffer, assumed to be in RGBA swizzling, to block.
*
*	@param	block			Output block
*	@param	data			Input buffer
*	@param	max_elements	Max elements to read
*/
template <typename src_type, typename Block>
void store_block(Block &block, const src_type *data, std::size_t max_elements = 4) {
	if constexpr (Block::elements > 0)
		block.template component<Block::template index_for_component<gl::component_swizzle::r>()>() =
			static_cast<typename Block::r_comp_type>(max_elements > 0 ? static_cast<typename Block::r_comp_writer_type>(*(data + 0)) : static_cast<typename Block::r_comp_writer_type>(0));
	if constexpr (Block::elements > 1)
		block.template component<Block::template index_for_component<gl::component_swizzle::g>()>() =
			static_cast<typename Block::g_comp_type>(max_elements > 1 ? static_cast<typename Block::g_comp_writer_type>(*(data + 1)) : static_cast<typename Block::g_comp_writer_type>(0));
	if constexpr (Block::elements > 2)
		block.template component<Block::template index_for_component<gl::component_swizzle::b>()>() =
			static_cast<typename Block::b_comp_type>(max_elements > 2 ? static_cast<typename Block::b_comp_writer_type>(*(data + 2)) : static_cast<typename Block::b_comp_writer_type>(0));
	if constexpr (Block::elements > 3) {
		typename Block::a_comp_writer_type a_default_val;
		if constexpr (is_floating_point_v<typename Block::a_comp_writer_type>)
			a_default_val = static_cast<typename Block::a_comp_writer_type>(1.0);
		else
			a_default_val = std::numeric_limits<typename Block::a_comp_type>::max();

		block.template component<Block::template index_for_component<gl::component_swizzle::a>()>() =
			static_cast<typename Block::a_comp_type>(max_elements > 3 ? static_cast<typename Block::a_comp_writer_type>(*(data + 3)) : a_default_val);
	}
}

/**
*	@brief	Encodes and stores 8 components of 8 consecutive blocks from the result of a load operation.
*
*	@param	block		Address of first output block
*	@param	data		Input data. Must be a result of a load operation,
*						i.e. an AVX2 __m256/__m256i register, or 8 component array of a common type.
*	@param	count		Count blocks to store. Limited to 8.
*
*	@return	Read components
*/
template <gl::component_swizzle component, typename Source, typename Block>
void store_block_8component(Block *block, const Source data, unsigned count = 8) {
	using source_type = std::remove_cv_t<std::remove_reference_t<decltype(data)>>;

	// Use dedicated routines for 8-component store
	if constexpr (std::is_same_v<source_type, __m256>)
		_detail::store_block_8_components_fp32<component>(block, data, count);
	else if constexpr (std::is_same_v<source_type, __m256i>)
		_detail::store_block_8_components_i32<component>(block, data, count);
	else {
		// No dedicated routine exists, store manually
		for (unsigned i = 0; i < count; ++i)
			(block + i)->template component<Block::template index_for_component<component>()>() = static_cast<typename Block::template comp_type<component>>(static_cast<typename Block::r_comp_writer_type>(data[i]));
	}
}

/**
*	@brief	Encodes and stores the default value for 8 components of 8 consecutive blocks.
*
*	@param	block		Address of first output block
*	@param	count		Count blocks to store. Limited to 8.
*
*	@return	Read components
*/
template <gl::component_swizzle component, typename Block>
void store_block_8component_default(Block *block, unsigned count = 8) {
	// Create default value
	using default_val_t = typename Block::template comp_writer_type<component>;
	default_val_t default_val;
	if constexpr (component == gl::component_swizzle::a) {
		if constexpr (is_floating_point_v<default_val_t>)
			default_val = static_cast<default_val_t>(1.0);
		else
			default_val = std::numeric_limits<typename Block::a_comp_type>::max();
	}
	else {
		default_val = static_cast<default_val_t>(0);
	}

	// Use dedicated routines for 8-component store for floating point types and sub-32 bit integer types
	if constexpr (Block::template size_for_component<component>() <= 32 && is_floating_point_v<default_val_t>)
		store_block_8component<component>(block,
										  _mm256_set1_ps(static_cast<float>(default_val)),
										  count);
	else if constexpr (Block::template size_for_component<component>() < 32 && !is_floating_point_v<default_val_t>)
		store_block_8component<component>(block,
										  _mm256_set1_epi32(static_cast<std::int32_t>(default_val)),
										  count);
	else {
		// No dedicated routine exists, store manually
		for (unsigned i = 0; i < count; ++i)
			(block + i)->template component<Block::template index_for_component<component>()>() = static_cast<typename Block::template comp_type<component>>(default_val);
	}
}

}
}
