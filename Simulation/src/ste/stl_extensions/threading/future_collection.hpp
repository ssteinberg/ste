// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <vector>
#include <future>
#include <functional>

namespace StE {

template <typename R>
class future_collection {
public:
	using future_type = std::future<R>;

private:
	std::vector<future_type> futures;

public:
	void insert(future_type &&f) {
		futures.push_back(std::move(f));
	}

	int wait_for_any() const {
		while (size() > 0) {
			for (auto it = futures.begin(); it != futures.end(); ++it)
				if (it->wait_for(std::chrono::seconds(0)) == std::future_status::ready)
					return static_cast<int>(it - futures.begin());
		}
		return -1;
	}

	void wait() const {
		for (auto it = futures.begin(); it != futures.end(); ++it)
			it->wait();
	}

	bool ready_all() const {
		for (auto it = futures.begin(); it != futures.end(); ++it)
			if (it->wait_for(std::chrono::seconds(0)) != std::future_status::ready)
				return false;
		return true;
	}

	int ready_any() const {
		for (auto it = futures.begin(); it != futures.end(); ++it)
			if (it->wait_for(std::chrono::seconds(0)) == std::future_status::ready)
				return static_cast<int>(it - futures.begin());
		return -1;
	}

	std::vector<R> get() {
		std::vector<R> v;
		for (auto it = futures.begin(); it != futures.end(); ++it)
			v = it->get();
		futures.clear();
		return v;
	}

	R get(int index) {
		R r = futures[index].get();
		futures.erase(futures.begin() + index);
		return r;
	}

	auto size() const { return futures.size(); }
};

}
