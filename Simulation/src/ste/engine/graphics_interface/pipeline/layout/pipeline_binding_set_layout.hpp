//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <vk_descriptor_set_layout.hpp>
#include <pipeline_layout_set_index.hpp>

#include <allow_type_decay.hpp>

#include <string>
#include <boost/container/flat_map.hpp>

namespace StE {
namespace GL {

/**
*	@brief	Describes the descriptor set layout
*/
class pipeline_binding_set_layout : public allow_type_decay<pipeline_binding_set_layout, vk_descriptor_set_layout> {
public:
	using bindings_vec_t = std::vector<const pipeline_binding_set_layout_binding*>;
	using name_bindings_map_t = boost::container::flat_map<std::string, const pipeline_binding_set_layout_binding*>;

private:
	bindings_vec_t bindings;
	pipeline_layout_set_index set_idx{ 0 };
	name_bindings_map_t name_map;

	vk_descriptor_set_layout vk_layout;

private:
	auto generate_vk_layout(const ste_context &ctx) {
		std::vector<vk_descriptor_set_layout_binding> vk_bindings;
		for (auto &b : bindings)
			vk_bindings.push_back(*b);

		return vk_descriptor_set_layout(ctx.device(), vk_bindings);
	}

public:
	pipeline_binding_set_layout(const ste_context &ctx,
								bindings_vec_t &&bindings)
		: bindings(std::move(bindings)),
		vk_layout(generate_vk_layout(ctx))
	{
		assert(this->bindings.size());
		set_idx = this->bindings.back()->set_idx();

		// Create name map
		for (auto &b : this->bindings) {
			name_map[b->name()] = b;
			assert(set_idx == b->set_idx());
		}
	}
	~pipeline_binding_set_layout() noexcept {}

	pipeline_binding_set_layout(pipeline_binding_set_layout&&o) = default;
	pipeline_binding_set_layout &operator=(pipeline_binding_set_layout&&o) = default;

	/**
	 *	@brief	Lookups a binding using variable name
	 */
	const pipeline_binding_set_layout_binding* operator[](const std::string &name) const {
		auto it = name_map.find(name);
		if (it != name_map.end())
			return it->second;
		return nullptr;
	}
	auto begin() const { return bindings.begin(); }
	auto end() const { return bindings.end(); }
	auto size() const { return bindings.size(); }

	/**
	*	@brief	Returns the bindings that participate in this descriptor set
	*/
	auto& get_bindings() const { return bindings; }

	/**
	 *	@brief	Returns the Vulkan descriptor set layout
	 */
	auto &get() const {
		return vk_layout;
	}

	/**
	 *	@brief	Returns the binding set index
	 */
	auto &get_set_index() const {
		return set_idx;
	}
};

}
}
