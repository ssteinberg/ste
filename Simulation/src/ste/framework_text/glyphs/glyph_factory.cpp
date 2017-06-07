
#include "stdafx.hpp"
#include "glyph_factory.hpp"

#include "make_distance_map.hpp"

#include "thread_constants.hpp"
#include "hash_combine.hpp"

#include <unordered_map>
#include <functional>

#include <mutex>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <stdexcept>

using namespace StE;
using namespace StE::Text;

namespace StE {
namespace Text {

struct text_glyph_pair_key {
	wchar_t left;
	wchar_t right;
	int pixel_size;

	bool operator==(const text_glyph_pair_key &rhs) const {
		return left == rhs.left && right == rhs.right && pixel_size == rhs.pixel_size;
	}
};

}
}


namespace std {

template <> struct hash<StE::Text::text_glyph_pair_key> {
	size_t inline operator()(const StE::Text::text_glyph_pair_key &x) const {
		auto h1 = std::hash<decltype(x.right)>()(x.right);
		auto h2 = std::hash<decltype(x.left)>()(x.left);
		auto h3 = std::hash<decltype(x.pixel_size)>()(x.pixel_size);
		return StE::hash_combine(h1, StE::hash_combine(h2, h3));
	}
};

}


namespace StE {
namespace Text {

class glyph_factory_ft_lib {
private:
	FT_Library library;

public:
	glyph_factory_ft_lib() {
		auto error = FT_Init_FreeType(&library);
		if (error) {
			throw std::runtime_error("Couldn't init FreeType");
		}
	}
	~glyph_factory_ft_lib() {
		FT_Done_FreeType(library);
	}
	glyph_factory_ft_lib(glyph_factory_ft_lib &&) = default;
	glyph_factory_ft_lib &operator=(glyph_factory_ft_lib &&) = default;

	FT_Library get_lib() { return library; }
	FT_Library get_lib() const { return library; }
};

class glyph_factory_font {
private:
	FT_Face face{ nullptr };
	std::unordered_map<text_glyph_pair_key, int> spacing_cache;

public:
	glyph_factory_font(const font &font, FT_Library ft_lib) {
		auto error = FT_New_Face(ft_lib,
								 font.get_path().string().c_str(),
								 0,
								 &face);
		if (error) {
			face = nullptr;
			throw std::runtime_error("Failed loading FreeType font");
		}

		FT_Select_Charmap(face, ft_encoding_unicode);
	}
	~glyph_factory_font() {
		if (face)
			FT_Done_Face(face);
	}
	glyph_factory_font(glyph_factory_font &&f) noexcept : face(f.face), spacing_cache(std::move(f.spacing_cache)) {
		f.face = nullptr;
	}
	glyph_factory_font &operator=(glyph_factory_font &&f) noexcept {
		face = f.face;
		spacing_cache = std::move(f.spacing_cache);
		f.face = nullptr;

		return *this;
	}

	FT_Face get_face() { return face; }
	FT_Face get_face() const { return face; }

	bool get_spacing(wchar_t left, wchar_t right, int pixel_size, int *spacing) const {
		auto it = spacing_cache.find(text_glyph_pair_key{ left, right, pixel_size });
		if (it != spacing_cache.end()) {
			*spacing = it->second;
			return true;
		}
		return false;
	}
	void insert_into_spacing_cache(wchar_t left, wchar_t right, int pixel_size, int spacing) {
		spacing_cache[text_glyph_pair_key{ left, right, pixel_size }] = spacing;
	}
};

struct glyph_factory_impl {
	std::mutex m;
	glyph_factory_ft_lib lib;
	std::unordered_map<font, glyph_factory_font> fonts;

	auto& get_factory_font(const font &font) {
		auto it = fonts.find(font);
		if (it == fonts.end())
			it = fonts.emplace(std::make_pair(font, glyph_factory_font(font, lib.get_lib()))).first;
		return it->second;
	}

