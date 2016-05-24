// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "gstack.hpp"

#include <set>

namespace StE {
namespace Core {

template <typename T>
class gstack_stable : public gstack<T> {
	using Base = gstack<T>;

private:
	std::set<std::size_t> free_slots;

public:
	using Base::Base;

	void mark_tombstone(std::size_t n) {
		assert(Base::len > n && "Subscript out of range.");
		free_slots.insert(n);
	}

	std::size_t insert(const T &t) {
		if (free_slots.size()) {
			auto slot = free_slots.begin();
			free_slots.erase(slot);

			Base::overwrite(*slot, t);
			return *slot;
		}

		Base::push_back(t);
		return Base::size() - 1;
	}
};

}
}
