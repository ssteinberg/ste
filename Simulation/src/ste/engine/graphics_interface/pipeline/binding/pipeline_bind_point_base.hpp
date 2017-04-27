//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_shader_stage_variable.hpp>

#include <type_traits>

namespace ste {
namespace gl {

namespace _internal {

template <bool assignable>
struct pipeline_bind_point_dispatcher_dispatch {
	template <typename BindPoint, typename V>
	static void dispatch(BindPoint *bind, V&& v) {
		(*bind) = std::forward<V>(v);
	}
};
template <>
struct pipeline_bind_point_dispatcher_dispatch<false> {
	template <typename BindPoint, typename V>
	static void dispatch(BindPoint*, V&&) {
		BindPoint::template errorneous_assign<V>();
	}
};

template <typename Src, typename Target0, typename... Targets>
struct pipeline_bind_point_dispatcher {
	template <typename T, typename V>
	static void assign(Src *src, V&& v) {
		if (auto *ptr = dynamic_cast<Target0*>(src)) {
			static constexpr bool assignable = std::is_assignable_v<Target0, V>;
			pipeline_bind_point_dispatcher_dispatch<assignable>::dispatch(ptr,
																		  std::forward<V>(v));
			return;
		}
		pipeline_bind_point_dispatcher<Src, Targets...>::template assign<T>(src,
																			std::forward<V>(v));
	}
};
template <typename Src, typename Target0>
struct pipeline_bind_point_dispatcher<Src, Target0> {
	template <typename T, typename V>
	static void assign(Src *src, V&& v) {
		if (auto *ptr = dynamic_cast<Target0*>(src)) {
			static constexpr bool assignable = std::is_assignable_v<Target0, V>;
			pipeline_bind_point_dispatcher_dispatch<assignable>::dispatch(ptr,
																		  std::forward<V>(v));
			return;
		}

		assert(false && "No target found");
	}
};

}

class pipeline_specialization_constant_bind_point;
class pipeline_external_resource_bind_point;
class pipeline_push_constant_bind_point;
class pipeline_resource_bind_point;

class pipeline_bind_point_base {
private:
	using dispatcher = _internal::pipeline_bind_point_dispatcher<
		pipeline_bind_point_base,
		pipeline_push_constant_bind_point,
		pipeline_resource_bind_point,
		pipeline_specialization_constant_bind_point
	>;

private:
	const ste_shader_stage_variable *variable;

public:
	pipeline_bind_point_base(const ste_shader_stage_variable *variable) : variable(variable) {}
	virtual ~pipeline_bind_point_base() noexcept {}

	pipeline_bind_point_base(pipeline_bind_point_base&&) = default;
	pipeline_bind_point_base &operator=(pipeline_bind_point_base&&) = default;

	/**
	*	@brief	Virtual binding point
	*/
	template <typename T>
	void operator=(T&& t) {
		dispatcher::assign<T>(this,
							  std::forward<T>(t));
	}

	auto& get_var() const { return *variable; }
	auto* operator->() const { return variable; }
};

}
}
