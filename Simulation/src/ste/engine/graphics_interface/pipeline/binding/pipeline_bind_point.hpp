//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <pipeline_bind_point_base.hpp>

#include <memory>

namespace StE {
namespace GL {

class pipeline_bind_point {
private:
	std::unique_ptr<pipeline_bind_point_base> point;

public:
	pipeline_bind_point(std::unique_ptr<pipeline_bind_point_base> &&point) : point(std::move(point)) {}
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
