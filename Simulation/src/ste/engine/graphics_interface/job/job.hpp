//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

namespace ste {
namespace gl {

template <typename Future>
class job {
private:
	Future future;

public:
	template <typename F>
	job(F&& future) : future(std::forward<F>(future)) {}
	~job() noexcept {
		finish();
	}

	job(job&&) = default;
	job &operator=(job&&) = default;

	/**
	 *	@brief	Waits for the job to complete.
	 */
	void finish() const { future.wait(); }
};

template <typename Future>
auto make_job(Future&& future) {
	return job<Future>(std::forward<Future>(future));
}

}
}
