//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <srgb.hpp>
#include <half.hpp>

#include <image_view_swizzle.hpp>

namespace ste {
namespace resource {

enum class block_type {
	block_unorm,
	block_snorm,
	block_uscaled,
	block_sscaled,
	block_srgb,
	block_uint,
	block_sint,
	block_fp,
};

namespace _detail {

template <block_type blocktype, int element_bits>
struct block_primary_type_selector {
	static_assert(element_bits <= 64);
	static_assert(element_bits > 0);

	static constexpr bool is_signed = blocktype == block_type::block_snorm || blocktype == block_type::block_sscaled || blocktype == block_type::block_sint;

	using fp_t = std::conditional_t<element_bits <= 32, float, double>;
	using integer_t = std::conditional_t<
		element_bits == 1, bool, std::conditional_t<
		element_bits <= 8, std::int8_t, std::conditional_t<
		element_bits <= 16, std::int16_t, std::conditional_t<
		element_bits <= 32, std::int32_t,
		std::int64_t>>>>;
	using uinteger_t = std::conditional_t<
		element_bits == 1, bool, std::conditional_t<
		element_bits <= 8, std::uint8_t, std::conditional_t<
		element_bits <= 16, std::uint16_t, std::conditional_t<
		element_bits <= 32, std::uint32_t,
		std::uint64_t>>>>;

	static constexpr bool uses_fp = blocktype != block_type::block_uint && blocktype != block_type::block_sint;

	using read_type = std::conditional_t<element_bits <= 32, std::uint32_t, std::uint64_t>;
	using type = std::conditional_t<uses_fp, fp_t, std::conditional_t<is_signed, integer_t, uinteger_t>>;
};
template <int element_bits>
struct block_primary_type_selector<block_type::block_fp, element_bits> {
	static_assert(element_bits <= 64);
	static_assert(element_bits == 16 || element_bits == 32 || element_bits == 64);	// fp16, fp32, fp64 only

	static constexpr bool is_signed = false;
	using read_type = std::conditional_t<element_bits <= 32, std::uint32_t, std::uint64_t>;
	using type = std::conditional_t<element_bits == 32, float, std::conditional_t<element_bits == 64, double, half_float::half>>;
};

template <block_type type, unsigned size_bits, unsigned offset_bits, bool is_const>
class block_element {
	using T = typename block_primary_type_selector<type, size_bits>::type;
	using read_type = typename block_primary_type_selector<type, size_bits>::public_type;

	using ptr_t = std::conditional_t<is_const, const std::uint8_t*, std::uint8_t*>;

	ptr_t b;

	static constexpr auto mask() {
		if constexpr (size_bits == 64)
			return ~(0ull);
		return (1ull << size_bits) - 1ull;
	}

	read_type read() const {
		return (*reinterpret_cast<const read_type*>(b + offset_bits / 8) >> (offset_bits % 8)) & mask();
	}
	template <bool b = is_const, typename = typename std::enable_if_t<!b>>
	void write(read_type r) {
		auto m = mask() << (offset_bits % 8);
		auto &val = *reinterpret_cast<const read_type*>(b + offset_bits / 8);

		// Zero out bits
		val &= ~m;
		// Write new bits
		val += (r << (offset_bits % 8)) & m;
	}

	T convert_read_to_public(read_type data) {
		switch (type) {
		default:
		case block_type::block_unorm:
			return static_cast<T>(data) / static_cast<T>(mask());
		case block_type::block_snorm:
			return static_cast<T>(data) / static_cast<T>((1ull << (size_bits - 1)) - (data > 0 ? 1ull : 0ul));
		case block_type::block_uscaled:
		case block_type::block_sscaled:
			return static_cast<T>(data);
		case block_type::block_srgb:
			auto sRGB = static_cast<T>(data) / static_cast<T>(mask());
			return graphics::sRGB_to_linear(sRGB);
		case block_type::block_uint:
			return static_cast<T>(data);
		case block_type::block_sint:
			return static_cast<T>(data);
		case block_type::block_fp:
			return *reinterpret_cast<const T*>(&data);
		}
	}
	read_type convert_public_to_read(T data) {
		switch (type) {
		case block_type::block_srgb:
			data = graphics::linear_to_sRGB(data);
		default:
		case block_type::block_unorm:
			return static_cast<read_type>(glm::round(glm::clamp<T>(data,0,1) * static_cast<T>(mask())));
		case block_type::block_snorm:
			auto max = (1ull << (size_bits - 1)) - (data > 0 ? 1ull : 0ul);
			return static_cast<read_type>(glm::round(glm::clamp<T>(data, -1, 1) * static_cast<T>(max)));
		case block_type::block_uscaled:
		case block_type::block_sscaled:
			return static_cast<read_type>(glm::round(data));
		case block_type::block_uint:
			return static_cast<read_type>(data);
		case block_type::block_sint:
			return static_cast<read_type>(data);
		case block_type::block_fp:
			std::uint64_t temp = 0;
			*reinterpret_cast<T*>(&temp) = data;
			return *reinterpret_cast<const read_type*>(&temp);
		}
	}

public:
	block_element() = default;
	block_element(ptr_t b) : b(b) {}

