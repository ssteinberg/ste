// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <attrib_type.hpp>
#include <font.hpp>

#include <functional>
#include <typeinfo>

#include <memory>
#include <packed_ptr.hpp>

namespace StE {
namespace Text {

template <typename CharT>
class attributed_string_common;

namespace Attributes {

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
		::new (reinterpret_cast<S*>(storage)) S(std::move(s));
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
	attributed_string_common<char> operator()(const std::string &str) const;
	attributed_string_common<char16_t> operator()(const std::u16string &str) const;
	attributed_string_common<char32_t> operator()(const std::u32string &str) const;
	attributed_string_common<wchar_t> operator()(const std::wstring &str) const;
	attributed_string_common<char> operator()(std::string &&str) const;
	attributed_string_common<char16_t> operator()(std::u16string &&str) const;
	attributed_string_common<char32_t> operator()(std::u32string &&str) const;
	attributed_string_common<wchar_t> operator()(std::wstring &&str) const;
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
extern const rgb clear_color;
extern const rgb maroon;
extern const rgb dark_red;
extern const rgb brown;
extern const rgb firebrick;
extern const rgb crimson;
extern const rgb red;
extern const rgb tomato;
extern const rgb coral;
extern const rgb indian_red;
extern const rgb light_coral;
extern const rgb dark_salmon;
extern const rgb salmon;
extern const rgb light_salmon;
extern const rgb orange_red;
extern const rgb dark_orange;
extern const rgb orange;
extern const rgb gold;
extern const rgb dark_golden_rod;
extern const rgb golden_rod;
extern const rgb pale_golden_rod;
extern const rgb dark_khaki;
extern const rgb khaki;
extern const rgb olive;
extern const rgb yellow;
extern const rgb yellow_green;
extern const rgb dark_olive_green;
extern const rgb olive_drab;
extern const rgb lawn_green;
extern const rgb chart_reuse;
extern const rgb green_yellow;
extern const rgb dark_green;
extern const rgb green;
extern const rgb forest_green;
extern const rgb lime;
extern const rgb lime_green;
extern const rgb light_green;
extern const rgb pale_green;
extern const rgb dark_sea_green;
extern const rgb medium_spring_green;
extern const rgb spring_green;
extern const rgb sea_green;
extern const rgb medium_aqua_marine;
extern const rgb medium_sea_green;
extern const rgb light_sea_green;
extern const rgb dark_slate_gray;
extern const rgb teal;
extern const rgb dark_cyan;
extern const rgb aqua;
extern const rgb cyan;
extern const rgb light_cyan;
extern const rgb dark_turquoise;
extern const rgb turquoise;
extern const rgb medium_turquoise;
extern const rgb pale_turquoise;
extern const rgb aqua_marine;
extern const rgb powder_blue;
extern const rgb cadet_blue;
extern const rgb steel_blue;
extern const rgb corn_flower_blue;
extern const rgb deep_sky_blue;
extern const rgb dodger_blue;
extern const rgb light_blue;
extern const rgb sky_blue;
extern const rgb light_sky_blue;
extern const rgb midnight_blue;
extern const rgb navy;
extern const rgb dark_blue;
extern const rgb medium_blue;
extern const rgb blue;
extern const rgb royal_blue;
extern const rgb blue_violet;
extern const rgb indigo;
extern const rgb dark_slate_blue;
extern const rgb slate_blue;
extern const rgb medium_slate_blue;
extern const rgb medium_purple;
extern const rgb dark_magenta;
extern const rgb dark_violet;
extern const rgb dark_orchid;
extern const rgb medium_orchid;
extern const rgb purple;
extern const rgb thistle;
extern const rgb plum;
extern const rgb violet;
extern const rgb magenta;
extern const rgb orchid;
extern const rgb medium_violet_red;
extern const rgb pale_violet_red;
extern const rgb deep_pink;
extern const rgb hot_pink;
extern const rgb light_pink;
extern const rgb pink;
extern const rgb antique_white;
extern const rgb beige;
extern const rgb bisque;
extern const rgb blanched_almond;
extern const rgb wheat;
extern const rgb corn_silk;
extern const rgb lemon_chiffon;
extern const rgb light_golden_rod_yellow;
extern const rgb light_yellow;
extern const rgb saddle_brown;
extern const rgb sienna;
extern const rgb chocolate;
extern const rgb peru;
extern const rgb sandy_brown;
extern const rgb burly_wood;
extern const rgb tan;
extern const rgb rosy_brown;
extern const rgb moccasin;
extern const rgb navajo_white;
extern const rgb peach_puff;
extern const rgb misty_rose;
extern const rgb lavender_blush;
extern const rgb linen;
extern const rgb old_lace;
extern const rgb papaya_whip;
extern const rgb sea_shell;
extern const rgb mint_cream;
extern const rgb slate_gray;
extern const rgb light_slate_gray;
extern const rgb light_steel_blue;
extern const rgb lavender;
extern const rgb floral_white;
extern const rgb alice_blue;
extern const rgb ghost_white;
extern const rgb honeydew;
extern const rgb ivory;
extern const rgb azure;
extern const rgb snow;
extern const rgb black;
extern const rgb dim_gray;
extern const rgb gray;
extern const rgb dark_gray;
extern const rgb silver;
extern const rgb light_gray;
extern const rgb gainsboro;
extern const rgb white_smoke;
extern const rgb white;

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
	using T = ::StE::Text::font;
	using storage = std::unique_ptr<T>;

	static constexpr auto tag = attrib_type::font;

public:
	static const font* bind(const attrib *att) {
		return att && att->type() == tag ?
			reinterpret_cast<const font*>(att) :
			nullptr;
	}

	font(T &&f) : attrib(tag,
						 std::make_unique<T>(std::move(f)))
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
extern const size huge;
extern const size vvlarge;
extern const size vlarge;
extern const size large;
extern const size regular;
extern const size small;
extern const size vsmall;
extern const size tiny;

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
extern const align left;
extern const align center;
extern const align right;

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
extern const weight b;

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
extern const underline u;

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
extern const italic i;

class link : public attrib {
private:
	using T = std::string;
	using storage = std::unique_ptr<T>;

	static constexpr auto tag = attrib_type::link;

public:
	static const link* bind(const attrib *att) {
		return att && att->type() == tag ?
			reinterpret_cast<const link*>(att) :
			nullptr;
	}

	link(T &&l) : attrib(tag,
						 std::make_unique<T>(std::move(l)))
	{}

	const T& get() const { return *get_storage<storage>(); }
	operator const T&() const { return *get_storage<storage>(); }
};

}
}
}