//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <pipeline_bind_point_base.hpp>

#include <lib/unique_ptr.hpp>

namespace ste {
namespace gl {

class pipeline_bind_point {
private:
	lib::unique_ptr<pipeline_bind_point_base> point;

public:
	pipeline_bind_point(lib::unique_ptr<pipeline_bind_point_base> &&point) : point(std::move(point)) {}
	virtual ~pipeline_bind_point() noexcept {}

	pipeline_bind_point(pipeline_bind_point&&) = default;
	pipeline_bind_point &operator=(pipeline_bind_point&&) = default;

	/**
	*	@brief	Virtual binding point
	*/
	template <typename T>
	void operator=(T&& t) {
		*point = std::forward<T>(t);
	}

	decltype(auto) get_var() const { return point->get_var(); }
	decltype(auto) operator->() const { return point->operator->(); }
};

}
}
