//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_shader_program_stage.hpp>

#include <initializer_list>

#include <lib/flat_set.hpp>

namespace ste {
namespace gl {

/**
*	@brief	Defines a collection of shader stages
*/
class pipeline_binding_stages_collection {
private:
	lib::flat_set<ste_shader_program_stage> set;

public:
	pipeline_binding_stages_collection() = default;
	pipeline_binding_stages_collection(const std::initializer_list<ste_shader_program_stage> &il) : set(il) {}

	auto& get() const { return set; }
	auto begin() const { return set.begin(); }
	auto end() const { return set.end(); }
	auto size() const { return set.size(); }

	void insert(const ste_shader_program_stage &s) {
		set.insert(s);
	}
	void insert(const pipeline_binding_stages_collection &stages) {
		for (auto &s : stages)
			set.insert(s);
	}
	void erase(const ste_shader_program_stage &s) {
		set.erase(s);
	}
	bool exists(const ste_shader_program_stage &s) const {
		return set.find(s) != set.end();
	}

	operator stage_flag() const {
		stage_flag stage = stage_flag::none;
		for (auto &s : *this)
			stage = stage | ste_shader_program_stage_to_stage_flag(s);
		return stage;
	}

private:
	friend bool operator<=(const pipeline_binding_stages_collection &lhs, const pipeline_binding_stages_collection &rhs);
	friend bool operator<(const pipeline_binding_stages_collection &lhs, const pipeline_binding_stages_collection &rhs);
	friend bool operator>=(const pipeline_binding_stages_collection &lhs, const pipeline_binding_stages_collection &rhs);
	friend bool operator>(const pipeline_binding_stages_collection &lhs, const pipeline_binding_stages_collection &rhs);
	friend bool operator==(const pipeline_binding_stages_collection &lhs, const pipeline_binding_stages_collection &rhs);
	friend bool operator!=(const pipeline_binding_stages_collection &lhs, const pipeline_binding_stages_collection &rhs);

	friend pipeline_binding_stages_collection operator|(const pipeline_binding_stages_collection &lhs, const pipeline_binding_stages_collection &rhs);
	friend pipeline_binding_stages_collection operator&(const pipeline_binding_stages_collection &lhs, const pipeline_binding_stages_collection &rhs);
	friend pipeline_binding_stages_collection operator^(const pipeline_binding_stages_collection &lhs, const pipeline_binding_stages_collection &rhs);
};

/**
*	@brief	Is sub-set operator
*
*	@return	True if every shader stage in lhs exists in rhs
*/
bool inline operator<=(const pipeline_binding_stages_collection &lhs, const pipeline_binding_stages_collection &rhs) {
	auto lf = static_cast<stage_flag>(lhs);
	auto rf = static_cast<stage_flag>(rhs);
	return (lf & rf) == lf;
}
/**
*	@brief	Is strict sub-set operator
*
*	@return	True if every shader stage in lhs exists in rhs, and there exists a shader stage in rhs which doesn't exist in lhs
*/
bool inline operator<(const pipeline_binding_stages_collection &lhs, const pipeline_binding_stages_collection &rhs) {
	auto lf = static_cast<stage_flag>(lhs);
	auto rf = static_cast<stage_flag>(rhs);
	return (lf & rf) == lf && lf != rf;
}
/**
*	@brief	Is super-set operator
*
*	@return	True if every shader stage in rhs exists in lhs
*/
bool inline operator>=(const pipeline_binding_stages_collection &lhs, const pipeline_binding_stages_collection &rhs) {
	auto lf = static_cast<stage_flag>(lhs);
	auto rf = static_cast<stage_flag>(rhs);
	return (lf & rf) == rf;
}
/**
*	@brief	Is strict super-set operator
*
*	@return	True if every shader stage in rhs exists in lhs, and there exists a shader stage in lhs which doesn't exist in rhs
*/
bool inline operator>(const pipeline_binding_stages_collection &lhs, const pipeline_binding_stages_collection &rhs) {
	auto lf = static_cast<stage_flag>(lhs);
	auto rf = static_cast<stage_flag>(rhs);
	return (lf & rf) == rf && lf != rf;
}
/**
*	@brief	Are sets equal operator
*
*	@return	True if every shader stage in rhs exists in lhs and every shader stage in lhs exists in rhs
*/
bool inline operator==(const pipeline_binding_stages_collection &lhs, const pipeline_binding_stages_collection &rhs) {
	return static_cast<stage_flag>(lhs) == static_cast<stage_flag>(rhs);
}
bool inline operator!=(const pipeline_binding_stages_collection &lhs, const pipeline_binding_stages_collection &rhs) {
	return !(lhs == rhs);
}

/**
*	@brief	Sets union operator
*
*	@return	A set with all stages from lhs and rhs
*/
pipeline_binding_stages_collection inline operator|(const pipeline_binding_stages_collection &lhs, const pipeline_binding_stages_collection &rhs) {
	pipeline_binding_stages_collection ret = lhs;
	ret.set.insert(rhs.begin(), rhs.end());
	return ret;
}
/**
*	@brief	Sets intersection operator
*
*	@return	A set with the stages contained both in lhs and rhs
*/
pipeline_binding_stages_collection inline operator&(const pipeline_binding_stages_collection &lhs, const pipeline_binding_stages_collection &rhs) {
	pipeline_binding_stages_collection ret;
	for (auto &s : lhs)
		if (rhs.exists(s))
			ret.insert(s);
	return ret;
}
/**
*	@brief	Sets disjunctive union operator
*
*	@return	A set with the stages contained in either lhs or rhs, but not both
*/
pipeline_binding_stages_collection inline operator^(const pipeline_binding_stages_collection &lhs, const pipeline_binding_stages_collection &rhs) {
	pipeline_binding_stages_collection ret;
	for (auto &s : lhs)
		if (!rhs.exists(s))
			ret.insert(s);
	for (auto &s : rhs)
		if (!lhs.exists(s))
			ret.insert(s);
	return ret;
}

}
}
