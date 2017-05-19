// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <attrib_type.hpp>
#include <font.hpp>

#include <functional>
#include <typeinfo>

#include <lib/unique_ptr.hpp>
#include <packed_ptr.hpp>

namespace ste {
namespace text {

template <typename CharT>
class attributed_string_common;

namespace attributes {

class attrib {
public:
	static constexpr auto max_storage_bytes = 8;

private:
	using storage_dtor_ptr = void(*)(const void*);

private:
	std::uint8_t storage[max_storage_bytes];
	// Packed attribute type and pointer to storage desctruction function
	packed_ptr<storage_dtor_ptr> type_and_dtor;

protected:
	template <typename Storage>
	attrib(attrib_type type,
		   Storage &&s)
		: type_and_dtor(static_cast<std::uint16_t>(type), nullptr)
	{
		using S = std::remove_const_t<std::remove_reference_t<Storage>>;

		static_assert(sizeof(S) <= max_storage_bytes, "Storage size greater than max_storage_bytes");
		static_assert(std::is_move_constructible_v<S>, "Storage must be move constructible");

		// Create the storage in the preallocated space and store a pointer to the desctructor in form of a non-capturing lambda.
		lib::default_alloc<S>::ctor(reinterpret_cast<S*>(storage), std::move(s));
		type_and_dtor.set([](const void* p) { reinterpret_cast<const S*>(p)->~S(); });
	}

	template <typename Storage>
	const auto &get_storage() const { return *reinterpret_cast<const Storage*>(storage); }

public:
	~attrib() noexcept {
		type_and_dtor.get()(storage);
	}

	attrib(const attrib &) = default;
	attrib &operator=(const attrib &) = default;
	attrib(attrib &&) = default;
	attrib &operator=(attrib &&) = default;

	auto type() const noexcept { return static_cast<attrib_type>(type_and_dtor.get_integer()); }
	bool is_same_attrib(const attrib &rhs) const { return type() == rhs.type(); }

	template<typename T>
	attributed_string_common<T> operator()(attributed_string_common<T> &&str) const {
		attributed_string_common<T> newstr = std::move(str);
		newstr.add_attrib({ 0,static_cast<std::uint32_t>(newstr.length()) }, *this);
		return newstr;
	}
	template<typename T>
	attributed_string_common<T> operator()(const attributed_string_common<T> &str) const {
		return (*this)(attributed_string_common<T>(str));
	}
	attributed_string_common<char> operator()(const lib::string &str) const;
	attributed_string_common<char16_t> operator()(const lib::u16string &str) const;
	attributed_string_common<char32_t> operator()(const lib::u32string &str) const;
	attributed_string_common<wchar_t> operator()(const lib::wstring &str) const;
	attributed_string_common<char> operator()(lib::string &&str) const;
	attributed_string_common<char16_t> operator()(lib::u16string &&str) const;
	attributed_string_common<char32_t> operator()(lib::u32string &&str) const;
	attributed_string_common<wchar_t> operator()(lib::wstring &&str) const;
	attributed_string_common<char> operator()(const char* str) const;
	attributed_string_common<char16_t> operator()(const char16_t* str) const;
	attributed_string_common<char32_t> operator()(const char32_t* str) const;
	attributed_string_common<wchar_t> operator()(const wchar_t* str) const;
};

class rgb : public attrib {
private:
	using storage = glm::u8vec4;

	static constexpr auto tag = attrib_type::color;

public:
	static const rgb* bind(const attrib *att) {
		return att && att->type() == tag ?
			reinterpret_cast<const rgb*>(att) :
			nullptr;
	}

	rgb(glm::u8vec4 &&color) : attrib(tag, std::move(color)) {}
	rgb(glm::u8vec3 &&color) : rgb({ color.r, color.g, color.b, 255 }) {}

