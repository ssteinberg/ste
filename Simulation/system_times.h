// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "windows.h"

namespace StE {

class system_times {
private:
	std::uint64_t old_idle{ 0 }, old_kernel{ 0 }, old_user{ 0 };

public:
	bool get_times_since_last_call(float &idle_frac, float &kernel_frac, float &user_frac) {
#ifdef _MSC_VER
		std::uint64_t idle, kernel, user;
		if (GetSystemTimes(reinterpret_cast<PFILETIME>(&idle),
						   reinterpret_cast<PFILETIME>(&kernel),
						   reinterpret_cast<PFILETIME>(&user))) {
			auto d_user = user - old_user;
			auto d_kernel = kernel - old_kernel;
			auto d_idle = idle - old_idle;
			if (!d_user)
				return false;
			float total = d_user + d_kernel + d_idle;
			idle_frac = static_cast<float>(d_idle) / total;
			kernel_frac = static_cast<float>(d_kernel) / total;
			user_frac = static_cast<float>(d_user) / total;

			old_idle = idle;
			old_kernel = kernel;
			old_user = user;

			return true;
		}

		return false;
	}
#else
#error Unsupported
#endif
};

}
