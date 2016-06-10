// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "range.hpp"

#include <vector>

namespace StE {
namespace Core {

template <bool sparse>
class sparse_buffer_commitment_data {
public:
	bool commit(const range<> &r) {
		assert(false);
		return false;
	}
	bool uncommit(const range<> &r) {
		assert(false);
		return false;
	}
	void clear() {}
};

template <>
class sparse_buffer_commitment_data<true> {
private:
	std::vector<bool> commited_pages;

public:
	bool commit(const range<> &r) {
		assert(r.length);

		bool changed = false;
		for (auto i = r.start; i < r.start + r.length && i < commited_pages.size(); ++i) {
			if (!commited_pages[i]) {
				changed = true;
				commited_pages[i] = true;
			}
		}

		if (commited_pages.size() < r.start + r.length) {
			if (commited_pages.size() < r.start)
				commited_pages.resize(r.start, false);
			commited_pages.resize(r.start + r.length, true);
			return true;
		}

		return changed;
	}

	bool uncommit(const range<> &r) {
		assert(r.length);

		bool changed = false;
		for (auto i = r.start; i < r.start + r.length && i < commited_pages.size(); ++i) {
			if (commited_pages[i]) {
				changed = true;
				commited_pages[i] = false;
			}
		}

		return changed;
	}

	void clear() {
		commited_pages.clear();
	}
};

}
}