	glm::u8vec4 get() const { return get_storage<storage>(); }
	operator glm::u8vec4() const { return get_storage<storage>(); }
};
static const rgb clear_color = rgb({ 0, 0, 0, 0 });
static const rgb maroon = rgb({ 128, 0, 0 });
static const rgb dark_red = rgb({ 139, 0, 0 });
static const rgb brown = rgb({ 165, 42, 42 });
static const rgb firebrick = rgb({ 178, 34, 34 });
static const rgb crimson = rgb({ 220, 20, 60 });
static const rgb red = rgb({ 255, 0, 0 });
static const rgb tomato = rgb({ 255, 99, 71 });
static const rgb coral = rgb({ 255, 127, 80 });
static const rgb indian_red = rgb({ 205, 92, 92 });
static const rgb light_coral = rgb({ 240, 128, 128 });
static const rgb dark_salmon = rgb({ 233, 150, 122 });
static const rgb salmon = rgb({ 250, 128, 114 });
static const rgb light_salmon = rgb({ 255, 160, 122 });
static const rgb orange_red = rgb({ 255, 69, 0 });
static const rgb dark_orange = rgb({ 255, 140, 0 });
static const rgb orange = rgb({ 255, 165, 0 });
static const rgb gold = rgb({ 255, 215, 0 });
static const rgb dark_golden_rod = rgb({ 184, 134, 11 });
static const rgb golden_rod = rgb({ 218, 165, 32 });
static const rgb pale_golden_rod = rgb({ 238, 232, 170 });
static const rgb dark_khaki = rgb({ 189, 183, 107 });
static const rgb khaki = rgb({ 240, 230, 140 });
static const rgb olive = rgb({ 128, 128, 0 });
static const rgb yellow = rgb({ 255, 255, 0 });
static const rgb yellow_green = rgb({ 154, 205, 50 });
static const rgb dark_olive_green = rgb({ 85, 107, 47 });
static const rgb olive_drab = rgb({ 107, 142, 35 });
static const rgb lawn_green = rgb({ 124, 252, 0 });
static const rgb chart_reuse = rgb({ 127, 255, 0 });
static const rgb green_yellow = rgb({ 173, 255, 47 });
static const rgb dark_green = rgb({ 0, 100, 0 });
static const rgb green = rgb({ 0, 128, 0 });
static const rgb forest_green = rgb({ 34, 139, 34 });
static const rgb lime = rgb({ 0, 255, 0 });
static const rgb lime_green = rgb({ 50, 205, 50 });
static const rgb light_green = rgb({ 144, 238, 144 });
static const rgb pale_green = rgb({ 152, 251, 152 });
static const rgb dark_sea_green = rgb({ 143, 188, 143 });
static const rgb medium_spring_green = rgb({ 0, 250, 154 });
static const rgb spring_green = rgb({ 0, 255, 127 });
static const rgb sea_green = rgb({ 46, 139, 87 });
static const rgb medium_aqua_marine = rgb({ 102, 205, 170 });
static const rgb medium_sea_green = rgb({ 60, 179, 113 });
static const rgb light_sea_green = rgb({ 32, 178, 170 });
static const rgb dark_slate_gray = rgb({ 47, 79, 79 });
static const rgb teal = rgb({ 0, 128, 128 });
static const rgb dark_cyan = rgb({ 0, 139, 139 });
static const rgb aqua = rgb({ 0, 255, 255 });
static const rgb cyan = rgb({ 0, 255, 255 });
static const rgb light_cyan = rgb({ 224, 255, 255 });
static const rgb dark_turquoise = rgb({ 0, 206, 209 });
static const rgb turquoise = rgb({ 64, 224, 208 });
static const rgb medium_turquoise = rgb({ 72, 209, 204 });
static const rgb pale_turquoise = rgb({ 175, 238, 238 });
static const rgb aqua_marine = rgb({ 127, 255, 212 });
static const rgb powder_blue = rgb({ 176, 224, 230 });
static const rgb cadet_blue = rgb({ 95, 158, 160 });
static const rgb steel_blue = rgb({ 70, 130, 180 });
static const rgb corn_flower_blue = rgb({ 100, 149, 237 });
static const rgb deep_sky_blue = rgb({ 0, 191, 255 });
static const rgb dodger_blue = rgb({ 30, 144, 255 });
static const rgb light_blue = rgb({ 173, 216, 230 });
static const rgb sky_blue = rgb({ 135, 206, 235 });
static const rgb light_sky_blue = rgb({ 135, 206, 250 });
static const rgb midnight_blue = rgb({ 25, 25, 112 });
static const rgb navy = rgb({ 0, 0, 128 });
static const rgb dark_blue = rgb({ 0, 0, 139 });
static const rgb medium_blue = rgb({ 0, 0, 205 });
static const rgb blue = rgb({ 0, 0, 255 });
static const rgb royal_blue = rgb({ 65, 105, 225 });
static const rgb blue_violet = rgb({ 138, 43, 226 });
static const rgb indigo = rgb({ 75, 0, 130 });
static const rgb dark_slate_blue = rgb({ 72, 61, 139 });
static const rgb slate_blue = rgb({ 106, 90, 205 });
static const rgb medium_slate_blue = rgb({ 123, 104, 238 });
static const rgb medium_purple = rgb({ 147, 112, 219 });
static const rgb dark_magenta = rgb({ 139, 0, 139 });
static const rgb dark_violet = rgb({ 148, 0, 211 });
static const rgb dark_orchid = rgb({ 153, 50, 204 });
static const rgb medium_orchid = rgb({ 186, 85, 211 });
static const rgb purple = rgb({ 128, 0, 128 });
static const rgb thistle = rgb({ 216, 191, 216 });
static const rgb plum = rgb({ 221, 160, 221 });
static const rgb violet = rgb({ 238, 130, 238 });
static const rgb magenta = rgb({ 255, 0, 255 });
static const rgb orchid = rgb({ 218, 112, 214 });
static const rgb medium_violet_red = rgb({ 199, 21, 133 });
static const rgb pale_violet_red = rgb({ 219, 112, 147 });
static const rgb deep_pink = rgb({ 255, 20, 147 });
static const rgb hot_pink = rgb({ 255, 105, 180 });
static const rgb light_pink = rgb({ 255, 182, 193 });
static const rgb pink = rgb({ 255, 192, 203 });
static const rgb antique_white = rgb({ 250, 235, 215 });
static const rgb beige = rgb({ 245, 245, 220 });
static const rgb bisque = rgb({ 255, 228, 196 });
static const rgb blanched_almond = rgb({ 255, 235, 205 });
static const rgb wheat = rgb({ 245, 222, 179 });
static const rgb corn_silk = rgb({ 255, 248, 220 });
static const rgb lemon_chiffon = rgb({ 255, 250, 205 });
static const rgb light_golden_rod_yellow = rgb({ 250, 250, 210 });
static const rgb light_yellow = rgb({ 255, 255, 224 });
static const rgb saddle_brown = rgb({ 139, 69, 19 });
static const rgb sienna = rgb({ 160, 82, 45 });
static const rgb chocolate = rgb({ 210, 105, 30 });
static const rgb peru = rgb({ 205, 133, 63 });
static const rgb sandy_brown = rgb({ 244, 164, 96 });
static const rgb burly_wood = rgb({ 222, 184, 135 });
static const rgb tan = rgb({ 210, 180, 140 });
static const rgb rosy_brown = rgb({ 188, 143, 143 });
static const rgb moccasin = rgb({ 255, 228, 181 });
static const rgb navajo_white = rgb({ 255, 222, 173 });
static const rgb peach_puff = rgb({ 255, 218, 185 });
static const rgb misty_rose = rgb({ 255, 228, 225 });
static const rgb lavender_blush = rgb({ 255, 240, 245 });
static const rgb linen = rgb({ 250, 240, 230 });
static const rgb old_lace = rgb({ 253, 245, 230 });
static const rgb papaya_whip = rgb({ 255, 239, 213 });
static const rgb sea_shell = rgb({ 255, 245, 238 });
static const rgb mint_cream = rgb({ 245, 255, 250 });
static const rgb slate_gray = rgb({ 112, 128, 144 });
static const rgb light_slate_gray = rgb({ 119, 136, 153 });
static const rgb light_steel_blue = rgb({ 176, 196, 222 });
static const rgb lavender = rgb({ 230, 230, 250 });
static const rgb floral_white = rgb({ 255, 250, 240 });
static const rgb alice_blue = rgb({ 240, 248, 255 });
static const rgb ghost_white = rgb({ 248, 248, 255 });
static const rgb honeydew = rgb({ 240, 255, 240 });
static const rgb ivory = rgb({ 255, 255, 240 });
static const rgb azure = rgb({ 240, 255, 255 });
static const rgb snow = rgb({ 255, 250, 250 });
static const rgb black = rgb({ 0, 0, 0 });
static const rgb dim_gray = rgb({ 105, 105, 105 });
static const rgb gray = rgb({ 128, 128, 128 });
static const rgb dark_gray = rgb({ 169, 169, 169 });
static const rgb silver = rgb({ 192, 192, 192 });
static const rgb light_gray = rgb({ 211, 211, 211 });
static const rgb gainsboro = rgb({ 220, 220, 220 });
static const rgb white_smoke = rgb({ 245, 245, 245 });
static const rgb white = rgb({ 255, 255, 255 });

class stroke : public attrib {
private:
	struct storage {
		glm::u8vec4 color;
		float width;
	};

