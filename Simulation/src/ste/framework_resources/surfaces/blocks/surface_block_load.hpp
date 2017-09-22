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

enum class block_load_8component_result_type {
	i32,
	fp32,
	other
};
template <block_load_8component_result_type type, typename common_type>
struct block_load_8component_result_impl {};
template <typename common_type>
struct block_load_8component_result_impl<block_load_8component_result_type::fp32, common_type> { __m256 data; };
template <typename common_type>
struct block_load_8component_result_impl<block_load_8component_result_type::i32, common_type> { __m256i data; };
template <typename common_type>
struct block_load_8component_result_impl<block_load_8component_result_type::other, common_type> { common_type data[8]; };

template <typename common_type>
static constexpr block_load_8component_result_type block_load_8component_type_for_block = 
	std::is_same_v<common_type, float> ? block_load_8component_result_type::fp32 : 
	std::is_same_v<common_type, std::int32_t> ? block_load_8component_result_type::i32 : block_load_8component_result_type::other;

template <typename common_type>
using block_load_8component_result_t = block_load_8component_result_impl<block_load_8component_type_for_block<common_type>, common_type>;

// Loader into an fp32 __m256. Source size limited to 32bit.
template <block_type type, unsigned size_bits>
struct block_avx_8_fp32_loader {
	static constexpr bool has_specialization = false;
};

template <>
struct block_avx_8_fp32_loader<block_type::block_fp, 32> {
	static constexpr bool has_specialization = true;

	template <unsigned offset_bits>
	static __m256 load(const std::uint8_t *data, std::size_t stride, unsigned count) {
		__m256 res;
		for (unsigned i = 0; i < count; ++i) {
			auto b = *reinterpret_cast<const std::uint64_t*>(data + i * stride + offset_bits / 8) >> (offset_bits % 8);
			res.m256_f32[i] = *reinterpret_cast<const float*>(&b);
		}
		return res;
	}
};

template <unsigned size_bits>
struct block_avx_8_fp32_loader<block_type::block_sscaled, size_bits> {
	static_assert(size_bits <= 32);
	static constexpr bool has_specialization = true;

	template <unsigned offset_bits>
	static __m256 load(const std::uint8_t *data, std::size_t stride, unsigned count) {
		__m256i res;
		for (unsigned i = 0; i < count; ++i) {
			auto b = *reinterpret_cast<const std::uint64_t*>(data + i * stride + offset_bits / 8) >> (offset_bits % 8);
			res.m256i_i32[i] = *reinterpret_cast<const std::int32_t*>(&b);
		}

		// Convert
		return _mm256_cvtepi32_ps(res);
	}
};

template <unsigned size_bits>
struct block_avx_8_fp32_loader<block_type::block_unorm, size_bits> {
	static_assert(size_bits <= 32);
	static constexpr bool has_specialization = true;

	template <unsigned offset_bits>
	static __m256 load(const std::uint8_t *data, std::size_t stride, unsigned count) {
		const auto mask = (1ull << size_bits) - 1ull;
		__m256i res;
		for (unsigned i = 0; i < count; ++i) {
			auto b = *reinterpret_cast<const std::uint64_t*>(data + i * stride + offset_bits / 8) >> (offset_bits % 8);
			res.m256i_i32[i] = *reinterpret_cast<const std::int32_t*>(&b);
		}

		// Mask
		const auto mask256 = _mm256_set1_epi32(*reinterpret_cast<const std::int32_t*>(&mask));
		res = _mm256_and_si256(res, mask256);

		// Convert to floats
		const auto fp32 = _mm256_cvtepi32_ps(res);

		// Normalize
		const auto mask256_fp32 = _mm256_set1_ps(static_cast<float>(mask));
		return _mm256_div_ps(fp32, mask256_fp32);
	}
};

template <unsigned size_bits>
struct block_avx_8_fp32_loader<block_type::block_srgb, size_bits> {
	static_assert(size_bits <= 32);
	static constexpr bool has_specialization = true;

