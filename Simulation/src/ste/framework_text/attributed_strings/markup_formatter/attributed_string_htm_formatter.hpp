// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

#include <attrib.hpp>
#include <attributed_string_common.hpp>

namespace ste {
namespace text {

template <typename CharT>
class attributed_string_htm_formatter {
public:
	typename attributed_string_common<CharT>::string_type operator()(const attributed_string_common<CharT> &str) {
		if (!str.length()) return typename attributed_string_common<CharT>::string_type();

		using namespace attributes;

		typename attributed_string_common<CharT>::string_type pstr;
		typename attributed_string_common<CharT>::range_type r{ 0, static_cast<std::uint32_t>(str.length()) };

		while (r.start < str.length()) {
			typename attributed_string_common<CharT>::range_type cr = r, cw = r, cs = r, ci = r;
			auto color_attrib =		attributes::rgb::bind(str.attrib_of_type(attributes::attrib_type::color, &cr));
			auto weight_attrib =	attributes::weight::bind(str.attrib_of_type(attributes::attrib_type::weight, &cw));
			auto size_attrib =		attributes::size::bind(str.attrib_of_type(attributes::attrib_type::size, &cs));
			auto italic_attrib =	attributes::italic::bind(str.attrib_of_type(attributes::attrib_type::italic, &ci));

			if (color_attrib && cr.start > r.start)		{ r.length = std::min(r.length, cr.start - r.start); color_attrib = nullptr; }
			if (weight_attrib && cw.start > r.start)	{ r.length = std::min(r.length, cw.start - r.start); weight_attrib = nullptr; }
			if (size_attrib && cs.start > r.start)		{ r.length = std::min(r.length, cs.start - r.start); size_attrib = nullptr; }
			if (italic_attrib && ci.start > r.start)	{ r.length = std::min(r.length, ci.start - r.start); italic_attrib = nullptr; }

			if (color_attrib)	r.length = std::min(r.length, cr.start - r.start + cr.length);
			if (weight_attrib)	r.length = std::min(r.length, cw.start - r.start + cw.length);
			if (size_attrib)	r.length = std::min(r.length, cs.start - r.start + cs.length);
			if (italic_attrib)	r.length = std::min(r.length, ci.start - r.start + ci.length);

			lib::string style;
			if (color_attrib) 
				style += "color:rgb(" + lib::to_string(color_attrib->get().r) + "," + lib::to_string(color_attrib->get().g) + "," + lib::to_string(color_attrib->get().b) + ");";
			if (weight_attrib) 
				style += "font-weight:" + lib::to_string(weight_attrib->get()) + ";";
			if (size_attrib) 
				style += "font-size:" + lib::to_string(size_attrib->get()) + "px;";
			if (italic_attrib) 
				style += "font-style: italic;";

			if (style.length()) {
				lib::string str_open_tag = "<span style=\"" + style + "\">";
				typename attributed_string_common<CharT>::string_type open_tag(str_open_tag.begin(), str_open_tag.end());
				lib::string str_end_tag = "</span>";
				typename attributed_string_common<CharT>::string_type end_tag(str_end_tag.begin(), str_end_tag.end());
				pstr += open_tag + str.plain_string().substr(r.start, r.length) + end_tag;
			}
			else
				pstr += str.plain_string().substr(r.start, r.length);

			r.start = r.start + r.length;
			r.length = static_cast<std::uint32_t>(str.length()) - r.start;
		}

		return pstr;
	}
};

}
}