	static constexpr auto tag = attrib_type::stroke;

public:
	static const stroke* bind(const attrib *att) {
		return att && att->type() == tag ?
			reinterpret_cast<const stroke*>(att) :
			nullptr;
	}
	
	stroke(const glm::u8vec4 &color, float w) : attrib(tag, storage{ color, w }) {}

	const glm::u8vec4 &get_color() const { return get_storage<storage>().color; }
	float get_width() const { return get_storage<storage>().width; }
};

class font : public attrib {
private:
	using T = ::ste::text::font;
	using storage = lib::unique_ptr<T>;

	static constexpr auto tag = attrib_type::font;

public:
	static const font* bind(const attrib *att) {
		return att && att->type() == tag ?
			reinterpret_cast<const font*>(att) :
			nullptr;
	}

	font(T &&f) : attrib(tag,
						 lib::allocate_unique<T>(std::move(f)))
	{}

	const auto& get() const { return *get_storage<storage>(); }
	operator const T&() const { return *get_storage<storage>(); }
};

class size : public attrib {
private:
	using T = int;

	static constexpr auto tag = attrib_type::size;

public:
	static const size* bind(const attrib *att) {
		return att && att->type() == tag ?
			reinterpret_cast<const size*>(att) :
			nullptr;
	}

	size(const T &s) : attrib(tag, s) {}

