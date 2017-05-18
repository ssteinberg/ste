//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <lib/string.hpp>
#include <lib::deque>
#include <split.hpp>

namespace ste {
namespace gl {

class push_constant_path {
private:
	lib::deque<lib::string> path;

public:
	push_constant_path(const lib::string &s)
		: path(split<lib::deque, std::front_insert_iterator>(s,
															 '.',
															 false))
	{}

	push_constant_path(push_constant_path&&) = default;
	push_constant_path &operator=(push_constant_path&&) = default;

	/**
	 *	@brief	Attempts to step a level into the path. Consumes the level and returns it.
	 *			Throws if path is empty
	 */
	lib::string step_in() {
		auto ret = path.back();
		path.pop_back();
		return ret;
	}

	bool empty() const { return path.empty(); }
};

}
}
