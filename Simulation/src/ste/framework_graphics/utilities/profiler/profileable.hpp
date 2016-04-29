// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "timestamp_query.hpp"

namespace StE {
namespace Graphics {

class profileable {
private:
	mutable int timer_in_use{ -1 };
	mutable Core::timestamp_query start_time_query[2];
	mutable Core::timestamp_query end_time_query[2];
	mutable std::uint64_t start_time{ 0 };
	mutable std::uint64_t end_time{ 0 };

public:
	void query_start() const {
		if (timer_in_use != -1) {
			start_time = start_time_query[timer_in_use].get_query_result_u64(Core::QueryResultType::QueryResult);
			end_time = end_time_query[timer_in_use].get_query_result_u64(Core::QueryResultType::QueryResult);
		}

		timer_in_use = (timer_in_use+1) % 2;

		start_time_query[timer_in_use].query_counter();
	}

	void query_end() const {
		end_time_query[timer_in_use].query_counter();
	}

	auto get_start_time() const { return start_time; }
	auto get_end_time() const { return end_time; }
};

}
}
