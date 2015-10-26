// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

#include "attrib.h"
#include "attributed_string.h"

namespace StE {
namespace Text {

template <typename CharT>
class attributed_string_htm_formatter {
public:
	attributed_string<CharT>::string_type operator()(const attributed_string<CharT> &str) {
		if (!str.length()) return attributed_string<CharT>::string_type();

		using namespace Attributes;

		attributed_string<CharT>::string_type pstr;
		attributed_string<CharT>::range_type r{ 0, str.length() };
		
		while (r.start < str.length()) {
			attributed_string<CharT>::range_type cr = r, cw = r, cs = r, ci = r;
			optional<const Attributes::rgb*>	color_attrib = str.attrib_of_type(rgb::attrib_type_s(), &cr);
			optional<const Attributes::weight*>	weight_attrib = str.attrib_of_type(weight::attrib_type_s(), &cw);
			optional<const Attributes::size*>	size_attrib = str.attrib_of_type(size::attrib_type_s(), &cs);
			optional<const Attributes::italic*>	italic_attrib = str.attrib_of_type(italic::attrib_type_s(), &ci);

			if (color_attrib && cr.start > r.start)		{ r.length = std::min(r.length, cr.start - r.start); color_attrib = none; }
			if (weight_attrib && cw.start > r.start)	{ r.length = std::min(r.length, cw.start - r.start); weight_attrib = none; }
			if (size_attrib && cs.start > r.start)		{ r.length = std::min(r.length, cs.start - r.start); size_attrib = none; }
			if (italic_attrib && ci.start > r.start)	{ r.length = std::min(r.length, ci.start - r.start); italic_attrib = none; }

			if (color_attrib)	r.length = std::min(r.length, cr.start - r.start + cr.length);
			if (weight_attrib)	r.length = std::min(r.length, cw.start - r.start + cw.length);
			if (size_attrib)	r.length = std::min(r.length, cs.start - r.start + cs.length);
			if (italic_attrib)	r.length = std::min(r.length, ci.start - r.start + ci.length);

			std::string style;
			if (color_attrib) style += "color:rgb(" + std::to_string(color_attrib->get().r) + "," + std::to_string(color_attrib->get().g) + "," + std::to_string(color_attrib->get().b) + ");";
			if (weight_attrib) { style += "font-weight:" + std::to_string(weight_attrib->get()) + ";"; }
			if (size_attrib) { style += "font-size:" + std::to_string(size_attrib->get()) + "px;"; }
			if (italic_attrib) { style += "font-style: italic;"; }

			if (style.length()) {
				std::string str_open_tag = "<span style=\"" + style + "\">";
				attributed_string<CharT>::string_type open_tag(str_open_tag.begin(), str_open_tag.end());
				std::string str_end_tag = "</span>";
				attributed_string<CharT>::string_type end_tag(str_end_tag.begin(), str_end_tag.end());
				pstr += open_tag + str.plain_string().substr(r.start, r.length) + end_tag;
			}
			else
				pstr += str.plain_string().substr(r.start, r.length);

			r.start = r.start + r.length;
			r.length = str.length() - r.start;
		}

		return pstr;
	}
};

}
}
