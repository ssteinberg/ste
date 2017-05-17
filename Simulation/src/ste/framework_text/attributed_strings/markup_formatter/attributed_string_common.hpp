// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

#include <attrib.hpp>

#include <range.hpp>
#include <optional.hpp>
#include <remove_iterator_constness.hpp>

#include <attributed_string_htm_formatter.hpp>

#include <functional>

#include <ostream>
#include <codecvt>
#include <locale>
#include <string>

#include <initializer_list>
#include <vector>
#include <algorithm>

namespace ste {
namespace text {

template <typename CharT>
class attributed_string_common {
public:
	using char_type = CharT;
	using string_type = std::basic_string<CharT, std::char_traits<CharT>, std::allocator<CharT>>;

	using range_type = range<std::size_t>;
	using attrib_type = Attributes::attrib;
	using attrib_id_type = attrib_type::attrib_id_t;
	using attrib_storage = std::vector<std::pair<range_type, std::unique_ptr<attrib_type>>>;
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

	void attrib_insert(attrib_storage::value_type &&v) {
		auto it = std::lower_bound(attributes.begin(), attributes.end(),
								   v, 
								   [](const attrib_storage::value_type &lhs, const attrib_storage::value_type &rhs) {
			return lhs.first < rhs.first;
		});
		attributes.insert(it, std::move(v));
	}

	attrib_storage_iterator attrib_remove(const attrib_storage_iterator &it) {
		return attributes.erase(it);
	}

	void splice(attrib_storage_iterator it, const range_type &r) {
		auto it_range = it->first;
		auto it_attrib = std::move(it->second);
		attrib_remove(it);

		auto length_before = static_cast<std::size_t>(std::max<std::int64_t>(r.start - it_range.start, 0));
		auto length_after = static_cast<std::size_t>(std::max<std::int64_t>(it_range.start + it_range.length - r.length - r.start, 0));

		if (length_before)
			attrib_insert(std::make_pair(range_type{ it_range.start, length_before }, std::unique_ptr<attrib_type>(it_attrib->clone())));
		if (length_after)
			attrib_insert(std::make_pair(range_type{ r.start + r.length, length_after }, std::unique_ptr<attrib_type>(it_attrib->clone())));
	}

public:
	attributed_string_common() = default;
	attributed_string_common(const char_type &c) : string(c) {}
	attributed_string_common(const char_type *cstr) : string(cstr) {}
	attributed_string_common(const string_type &str) : string(str) {}
	attributed_string_common(const string_type &str, std::initializer_list<attrib_type*> attribs) : string(str) {
		for (auto it = attribs.begin(); it != attribs.end(); ++it)
			add_attrib({ 0, str.length() }, std::unique_ptr<attrib_type>((*it)->clone()));
	}

	attributed_string_common(attributed_string_common &&) = default;
	attributed_string_common &operator=(attributed_string_common &&) = default;
	attributed_string_common(const attributed_string_common &other) : string(other.string) {
		for (auto it = other.attributes.begin(); it != other.attributes.end(); ++it)
			attrib_insert(std::make_pair(it->first, std::unique_ptr<attrib_type>(it->second->clone())));
	}
	attributed_string_common &operator=(const attributed_string_common &other) {
		string = other.string;
		for (auto it = other.attributes.begin(); it != other.attributes.end(); ++it)
			attrib_insert(std::make_pair(it->first, std::unique_ptr<attrib_type>(it->second->clone())));
		return *this;
	}

	virtual ~attributed_string_common() noexcept {}

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
		attrib_insert(std::make_pair(r, std::unique_ptr<attrib_type>(a.clone())));
	}

	void remove_all_attrib_of_type(attrib_id_type id, const range_type &r) {
		for (;;) {
			auto it = attrib_iterator_for_type(id, r);
			if (it == attributes.end()) break;
			attrib_remove(it, r);
		}
	}

	std::size_t length() const { return string.length(); }

	const string_type &plain_string() const { return string; }
	explicit operator string_type() const { return plain_string(); }

	template <class Formatter = attributed_string_htm_formatter<CharT>>
	std::string markup() const {
		auto str = Formatter()(*this);
		return std::wstring_convert<std::codecvt_utf8<CharT>, CharT>().to_bytes(str);
	}

	char_type &operator[](std::size_t index) { return string[index]; }
	const char_type &operator[](std::size_t index) const { return string[index]; }

	attributed_string_common &operator+=(const attributed_string_common &str) {
		for (auto it = str.attributes.begin(); it != str.attributes.end(); ++it)
			attrib_insert(std::make_pair(range_type{ it->first.start + this->length(), it->first.length },
										 std::unique_ptr<attrib_type>(it->second->clone())));
		string += str.string;
		return *this;
	}
	attributed_string_common &operator+=(const string_type &str) {
		string += str;
		return *this;
	}
	attributed_string_common &operator+=(const char_type *str) {
		string += str;
		return *this;
	}
	attributed_string_common &operator+=(const char_type &c) {
		string += c;
		return *this;
	}

	bool operator==(const attributed_string_common &str) const {
		return string == str.string;
	}
	bool operator!=(const attributed_string_common &str) const {
		return string != str.string;
	}
	bool operator<(const attributed_string_common &str) const {
		return string < str.string;
	}
};

template <typename CharT>
attributed_string_common<CharT> operator+(const attributed_string_common<CharT> &lhs, const attributed_string_common<CharT> &rhs) {
	attributed_string_common<CharT> str = lhs;
	str += rhs;
	return str;
}

template <typename CharT>
attributed_string_common<CharT> operator+(const attributed_string_common<CharT> &lhs, const typename attributed_string_common<CharT>::string_type &rhs) {
	attributed_string_common<CharT> str = lhs;
	str += rhs;
	return str;
}

template <typename CharT>
attributed_string_common<CharT> operator+(const attributed_string_common<CharT> &lhs, const typename attributed_string_common<CharT>::char_type &rhs) {
	attributed_string_common<CharT> str = lhs;
	str += rhs;
	return str;
}

}
}

namespace std {

template <typename CharT>
ostream& operator<<(ostream& os, const ste::text::attributed_string_common<CharT>& str) {
	os << str.template markup<>();
	return os;
}

template <typename CharT>
struct hash<ste::text::attributed_string_common<CharT>> {
	size_t inline operator()(const ste::text::attributed_string_common<CharT> &x) const {
		return hash<typename ste::text::attributed_string_common<CharT>::string_type>()(x.plain_string());
	}
};

}
