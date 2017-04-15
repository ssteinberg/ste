//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_shader_stage.hpp>

#include <boost/container/flat_set.hpp>

namespace StE {
namespace GL {

/**
*	@brief	Defines a collection of shader stages
*/
class pipeline_binding_stages_collection {
private:
	boost::container::flat_set<ste_shader_stage> set;

public:
	pipeline_binding_stages_collection() = default;

	void insert(const ste_shader_stage &s) {
		set.insert(s);
	}
	void erase(const ste_shader_stage &s) {
		set.erase(s);
	}
	bool exists(const ste_shader_stage &s) const {
		return set.find(s) != set.end();
	}

	auto& get() const { return set; }
	auto begin() const { return set.begin(); }
	auto end() const { return set.end(); }

	operator VkShaderStageFlags() const {
		VkShaderStageFlags stage = 0;
		for (auto &s : *this)
			stage |= ste_shader_stage_to_vk_stage(s);
		return stage;
	}

private:
	friend bool operator<=(const pipeline_binding_stages_collection &lhs, const pipeline_binding_stages_collection &rhs);
	friend bool operator<(const pipeline_binding_stages_collection &lhs, const pipeline_binding_stages_collection &rhs);
	friend bool operator>=(const pipeline_binding_stages_collection &lhs, const pipeline_binding_stages_collection &rhs);
	friend bool operator>(const pipeline_binding_stages_collection &lhs, const pipeline_binding_stages_collection &rhs);
	friend bool operator==(const pipeline_binding_stages_collection &lhs, const pipeline_binding_stages_collection &rhs);

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
	for (auto &b : lhs) {
		if (!rhs.exists(b))
			return false;
	}
	return true;
}
/**
*	@brief	Is strict sub-set operator
*
*	@return	True if every shader stage in lhs exists in rhs, and there exists a shader stage in rhs which doesn't exist in lhs
*/
bool inline operator<(const pipeline_binding_stages_collection &lhs, const pipeline_binding_stages_collection &rhs) {
	return lhs <= rhs && !(lhs >= rhs);
}
/**
*	@brief	Is super-set operator
*
*	@return	True if every shader stage in rhs exists in lhs
*/
bool inline operator>=(const pipeline_binding_stages_collection &lhs, const pipeline_binding_stages_collection &rhs) {
	for (auto &b : rhs) {
		if (!lhs.exists(b))
			return false;
	}
	return true;
}
/**
*	@brief	Is strict super-set operator
*
*	@return	True if every shader stage in rhs exists in lhs, and there exists a shader stage in lhs which doesn't exist in rhs
*/
bool inline operator>(const pipeline_binding_stages_collection &lhs, const pipeline_binding_stages_collection &rhs) {
	return lhs >= rhs && !(lhs <= rhs);
}
/**
*	@brief	Are sets equal operator
*
*	@return	True if every shader stage in rhs exists in lhs and every shader stage in lhs exists in rhs
*/
bool inline operator==(const pipeline_binding_stages_collection &lhs, const pipeline_binding_stages_collection &rhs) {
	return lhs <= rhs && lhs >= rhs;
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