	template <bool t = is_const, typename = typename std::enable_if_t<!t>>
	block_element &operator=(const T &input) {
		write(convert_public_to_read(input));
		return *this;
	}

	operator T() const {
		return convert_read_to_public(read());
	}
};

}

template <block_type type, unsigned comp0_bits, unsigned comp1_bits, unsigned comp2_bits, unsigned comp3_bits, gl::component_swizzle comp0, gl::component_swizzle comp1, gl::component_swizzle comp2, gl::component_swizzle comp3>
class block_4components {
	static_assert(comp0 == gl::component_swizzle::r || comp1 == gl::component_swizzle::r || comp2 == gl::component_swizzle::r || comp3 == gl::component_swizzle::r);
	static_assert(comp0 == gl::component_swizzle::g || comp1 == gl::component_swizzle::g || comp2 == gl::component_swizzle::g || comp3 == gl::component_swizzle::g);
	static_assert(comp0 == gl::component_swizzle::b || comp1 == gl::component_swizzle::b || comp2 == gl::component_swizzle::b || comp3 == gl::component_swizzle::b);
	static_assert(comp0 == gl::component_swizzle::a || comp1 == gl::component_swizzle::a || comp2 == gl::component_swizzle::a || comp3 == gl::component_swizzle::a);

	static constexpr auto a_type = type != block_type::block_srgb ? type : block_type::block_unorm;		// sRGB formats always use unorm for the alpha channel

public:
	static constexpr unsigned elements = 4;
	static constexpr block_type blocktype = type;

	static constexpr unsigned r_index = comp0 == gl::component_swizzle::r ? 0 :
		(comp1 == gl::component_swizzle::r ? 1 :
		(comp2 == gl::component_swizzle::r ? 2 : 3));
	static constexpr unsigned g_index = comp0 == gl::component_swizzle::g ? 0 :
		(comp1 == gl::component_swizzle::g ? 1 :
		(comp2 == gl::component_swizzle::g ? 2 : 3));
	static constexpr unsigned b_index = comp0 == gl::component_swizzle::b ? 0 :
		(comp1 == gl::component_swizzle::b ? 1 :
		(comp2 == gl::component_swizzle::b ? 2 : 3));
	static constexpr unsigned a_index = comp0 == gl::component_swizzle::a ? 0 :
		(comp1 == gl::component_swizzle::a ? 1 :
		(comp2 == gl::component_swizzle::a ? 2 : 3));
	static constexpr unsigned r_bits = comp0 == gl::component_swizzle::r ? comp0_bits :
		(comp1 == gl::component_swizzle::r ? comp1_bits :
		(comp2 == gl::component_swizzle::r ? comp2_bits : comp3_bits));
	static constexpr unsigned g_bits = comp0 == gl::component_swizzle::g ? comp0_bits :
		(comp1 == gl::component_swizzle::g ? comp1_bits :
		(comp2 == gl::component_swizzle::g ? comp2_bits : comp3_bits));
	static constexpr unsigned b_bits = comp0 == gl::component_swizzle::b ? comp0_bits :
		(comp1 == gl::component_swizzle::b ? comp1_bits :
		(comp2 == gl::component_swizzle::b ? comp2_bits : comp3_bits));
	static constexpr unsigned a_bits = comp0 == gl::component_swizzle::a ? comp0_bits :
		(comp1 == gl::component_swizzle::a ? comp1_bits :
		(comp2 == gl::component_swizzle::a ? comp2_bits : comp3_bits));
	static constexpr unsigned r_offset = comp0 == gl::component_swizzle::r ? 0 :
		(comp1 == gl::component_swizzle::r ? comp0_bits :
		(comp2 == gl::component_swizzle::r ? comp0_bits + comp1_bits : comp0_bits + comp1_bits + comp2_bits));
	static constexpr unsigned g_offset = comp0 == gl::component_swizzle::g ? 0 :
		(comp1 == gl::component_swizzle::g ? comp0_bits :
		(comp2 == gl::component_swizzle::g ? comp0_bits + comp1_bits : comp0_bits + comp1_bits + comp2_bits));
	static constexpr unsigned b_offset = comp0 == gl::component_swizzle::b ? 0 :
		(comp1 == gl::component_swizzle::b ? comp0_bits :
		(comp2 == gl::component_swizzle::b ? comp0_bits + comp1_bits : comp0_bits + comp1_bits + comp2_bits));
	static constexpr unsigned a_offset = comp0 == gl::component_swizzle::a ? 0 :
		(comp1 == gl::component_swizzle::a ? comp0_bits :
		(comp2 == gl::component_swizzle::a ? comp0_bits + comp1_bits : comp0_bits + comp1_bits + comp2_bits));

