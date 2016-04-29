// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "query_object.hpp"

namespace StE {
namespace Core {

class timestamp_query : public query_object<core_resource_type::QueryObjectTimestamp> {
	using Base = query_object<core_resource_type::QueryObjectTimestamp>;

private:
	using Base::begin_query;
	using Base::end_query;
};

}
}
