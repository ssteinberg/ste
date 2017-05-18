//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <pipeline_attachment_layout.hpp>

#include <lib/string.hpp>
#include <lib/flat_map.hpp>
#include <initializer_list>

namespace ste {
namespace gl {

/**
*	@brief	A collection of pipeline attachment layouts
*/
class pipeline_attachment_layout_collection {
public:
	using bind_map_t = lib::flat_map<lib::string, pipeline_attachment_layout>;

private:
	bind_map_t set;

public:
	pipeline_attachment_layout_collection() = default;
	pipeline_attachment_layout_collection(const std::initializer_list<bind_map_t::value_type> &il) : set(il) {}

	template <typename... Ts>
	auto try_emplace(Ts&&... ts) {
		return set.try_emplace(std::forward<Ts>(ts)...);
	}
	void erase(const lib::string &name) { set.erase(name); }
	void erase(bind_map_t::iterator it) { set.erase(it); }

	auto find(const lib::string &name) const { return set.find(name); }

	auto& get() const { return set; }
	auto begin() const { return set.begin(); }
	auto end() const { return set.end(); }
	auto begin() { return set.begin(); }
	auto end() { return set.end(); }
};

}
}
