// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

#include "attrib.h"

#include "range.h"
#include "optional.h"
#include "remove_iterator_constness.h"

#include "attributed_string_htm_formatter.h"

#include <functional>

#include <ostream>
#include <codecvt>
#include <locale>
#include <string>

#include <initializer_list>
#include <map>

namespace StE {
namespace Text {

template <typename CharT>
class attributed_string {
public:
	using char_type = CharT;
	using string_type = std::basic_string<CharT, std::char_traits<CharT>, std::allocator<CharT>>;

	using range_type = range<std::size_t>;
	using attrib_type = Attributes::attrib;
	using attrib_id_type = attrib_type::attrib_id_t;
	using attrib_storage = std::multimap<range_type, std::unique_ptr<attrib_type>>;
	using attrib_storage_iterator = attrib_storage::iterator;

private:
	attrib_storage attributes;
	string_type string;

	auto attrib_iterator_for_type(attrib_id_type id, const range_type &r) const {
		for (auto it = attributes.begin(); it != attributes.end(); ++it) {
			if (r.start + r.length < it->first.start)
				break;
			if (it->second->attrib_type() == id && it->first.overlaps(r))
				return it;
		}

		return attributes.end();
	}

	attrib_storage_iterator remove_attrib(const attrib_storage_iterator &it) { return attributes.erase(it); }

	void splice(attrib_storage_iterator it, const range_type &r) {
		auto it_range = it->first;
		auto it_attrib = std::move(it->second);
		remove_attrib(it);

		auto length_before = std::max<int>(r.start - it_range.start, 0);
		auto length_after = std::max<int>(it_range.start + it_range.length - r.length - r.start, 0);

		if (length_before)
			attributes.insert(std::make_pair(range_type{ it_range.start, length_before }, std::unique_ptr<attrib_type>(it_attrib->clone())));
		if (length_after)
			attributes.insert(std::make_pair(range_type{ r.start + r.length, length_after }, std::unique_ptr<attrib_type>(it_attrib->clone())));
	}

public:
	attributed_string() = default;
	attributed_string(const char_type &c) : string(c) {}
	attributed_string(const char_type *cstr) : string(cstr) {}
	attributed_string(const string_type &str) : string(str) {}
	attributed_string(const string_type &str, std::initializer_list<attrib_type*> attribs) : string(str) {
		for (auto &a : attribs)
			add_attrib({ 0, str.length() }, std::unique_ptr<attrib_type>(a->clone()));
	}

	attributed_string(attributed_string &&) = default;
	attributed_string &operator=(attributed_string &&) = default;
	attributed_string(const attributed_string &other) : string(other.string) {
		for (auto &pair : other.attributes)
			attributes.insert(std::make_pair(pair.first, std::unique_ptr<attrib_type>(pair.second->clone())));
	}
	attributed_string &operator=(const attributed_string &other) {
		string = other.string;
		for (auto &pair : other.attributes)
			attributes.insert(std::make_pair(pair.first, std::unique_ptr<attrib_type>(pair.second->clone())));
		return *this;
	}

	optional<attrib_type*> attrib_of_type(attrib_id_type id, range_type *r) {
		auto it = attrib_iterator_for_type(id, *r);
		if (it == attributes.end()) return none;
		*r = it->first;
		return it->second.get();
	}
	optional<attrib_type*> attrib_of_type(attrib_id_type id, const range_type &r) {
		range_type t = r;
		return attrib_of_type(id, &t);
	}

	optional<const attrib_type*> attrib_of_type(attrib_id_type id, range_type *r) const {
		auto it = attrib_iterator_for_type(id, *r);
		if (it == attributes.end()) return none;
		*r = it->first;
		return it->second.get();
	}
	optional<const attrib_type*> attrib_of_type(attrib_id_type id, const range_type &r) const {
		range_type t = r;
		return attrib_of_type(id, &t);
	}

	void add_attrib(const range_type &r, const attrib_type &a) {
		for (;;) {
			auto it = attrib_iterator_for_type(a.attrib_type(), r);
			if (it == attributes.end()) break;
			splice(remove_iterator_constness(attributes, it), r);
		}
		attributes.insert(std::make_pair(r, std::unique_ptr<attrib_type>(a.clone())));
	}

	void remove_all_attrib_of_type(attrib_id_type id, const range_type &r) {
		for (;;) {
			auto it = attrib_iterator_for_type(id, r);
			if (it == attributes.end()) break;
			remove_attrib(it, r);
		}
	}

	std::size_t length() const { return string.length(); }

	const string_type &plain_string() const { return string; }
	explicit operator string_type() const { return plain_string(); }

	template <class Formatter = attributed_string_htm_formatter<CharT>>
	std::string markup() const {
		auto str = Formatter()(*this);
		// Undefined reference error to 'std::codecvt_utf8' on ICC
		return std::string(str.begin(), str.end());//std::wstring_convert<std::codecvt_utf8<CharT>, CharT>().to_bytes(str);
	}

	char_type &operator[](std::size_t index) { return string[index]; }
	const char_type &operator[](std::size_t index) const { return string[index]; }

	attributed_string &operator+=(const attributed_string &str) {
		for (auto &pair : str.attributes)
			attributes.insert(std::make_pair(range_type{ pair.first.start + this->length(), pair.first.length }, std::unique_ptr<attrib_type>(pair.second->clone())));
		string += str.string;
		return *this;
	}
	attributed_string &operator+=(const string_type &str) {
		string += str;
		return *this;
	}
	attributed_string &operator+=(const char_type &c) {
		string += c;
		return *this;
	}

	bool operator==(const attributed_string &str) const {
		return string == str.string;
	}
	bool operator!=(const attributed_string &str) const {
		return string != str.string;
	}
	bool operator<(const attributed_string &str) const {
		return string < str.string;
	}
};

template <typename CharT>
attributed_string<CharT> operator+(const attributed_string<CharT> &lhs, const attributed_string<CharT> &rhs) {
	attributed_string<CharT> str = lhs;
	str += rhs;
	return str;
}

template <typename CharT>
attributed_string<CharT> operator+(const attributed_string<CharT> &lhs, const typename attributed_string<CharT>::string_type &rhs) {
	attributed_string<CharT> str = lhs;
	str += rhs;
	return str;
}

template <typename CharT>
attributed_string<CharT> operator+(const attributed_string<CharT> &lhs, const typename attributed_string<CharT>::char_type &rhs) {
	attributed_string<CharT> str = lhs;
	str += rhs;
	return str;
}

}
}

namespace std {

template <typename CharT>
ostream& operator<<(ostream& os, const StE::Text::attributed_string<CharT>& str) {
	os << str.template markup<>();
	return os;
}

template <typename CharT>
struct hash<StE::Text::attributed_string<CharT>> {
	size_t inline operator()(const StE::Text::attributed_string<CharT> &x) const {
		return hash<StE::Text::attributed_string<CharT>::string_type>()(x.plain_string());
	}
};

}
