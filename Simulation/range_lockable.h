// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

#include "gpu_sync.h"
#include "range.h"

#include <map>

#include <memory>

namespace StE {
namespace LLR {

class range_lockable {
private:
	using lock_map_type = std::multimap<range<>, gpu_sync>;
	mutable std::shared_ptr<lock_map_type> locks{ new lock_map_type };

public:
	void lock_range(const range<> &r) const {
		gpu_sync l;
		l.lock();
		locks->insert(std::make_pair(r, std::move(l)));
	}

	void wait_for_range(const range<> &r) const {
		for (auto it = locks->begin(); it != locks->end();) {
			if (it->first.start > r.start + r.length)
				break;
			if (it->first.overlaps(r)) {
				it->second.wait();
				it = locks->erase(it);
			}
			else
				++it;
		}
	}

	void client_wait_for_range(const range<> &r) const {
		for (auto it = locks->begin(); it != locks->end();) {
			if (it->first.start > r.start + r.length)
				break;
			if (it->first.overlaps(r)) {
				it->second.client_wait();
				it = locks->erase(it);
			}
			else
				++it;
		}
	}
};

}
}
