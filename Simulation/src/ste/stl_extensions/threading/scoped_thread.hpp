// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <thread>
#include <stdexcept>

namespace StE {

class scoped_thread {
private:
	std::thread t;

public:
	scoped_thread(std::thread &&t) : t(std::move(t)) {
		if (!t.joinable())
			throw std::logic_error("Un-joinable thread");
	}
	~scoped_thread() {
		t.join();
	}

	scoped_thread(const scoped_thread &) = delete;
	scoped_thread &operator=(const scoped_thread &) = delete;
};

}
