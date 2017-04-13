//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_shader_stage_binding.hpp>
#include <pipeline_binding_stages_set.hpp>

#include <vk_descriptor_set_layout_binding.hpp>

#include <string>
#include <unordered_map>

namespace StE {
namespace GL {

struct pipeline_binding_set_layout_binding {
	const ste_shader_stage_binding* binding;
	pipeline_binding_stages_set stages;

	operator vk_descriptor_set_layout_binding() const {
		std::uint32_t count = 1;
		auto array_var = dynamic_cast<const ste_shader_stage_binding_variable_array*>(binding->variable.get());
		if (array_var)
			count = array_var->size();

		return vk_descriptor_set_layout_binding(*binding,
												stages,
												binding->bind_idx,
												count);
	}
};

/**
*	@brief	Describes a pipeline binding set layout
*/
class pipeline_binding_layout_set {
public:
	using bind_map_t = std::unordered_map<std::string, pipeline_binding_set_layout_binding>;

private:
	bind_map_t set;

public:
	pipeline_binding_layout_set() = default;

	auto& operator[](const std::string &name) { return set[name]; }
	template <typename... Ts>
	auto try_emplace(Ts&&... ts) {
		return set.try_emplace(std::forward<Ts>(ts)...);
	}
	void erase(const std::string &name) { set.erase(name); }

	auto find(const std::string &name) const { return set.find(name); }

	auto& get() const { return set; }
	auto begin() const { return set.begin(); }
	auto end() const { return set.end(); }

private:
	friend bool operator<=(const pipeline_binding_layout_set &lhs, const pipeline_binding_layout_set &rhs);
	friend bool operator<(const pipeline_binding_layout_set &lhs, const pipeline_binding_layout_set &rhs);
	friend bool operator>=(const pipeline_binding_layout_set &lhs, const pipeline_binding_layout_set &rhs);
	friend bool operator>(const pipeline_binding_layout_set &lhs, const pipeline_binding_layout_set &rhs);
	friend bool operator==(const pipeline_binding_layout_set &lhs, const pipeline_binding_layout_set &rhs);
};

/**
*	@brief	Is sub-set operator
*
*	@return	True if every binding in lhs exists in rhs
*/
bool inline operator<=(const pipeline_binding_layout_set &lhs, const pipeline_binding_layout_set &rhs) {
	for (auto &b : lhs) {
		auto it = rhs.get().find(b.first);
		if (it == rhs.end())
			return false;
		if (*it->second.binding->variable != *b.second.binding->variable ||
			it->second.stages <= b.second.stages)
			return false;
	}
	return true;
}
/**
*	@brief	Is strict sub-set operator
*
*	@return	True if every binding in lhs exists in rhs, and there exists a binding in rhs which doesn't exist in lhs
*/
bool inline operator<(const pipeline_binding_layout_set &lhs, const pipeline_binding_layout_set &rhs) {
	return lhs <= rhs && !(lhs >= rhs);
}
/**
*	@brief	Is super-set operator
*
*	@return	True if every binding in rhs exists in lhs
*/
bool inline operator>=(const pipeline_binding_layout_set &lhs, const pipeline_binding_layout_set &rhs) {
	for (auto &b : rhs) {
		auto it = lhs.get().find(b.first);
		if (it == lhs.end())
			return false;
		if (*it->second.binding->variable != *b.second.binding->variable ||
			it->second.stages >= b.second.stages)
			return false;
	}
	return true;
}
/**
*	@brief	Is strict super-set operator
*
*	@return	True if every binding in rhs exists in lhs, and there exists a binding in lhs which doesn't exist in rhs
*/
bool inline operator>(const pipeline_binding_layout_set &lhs, const pipeline_binding_layout_set &rhs) {
	return lhs >= rhs && !(lhs <= rhs);
}
/**
*	@brief	Are sets equal operator
*
*	@return	True if every binding in rhs exists in lhs and every binding in lhs exists in rhs
*/
bool inline operator==(const pipeline_binding_layout_set &lhs, const pipeline_binding_layout_set &rhs) {
	return lhs <= rhs && lhs >= rhs;
}
bool inline operator!=(const pipeline_binding_layout_set &lhs, const pipeline_binding_layout_set &rhs) {
	return !(lhs == rhs);
}

}
}