	T get() const { return get_storage<T>(); }
	operator T() const { return get_storage<T>(); }
};
static const size huge = size(96);
static const size vvlarge = size(78);
static const size vlarge = size(64);
static const size large = size(48);
static const size regular = size(36);
static const size small = size(28);
static const size vsmall = size(20);
static const size tiny = size(12);

class line_height : public attrib {
private:
	using T = float;

	static constexpr auto tag = attrib_type::line_height;

public:
	static const line_height* bind(const attrib *att) {
		return att && att->type() == tag ?
			reinterpret_cast<const line_height*>(att) :
			nullptr;
	}

	line_height(const T &s) : attrib(tag, s) {}

	T get() const { return get_storage<T>(); }
	operator T() const { return get_storage<T>(); }
};

class kern : public attrib {
private:
	using T = float;

	static constexpr auto tag = attrib_type::kern;

public:
	static const kern* bind(const attrib *att) {
		return att && att->type() == tag ?
			reinterpret_cast<const kern*>(att) :
			nullptr;
	}

	kern(const T &k) : attrib(tag, k) {}

	T get() const { return get_storage<T>(); }
	operator T() const { return get_storage<T>(); }
};

class align : public attrib {
public:
	enum class alignment {
		None, Left, Center, Right,
	};

	static constexpr auto tag = attrib_type::align;

public:
	static const align* bind(const attrib *att) {
		return att && att->type() == tag ?
			reinterpret_cast<const align*>(att) :
			nullptr;
	}

	align(const alignment &a) : attrib(tag, a) {}

	alignment get() const { return get_storage<alignment>(); }
	operator alignment() const { return get_storage<alignment>(); }
};
static const align left = align(align::alignment::Left);
static const align center = align(align::alignment::Center);
static const align right = align(align::alignment::Right);

class weight : public attrib {
private:
	using T = int;

	static constexpr auto tag = attrib_type::weight;

public:
	static const weight* bind(const attrib *att) {
		return att && att->type() == tag ?
			reinterpret_cast<const weight*>(att) :
			nullptr;
	}

	weight(const T &w) : attrib(tag, w) {}

	T get() const { return get_storage<T>(); }
	operator T() const { return get_storage<T>(); }
};
static const weight b = weight(700);

class underline : public attrib {
	static constexpr auto tag = attrib_type::underline;

public:
	static const underline* bind(const attrib *att) {
		return att && att->type() == tag ?
			reinterpret_cast<const underline*>(att) :
			nullptr;
	}

	underline() : attrib(tag, .0f) {}
};
static const underline u = underline();

class italic : public attrib {
	static constexpr auto tag = attrib_type::italic;

public:
	static const italic* bind(const attrib *att) {
		return att && att->type() == tag ?
			reinterpret_cast<const italic*>(att) :
			nullptr;
	}

	italic() : attrib(tag, .0f) {}
};
static const italic i = italic();

class link : public attrib {
private:
	using T = lib::string;
	using storage = lib::unique_ptr<T>;

	static constexpr auto tag = attrib_type::link;

public:
	static const link* bind(const attrib *att) {
		return att && att->type() == tag ?
			reinterpret_cast<const link*>(att) :
			nullptr;
	}

	link(T &&l) : attrib(tag, 
						 lib::allocate_unique<T>(std::move(l)))
	{}

	const T& get() const { return *get_storage<storage>(); }
	operator const T&() const { return *get_storage<storage>(); }
};

}
}
}