	static constexpr int index_for_component(gl::component_swizzle c) {
		if constexpr (c == gl::component_swizzle::r) return r_index;
		if constexpr (c == gl::component_swizzle::g) return g_index;
		if constexpr (c == gl::component_swizzle::b) return b_index;
		if constexpr (c == gl::component_swizzle::a) return a_index;
		static_assert(c == gl::component_swizzle::r || c == gl::component_swizzle::g || c == gl::component_swizzle::b || c == gl::component_swizzle::a);
		return -1;
	}

	static constexpr unsigned total_bits = comp0_bits + comp1_bits + comp2_bits + comp3_bits;
	static constexpr unsigned bytes = total_bits / 8;

	using r_comp_type = typename _detail::block_primary_type_selector<type, r_bits>::type;
	using g_comp_type = typename _detail::block_primary_type_selector<type, g_bits>::type;
	using b_comp_type = typename _detail::block_primary_type_selector<type, b_bits>::type;
	using a_comp_type = typename _detail::block_primary_type_selector<a_type, a_bits>::type;

private:
	static_assert((total_bits % 8) == 0, "Block straddles bytes");
	static_assert(bytes > 0, "bytes is zero");

	using data_t = std::uint8_t[bytes];

	data_t data{};

public:
	block_4components() = default;
	block_4components(r_comp_type in_r, g_comp_type in_g, b_comp_type in_b, a_comp_type in_a) {
		r() = in_r;
		g() = in_g;
		b() = in_b;
		a() = in_a;
	}

	r_comp_type r() const { return _detail::block_element<type, r_bits, r_offset, true>(data); }
	g_comp_type g() const { return _detail::block_element<type, g_bits, g_offset, true>(data); }
	b_comp_type b() const { return _detail::block_element<type, b_bits, b_offset, true>(data); }
	a_comp_type a() const { return _detail::block_element<a_type, a_bits, a_offset, true>(data); }

	auto r() { return _detail::block_element<type, r_bits, r_offset, false>(data); }
	auto g() { return _detail::block_element<type, g_bits, g_offset, false>(data); }
	auto b() { return _detail::block_element<type, b_bits, b_offset, false>(data); }
	auto a() { return _detail::block_element<a_type, a_bits, a_offset, false>(data); }
};

template <block_type type, unsigned comp0_bits, unsigned comp1_bits, unsigned comp2_bits, gl::component_swizzle comp0, gl::component_swizzle comp1, gl::component_swizzle comp2>
class block_3components {
	static_assert(comp0 == gl::component_swizzle::r || comp1 == gl::component_swizzle::r || comp2 == gl::component_swizzle::r);
	static_assert(comp0 == gl::component_swizzle::g || comp1 == gl::component_swizzle::g || comp2 == gl::component_swizzle::g);
	static_assert(comp0 == gl::component_swizzle::b || comp1 == gl::component_swizzle::b || comp2 == gl::component_swizzle::b);

public:
	static constexpr unsigned elements = 3;
	static constexpr block_type blocktype = type;

