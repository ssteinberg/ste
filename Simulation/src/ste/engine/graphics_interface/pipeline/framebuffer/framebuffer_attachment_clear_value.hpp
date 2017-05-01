//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <cstring>

#include <type_traits>
#include <ste_type_traits.hpp>
#include <format_rtti.hpp>

namespace ste {
namespace gl {

namespace _internal {

struct framebuffer_attachment_clear_value_data {
	float       float32[4];
	int32_t     int32[4];
	uint32_t    uint32[4];
};

struct framebuffer_attachment_clear_value_assign_impl {
	template <typename T>
	static void assigner(framebuffer_attachment_clear_value_data &lhs, const T &rhs,
						 std::enable_if_t<is_vector_v<T>>* = nullptr) {
		lhs = {};

		auto elements = type_elements_count_v<T>;
		for (std::size_t i = 0; i < elements; ++i) {
			const auto &v = rhs[i];
			lhs.int32[i] = static_cast<std::int32_t>(v);
			lhs.uint32[i] = static_cast<std::uint32_t>(v);
			lhs.float32[i] = static_cast<float>(v);
		}
	}
	template <typename T>
	static void assigner(framebuffer_attachment_clear_value_data &lhs, const T &rhs,
						 std::enable_if_t<is_scalar_v<T>>* = nullptr) {
		lhs = {};

		lhs.int32[0] = static_cast<std::int32_t>(rhs);
		lhs.uint32[0] = static_cast<std::uint32_t>(rhs);
		lhs.float32[0] = static_cast<float>(rhs);
	}
	template <typename T>
	static void assigner(VkClearValue &lhs, const T &rhs,
						 std::enable_if_t<!is_vector_v<T> && !is_scalar_v<T>>* = nullptr) {
		static_assert(false, "T must be a scalar or vector type");
	}
};

}

class framebuffer_attachment_clear_value {
private:
	_internal::framebuffer_attachment_clear_value_data val;

public:
	framebuffer_attachment_clear_value() {
		val = {};
	}
	template <
		typename T,
		typename = typename std::enable_if<is_vector_v<T> || is_scalar_v<T>>::type>
	framebuffer_attachment_clear_value(T&& t) {
		*this = std::forward<T>(t);
	}

	framebuffer_attachment_clear_value(framebuffer_attachment_clear_value&&) = default;
	framebuffer_attachment_clear_value &operator=(framebuffer_attachment_clear_value&&) = default;
	framebuffer_attachment_clear_value(const framebuffer_attachment_clear_value&) = default;
	framebuffer_attachment_clear_value &operator=(const framebuffer_attachment_clear_value&) = default;

	template <
		typename T,
		typename = typename std::enable_if<is_vector_v<T> || is_scalar_v<T>>::type
	>
	auto& operator=(const T &t) {
		_internal::framebuffer_attachment_clear_value_assign_impl::assigner(val, t);
		return *this;
	}

	bool operator==(const framebuffer_attachment_clear_value &rhs) const {
		return val.float32[0] == rhs.val.float32[0] &&
			val.float32[1] == rhs.val.float32[1] &&
			val.float32[2] == rhs.val.float32[2] &&
			val.float32[3] == rhs.val.float32[3];
	}
	bool operator!=(const framebuffer_attachment_clear_value &rhs) const {
		return !(*this == rhs);
	}

	/**
	*	@brief	Returns the Vulkan clear value structure based on the image format
	*/
	VkClearValue vk_clear_value(format f) const {
		VkClearValue ret;

		auto d = format_id(f);

		if (d.is_depth) {
			// Depth format
			ret.depthStencil.depth = val.float32[0];
		}
		else {
			// Normalized-integer, scaled-integer and srgb color formats all use float clear value
			bool is_float_like_format = d.is_normalized_integer || d.is_scaled_integer || d.is_srgb;

			// Populate clear value based on format traits
			if (d.is_float || is_float_like_format) {
				static_assert(sizeof(val.float32) == sizeof(ret.color.float32));
				std::memcpy(ret.color.float32, val.float32, sizeof(val.float32));
			}
			else if (d.is_signed) {
				static_assert(sizeof(val.int32) == sizeof(ret.color.int32));
				std::memcpy(ret.color.int32, val.int32, sizeof(val.int32));
			}
			else {
				static_assert(sizeof(val.uint32) == sizeof(ret.color.uint32));
				std::memcpy(ret.color.uint32, val.uint32, sizeof(val.uint32));
			}
		}

		return ret;
	}
};

}
}