	unsigned char *render_glyph_with(const font&, wchar_t, int, int&, int&, int&, int&);
};

}
}


unsigned char* glyph_factory_impl::render_glyph_with(const font &font, wchar_t codepoint, int px_size, int &w, int &h, int &start_y, int &start_x) {
	std::unique_lock<std::mutex> l(m);

	auto face = get_factory_font(font).get_face();

	FT_UInt glyph_index = FT_Get_Char_Index(face, codepoint);
	FT_Set_Pixel_Sizes(face, 0, px_size);
	FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
	FT_Render_Glyph(face->glyph, FT_RENDER_MODE_LIGHT);

	auto bm = face->glyph->bitmap;
	auto metrics = face->glyph->metrics;

	start_x = (metrics.horiBearingX >> 6) - glyph::padding;
	start_y = ((metrics.horiBearingY - metrics.height) >> 6) - glyph::padding;
	w = bm.width + 2 * glyph::padding;
	h = bm.rows + 2 * glyph::padding;

	int padding_w = std::max<int>(0, (w - bm.width) >> 1);
	int padding_h = std::max<int>(0, (h - bm.rows) >> 1);

	unsigned char *glyph_buf = new unsigned char[w*h];
	memset(glyph_buf, 0, w * h);
	for (unsigned y = 0; y < bm.rows; ++y)
		memcpy(&glyph_buf[padding_w + (y + padding_h) * w], &reinterpret_cast<char*>(bm.buffer)[(bm.rows - y - 1) * bm.pitch], std::min<int>(w, bm.pitch));

	return glyph_buf;
}

glyph_factory::glyph_factory() : pimpl(std::make_unique<glyph_factory_impl>()) {}

glyph_factory::~glyph_factory() {}

StE::task_future<glyph> glyph_factory::create_glyph_async(task_scheduler *sched, const font &font, wchar_t codepoint) {
	return sched->schedule_now([=]() -> glyph {
		glyph g;
		int start_x, start_y, w, h;
		int px_size = glyph::ttf_pixel_size;

		unsigned char *glyph_buf = pimpl->render_glyph_with(font, codepoint, px_size, w, h, start_y, start_x);
		g.metrics.start_x = start_x;
		g.metrics.start_y = start_y;
		g.metrics.width = w;
		g.metrics.height = h;

		g.glyph_distance_field = std::make_unique<gli::texture2d>(gli::format::FORMAT_R32_SFLOAT_PACK32, glm::ivec2{ w, h }, 1);
		make_distance_map(glyph_buf, w, h, reinterpret_cast<float*>(g.glyph_distance_field->data()));
		delete[] glyph_buf;

		return std::move(g);
	});
}

int glyph_factory::read_kerning(const font &font, const std::pair<wchar_t, wchar_t> &p, int pixel_size) {
	glyph_factory_font &fac_font = pimpl->get_factory_font(font);
	int spacing;
	if (fac_font.get_spacing(p.first, p.second, pixel_size, &spacing))
		return spacing;

	std::unique_lock<std::mutex> l(pimpl->m);

	auto face = fac_font.get_face();

	FT_UInt left_index = FT_Get_Char_Index(face, p.first);
	FT_UInt right_index = FT_Get_Char_Index(face, p.second);
	FT_Set_Pixel_Sizes(face, 0, pixel_size);
	FT_Load_Glyph(face, left_index, FT_LOAD_DEFAULT);

	bool has_kern = FT_HAS_KERNING(face);

	if (has_kern) {
		FT_Vector delta;
		FT_Get_Kerning(face, left_index, right_index, FT_KERNING_DEFAULT, &delta);

		spacing = (face->glyph->advance.x + delta.x) >> 6;
	}
	else {
		spacing = face->glyph->advance.x >> 6;
	}

	fac_font.insert_into_spacing_cache(p.first, p.second, pixel_size, spacing);

	return spacing;
}