	static constexpr unsigned r_index = comp0 == gl::component_swizzle::r ? 0 :
		(comp1 == gl::component_swizzle::r ? 1 : 2);
	static constexpr unsigned g_index = comp0 == gl::component_swizzle::g ? 0 :
		(comp1 == gl::component_swizzle::g ? 1 : 2);
	static constexpr unsigned b_index = comp0 == gl::component_swizzle::b ? 0 :
		(comp1 == gl::component_swizzle::b ? 1 : 2);
	static constexpr unsigned r_bits = comp0 == gl::component_swizzle::r ? comp0_bits :
		(comp1 == gl::component_swizzle::r ? comp1_bits : comp2_bits);
	static constexpr unsigned g_bits = comp0 == gl::component_swizzle::g ? comp0_bits :
		(comp1 == gl::component_swizzle::g ? comp1_bits : comp2_bits);
	static constexpr unsigned b_bits = comp0 == gl::component_swizzle::b ? comp0_bits :
		(comp1 == gl::component_swizzle::b ? comp1_bits : comp2_bits);
	static constexpr unsigned r_offset = comp0 == gl::component_swizzle::r ? 0 :
		(comp1 == gl::component_swizzle::r ? comp0_bits : comp0_bits + comp1_bits);
	static constexpr unsigned g_offset = comp0 == gl::component_swizzle::g ? 0 :
		(comp1 == gl::component_swizzle::g ? comp0_bits : comp0_bits + comp1_bits);
	static constexpr unsigned b_offset = comp0 == gl::component_swizzle::b ? 0 :
		(comp1 == gl::component_swizzle::b ? comp0_bits : comp0_bits + comp1_bits);

	static constexpr int index_for_component(gl::component_swizzle c) {
		if constexpr (c == gl::component_swizzle::r) return r_index;
		if constexpr (c == gl::component_swizzle::g) return g_index;
		if constexpr (c == gl::component_swizzle::b) return b_index;
		static_assert(c == gl::component_swizzle::r || c == gl::component_swizzle::g || c == gl::component_swizzle::b);
		return -1;
	}

	static constexpr unsigned total_bits = comp0_bits + comp1_bits + comp2_bits;
	static constexpr unsigned bytes = total_bits / 8;

	using r_comp_type = typename _detail::block_primary_type_selector<type, r_bits>::type;
	using g_comp_type = typename _detail::block_primary_type_selector<type, g_bits>::type;
	using b_comp_type = typename _detail::block_primary_type_selector<type, b_bits>::type;

private:
	static_assert((total_bits % 8) == 0, "Block straddles bytes");
	static_assert(bytes > 0, "bytes is zero");

	using data_t = std::uint8_t[bytes];

	data_t data{};

public:
	block_3components() = default;
	block_3components(r_comp_type in_r, g_comp_type in_g, b_comp_type in_b) {
		r() = in_r;
		g() = in_g;
		b() = in_b;
	}

	r_comp_type r() const { return _detail::block_element<type, r_bits, r_offset, true>(data); }
	g_comp_type g() const { return _detail::block_element<type, g_bits, g_offset, true>(data); }
	b_comp_type b() const { return _detail::block_element<type, b_bits, b_offset, true>(data); }

	auto r() { return _detail::block_element<type, r_bits, r_offset, false>(data); }
	auto g() { return _detail::block_element<type, g_bits, g_offset, false>(data); }
	auto b() { return _detail::block_element<type, b_bits, b_offset, false>(data); }
};

template <block_type type, unsigned comp0_bits, unsigned comp1_bits, gl::component_swizzle comp0, gl::component_swizzle comp1>
class block_2components {
	static_assert(comp0 == gl::component_swizzle::r || comp1 == gl::component_swizzle::r);
	static_assert(comp0 == gl::component_swizzle::g || comp1 == gl::component_swizzle::g);

public:
	static constexpr unsigned elements = 2;
	static constexpr block_type blocktype = type;

