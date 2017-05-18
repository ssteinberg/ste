//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <push_constant_path.hpp>

#include <lib/string.hpp>
#include <lib/flat_map.hpp>
#include <optional.hpp>
#include <lib::deque>

namespace ste {
namespace gl {

class push_constant_descriptor {
private:
	using children_descriptors_map_t = lib::flat_map<lib::string, push_constant_descriptor>;

	friend class pipeline_push_constants_layout;

private:
	const ste_shader_stage_variable *variable{ nullptr };

	// Total offset of variable in parent push constant layout
	std::uint32_t total_offset{ 0 };
	children_descriptors_map_t children;

private:
	auto& emplace_child(const ste_shader_stage_variable *element,
						std::uint32_t parent_offset) {
		auto offset = parent_offset + element->offset();
		auto pair = children.emplace(element->name(), push_constant_descriptor(element,
																			   offset));

		return pair.first->second;
	}

	push_constant_descriptor() = default;
	push_constant_descriptor(const ste_shader_stage_variable *variable,
							 std::uint32_t parent_offset)
		: variable(variable),
		total_offset(parent_offset + variable->offset())
	{}

public:
	~push_constant_descriptor() noexcept {}

	push_constant_descriptor(push_constant_descriptor&&) = default;
	push_constant_descriptor &operator=(push_constant_descriptor&&) = default;

	bool is_struct() const { return !children.empty(); }

	auto find(const children_descriptors_map_t::key_type &k) const { return children.find(k); }
	auto begin() const { return children.begin(); }
	auto end() const { return children.end(); }

	optional<const push_constant_descriptor*> operator[](push_constant_path &path) const {
		if (path.empty())
			return this;

		// Step a level into path
		auto name = path.step_in();
		auto it = find(name);
		if (it == end())
			return none;
		return it->second[path];
	}

	template <typename T>
	void validate() const {
		assert(variable && "Attempting to access a root node");
		variable->validate<T>();
	}

	auto offset() const { return total_offset; }
	auto size_bytes() const {
		assert(variable && "Attempting to access a root node");
		return variable->size_bytes();
	}

	auto &get_var() const { return *variable; }
};

}
}