	template <unsigned offset_bits>
	static __m256 load(const std::uint8_t *data, std::size_t stride, unsigned count) {
		// Read unorm value
		const auto res = block_avx_8_fp32_loader<block_type::block_unorm, size_bits>::template load<offset_bits>(data, stride, count);

		// Compare to 0 and srgb cutoff
		static constexpr float sRGB_to_linear_cutoff = 0.0404482362771082f;
		const auto ge_0 = _mm256_cmp_ps(res, _mm256_set1_ps(.0f), _CMP_GE_OS);
		const auto gt_cutoff = _mm256_cmp_ps(res, _mm256_set1_ps(sRGB_to_linear_cutoff), _CMP_GT_OS);

		// Compute value for [0, cutoff] range: x / 12.92
		auto srgb_less_cutoff = _mm256_div_ps(res, _mm256_set1_ps(12.92f));

		// Compute value for (cutoff, 1] range: ((x+0.055) / 1.055)^2.4
		auto srgb = _mm256_div_ps(_mm256_add_ps(res, _mm256_set1_ps(0.055f)), _mm256_set1_ps(1.055f));
		srgb = exp256_ps(_mm256_mul_ps(_mm256_set1_ps(2.4f), log256_ps(srgb)));

		// ~ge_cutoff
		std::uint32_t mask = 0xFFFFFFFF;
		const auto neg_ge_cutoff = _mm256_xor_ps(gt_cutoff, _mm256_castsi256_ps(_mm256_set1_epi32(*reinterpret_cast<const std::int32_t*>(&mask))));

		// Mask values with comparisons
		srgb_less_cutoff = _mm256_and_ps(srgb_less_cutoff, ge_0);
		srgb_less_cutoff = _mm256_and_ps(srgb_less_cutoff, neg_ge_cutoff);
		srgb = _mm256_and_ps(srgb, gt_cutoff);

		// Or everything together
		return _mm256_or_ps(srgb, srgb_less_cutoff);
	}
};

// Loader into an i32 __m256i. Source size limited to 32bit.
template <block_type type, unsigned size_bits>
struct block_avx_8_i32_loader {
	static constexpr bool has_specialization = false;
};

template <unsigned size_bits>
struct block_avx_8_i32_loader<block_type::block_sint, size_bits> {
	static_assert(size_bits <= 32);
	static constexpr bool has_specialization = true;

	template <unsigned offset_bits>
	static __m256i load(const std::uint8_t *data, std::size_t stride, unsigned count) {
		const auto mask = (1ull << size_bits) - 1ull;
		__m256i res;
		for (unsigned i = 0; i < count; ++i) {
			auto b = *reinterpret_cast<const std::uint64_t*>(data + i * stride + offset_bits / 8) >> (offset_bits % 8);
			res.m256i_i32[i] = *reinterpret_cast<const std::int32_t*>(&b);
		}

		// Mask
		const auto mask256 = _mm256_set1_epi32(*reinterpret_cast<const std::int32_t*>(&mask));
		return _mm256_and_si256(res, mask256);
	}
};

/**
*	@brief	Loads 8 components from 8 consecutive blocks in memory and returns the decoded data in an AVX2 __m256 register.
*
*	@param	block		Address of first block
*	@param	count		Count blocks to read from. Limited to 8.
*
*	@return	Read components
*/
template <gl::component_swizzle component, typename Block>
__m256 load_block_8_components_fp32(const Block *block, unsigned count = 8) {
	static_assert(std::is_same_v<typename Block::common_type, float>);

	static constexpr auto type = Block::blocktype;
	static constexpr unsigned offset_bits = Block::template offset_for_component<component>();
	static constexpr unsigned size_bits = Block::template size_for_component<component>();
	static constexpr unsigned stride = sizeof(Block);
	const auto *data = reinterpret_cast<const std::uint8_t*>(block);

	count = std::min(count, 8u);

	// If we have accelerated AVX2 path, use it, otherwise load manually.
	if constexpr (_detail::block_avx_8_fp32_loader<type, size_bits>::has_specialization) {
		return _detail::block_avx_8_fp32_loader<type, size_bits>::template load<offset_bits>(data, stride, count);
	}
	else {
		__m256 res;
		for (unsigned i = 0; i < count; ++i)
			res.m256_f32[i] = (block + i)->template component<Block::template index_for_component<component>()>();

		return res;
	}
}

