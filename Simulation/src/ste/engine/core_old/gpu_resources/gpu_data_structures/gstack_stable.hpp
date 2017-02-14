// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <stdafx.hpp>
#include <gstack.hpp>

#include <forward_list>
#include <iterator>

namespace StE {
namespace Core {

template <typename T>
class gstack_stable : public gstack<T> {
	using Base = gstack<T>;

	struct free_slots_marker {
		std::size_t idx;
		std::size_t len;
	};

private:
	std::forward_list<free_slots_marker> free_slots;

public:
	using Base::Base;

	void mark_tombstone(std::size_t idx, std::size_t len = 1) {
		assert(Base::len >= idx + len && "Subscript out of range.");
		free_slots.push_front({ idx, len });
	}

	std::size_t insert(const T &t) {
		if (!free_slots.empty()) {
			auto &slot = free_slots.front();
			auto idx = slot.idx;

			if (slot.len == 1)
				free_slots.erase_after(free_slots.before_begin());
			else
				--slot.len;

			Base::overwrite(idx, t);
			return idx;
		}

		Base::push_back(t);
		return Base::size() - 1;
	}

	std::size_t insert(const T *t, std::size_t size) {
		auto prev = free_slots.before_begin();
		auto slot = std::next(prev);
		for (; slot != free_slots.end() && slot->len < size;
			 ++slot, ++prev) {}
		if (slot != free_slots.end()) {
			auto idx = slot->idx;

			if (slot->len == size)
				free_slots.erase_after(prev);
			else
				slot->len -= size;

			Base::overwrite(idx, t, size);
			return idx;
		}

		Base::push_back(t, size);
		return Base::size() - size;
	}
	std::size_t insert(const std::vector<T> &t) {
		return insert(&t[0], t.size());
	}
};

}
}