	static constexpr unsigned r_index = comp0 == gl::component_swizzle::r ? 0 : 1;
	static constexpr unsigned g_index = comp0 == gl::component_swizzle::g ? 0 : 1;
	static constexpr unsigned r_bits = comp0 == gl::component_swizzle::r ? comp0_bits : comp1_bits;
	static constexpr unsigned g_bits = comp0 == gl::component_swizzle::g ? comp0_bits : comp1_bits;
	static constexpr unsigned r_offset = comp0 == gl::component_swizzle::r ? 0 : comp0_bits;
	static constexpr unsigned g_offset = comp0 == gl::component_swizzle::g ? 0 : comp0_bits;

	static constexpr int index_for_component(gl::component_swizzle c) {
		if constexpr (c == gl::component_swizzle::r) return r_index;
		if constexpr (c == gl::component_swizzle::g) return g_index;
		static_assert(c == gl::component_swizzle::r || c == gl::component_swizzle::g);
		return -1;
	}

	static constexpr unsigned total_bits = comp0_bits + comp1_bits;
	static constexpr unsigned bytes = total_bits / 8;

	using r_comp_type = typename _detail::block_primary_type_selector<type, r_bits>::type;
	using g_comp_type = typename _detail::block_primary_type_selector<type, g_bits>::type;

private:
	static_assert((total_bits % 8) == 0, "Block straddles bytes");
	static_assert(bytes > 0, "bytes is zero");

	using data_t = std::uint8_t[bytes];

	data_t data{};

public:
	block_2components() = default;
	block_2components(r_comp_type in_r, g_comp_type in_g) {
		r() = in_r;
		g() = in_g;
	}

	r_comp_type r() const { return _detail::block_element<type, r_bits, r_offset, true>(data); }
	g_comp_type g() const { return _detail::block_element<type, g_bits, g_offset, true>(data); }

	auto r() { return _detail::block_element<type, r_bits, r_offset, false>(data); }
	auto g() { return _detail::block_element<type, g_bits, g_offset, false>(data); }
};

template <block_type type, unsigned comp0_bits>
class block_1components {
public:
	static constexpr unsigned elements = 1;
	static constexpr block_type blocktype = type;

	static constexpr unsigned r_index = 0;
	static constexpr unsigned r_bits = comp0_bits;
	static constexpr unsigned r_offset = 0;

	static constexpr int index_for_component(gl::component_swizzle c) {
		if constexpr (c == gl::component_swizzle::r) return r_index;
		static_assert(c == gl::component_swizzle::r);
		return -1;
	}

	static constexpr unsigned total_bits = comp0_bits;
	static constexpr unsigned bytes = total_bits / 8;

	using r_comp_type = typename _detail::block_primary_type_selector<type, r_bits>::type;

private:
	static_assert((total_bits % 8) == 0, "Block straddles bytes");
	static_assert(bytes > 0, "bytes is zero");

	using data_t = std::uint8_t[bytes];

	data_t data{};

public:
	block_1components() = default;
	block_1components(r_comp_type in_r) {
		r() = in_r;
	}

	r_comp_type r() const { return _detail::block_element<type, r_bits, r_offset, true>(data); }

	auto r() { return _detail::block_element<type, r_bits, r_offset, false>(data); }
};

template <block_type type, unsigned d_bits, unsigned d_offset_bits = 0>
class block_depth {
public:
	static constexpr unsigned elements = 1;
	static constexpr block_type blocktype = type;

	static constexpr unsigned d_index = 0;
	static constexpr unsigned d_offset = d_offset_bits;

	static constexpr int index_for_component(gl::component_swizzle c) {
		if constexpr (c == gl::component_swizzle::d) return d_index;
		static_assert(c == gl::component_swizzle::d);
		return -1;
	}

	static constexpr unsigned total_bits = d_bits + d_offset;
	static constexpr unsigned bytes = total_bits / 8;

	using d_comp_type = typename _detail::block_primary_type_selector<type, d_bits>::type;

private:
	static_assert((total_bits % 8) == 0, "Block straddles bytes");
	static_assert(bytes > 0, "bytes is zero");

	using data_t = std::uint8_t[bytes];

	data_t data{};

public:
	block_depth() = default;
	block_depth(d_comp_type in_d) {
		d() = in_d;
	}

	d_comp_type d() const { return _detail::block_element<type, d_bits, d_offset, true>(data); }

	auto d() { return _detail::block_element<type, d_bits, d_offset, false>(data); }
};

}
}