/**
*	@brief	Loads 8 components from 8 consecutive blocks in memory and returns the decoded data in an AVX2 __m256i register.
*
*	@param	block		Address of first block
*	@param	count		Count blocks to read from. Limited to 8.
*
*	@return	Read components
*/
template <gl::component_swizzle component, typename Block>
__m256i load_block_8_components_i32(const Block *block, unsigned count = 8) {
	static_assert(std::is_same_v<typename Block::common_type, std::int32_t>);

	static constexpr auto type = Block::blocktype;
	static constexpr unsigned offset_bits = Block::template offset_for_component<component>();
	static constexpr unsigned size_bits = Block::template size_for_component<component>();
	static constexpr unsigned stride = sizeof(Block);
	const auto *data = reinterpret_cast<const std::uint8_t*>(block);

	count = std::min(count, 8u);

	// If we have accelerated AVX2 path, use it, otherwise load manually.
	if constexpr (_detail::block_avx_8_i32_loader<type, size_bits>::has_specialization) {
		return _detail::block_avx_8_i32_loader<type, size_bits>::template load<offset_bits>(data, stride, count);
	}
	else {
		__m256i res;
		for (unsigned i = 0; i < count; ++i)
			res.m256i_i32[i] = (block + i)->template component<Block::template index_for_component<component>()>();

		return res;
	}
}

}

/**
*	@brief	Loads a block from memory and writes the decoded data, in RGBA swizzling, to output buffer.
*
*	@param	block		Input block
*	@param	rgba_output	Output buffer, must be able to hold sizeof(common_type)*4 bytes
*
*	@return	Element count written
*/
template <typename Block>
static std::size_t load_block(const Block &block,
							  typename Block::common_type *rgba_output) {
	using common_type = typename Block::common_type;

	if constexpr (Block::elements > 0) *(rgba_output + 0) = static_cast<common_type>(block.template component<Block::template index_for_component<gl::component_swizzle::r>()>());
	if constexpr (Block::elements > 1) *(rgba_output + 1) = static_cast<common_type>(block.template component<Block::template index_for_component<gl::component_swizzle::g>()>());
	if constexpr (Block::elements > 2) *(rgba_output + 2) = static_cast<common_type>(block.template component<Block::template index_for_component<gl::component_swizzle::b>()>());
	if constexpr (Block::elements > 3) *(rgba_output + 3) = static_cast<common_type>(block.template component<Block::template index_for_component<gl::component_swizzle::a>()>());

	return Block::elements;
}

/**
*	@brief	Loads a block from memory and writes the decoded data, in RGBA swizzling, to output buffer.
*
*	@param	block_data	Input block memory
*	@param	rgba_output	Output buffer, must be able to hold sizeof(common_type)*4 bytes
*
*	@return	Element count written
*/
template <typename Block>
static std::size_t load_block_buffer(const uint8_t *block_data,
									 typename Block::common_type *rgba_output) {
	const auto &block = *reinterpret_cast<const Block*>(block_data);
	return load_block(block, rgba_output);
}

template <typename common_type>
using block_load_8component_result_t = _detail::block_load_8component_result_t<common_type>;

/**
*	@brief	Loads 8 components from 8 consecutive blocks in memory and returns the decoded data.
*
*	@param	block		Address of first block
*	@param	count		Count blocks to read from. Limited to 8.
*
*	@return	Read components
*/
template <gl::component_swizzle component, typename Block>
auto load_block_8component(const Block *block, unsigned count = 8) {
	count = std::min(count, 8u);

	using common_type = typename Block::common_type;
	block_load_8component_result_t<common_type> result;

	// Use dedicated routines for 8-component loading
	if constexpr (_detail::block_load_8component_type_for_block<common_type> == _detail::block_load_8component_result_type::fp32)
		result.data = _detail::load_block_8_components_fp32<component>(block, count);
	else if constexpr (_detail::block_load_8component_type_for_block<common_type> == _detail::block_load_8component_result_type::i32)
		result.data = _detail::load_block_8_components_i32<component>(block, count);
	else {
		// No dedicated routine exists, load manually
		for (unsigned i = 0; i<count; ++i)
			result.data[i] = static_cast<common_type>((block + i)->template component<Block::template index_for_component<component>()>());
	}

	return result;
}

/**
*	@brief	Loads 8 components of 8 consecutive blocks from buffer and returns the decoded data.
*
*	@param	block_data	Input block memory
*	@param	count		Count blocks to read from. Limited to 8.
*
*	@return	Read components
*/
template <gl::component_swizzle component, typename Block>
auto load_block_8component_buffer(const uint8_t *block_data, unsigned count = 8) {
	const auto *block = reinterpret_cast<const Block*>(block_data);
	return load_block_8component<component>(block, count);
}

}
}
