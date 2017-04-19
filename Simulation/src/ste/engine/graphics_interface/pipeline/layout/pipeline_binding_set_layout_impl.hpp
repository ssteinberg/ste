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

namespace _internal {

/**
*	@brief	Describes the descriptor set layout
*/
template <typename BindingLayout>
class pipeline_binding_set_layout_impl : public allow_type_decay<pipeline_binding_set_layout_impl<BindingLayout>, vk_descriptor_set_layout> {
public:
	using bindings_vec_t = std::vector<BindingLayout>;
	using name_bindings_map_t = boost::container::flat_map<std::string, typename bindings_vec_t::const_iterator>;

private:
	bindings_vec_t bindings;
	pipeline_layout_set_index set_idx{ 0 };
	name_bindings_map_t name_map;

	vk_descriptor_set_layout vk_layout;

private:
	template <typename T = BindingLayout>
	auto &access_it(typename bindings_vec_t::const_iterator it,
					std::enable_if_t<std::is_pointer_v<T>>* = nullptr) const {
		return **it;
	}
	template <typename T = BindingLayout>
	auto &access_it(typename bindings_vec_t::const_iterator it,
					std::enable_if_t<!std::is_pointer_v<T>>* = nullptr) const {
		return *it;
	}

	auto generate_vk_layout(const ste_context &ctx) {
		std::vector<vk_descriptor_set_layout_binding> vk_bindings;
		for (auto it = begin(); it != end(); ++it)
			vk_bindings.push_back(access_it<>(it));

		return vk_descriptor_set_layout(ctx.device(), vk_bindings);
	}

public:
	pipeline_binding_set_layout_impl(const ste_context &ctx,
									 bindings_vec_t &&bindings)
		: bindings(std::move(bindings)),
		vk_layout(generate_vk_layout(ctx))
	{
		assert(this->bindings.size());
		set_idx = access_it<>(this->bindings.begin()).set_idx();

		// Create name map
		for (auto it = begin(); it != end(); ++it) {
			name_map[access_it<>(it).name()] = it;
			assert(set_idx == access_it<>(it).set_idx());
		}
	}
	~pipeline_binding_set_layout_impl() noexcept {}

	pipeline_binding_set_layout_impl(pipeline_binding_set_layout_impl&&) = default;
	pipeline_binding_set_layout_impl &operator=(pipeline_binding_set_layout_impl&&) = default;

	/**
	*	@brief	Checks existance of a variable name
	*/
	bool exists(const std::string &name) const {
		auto it = name_map.find(name);
		return it != name_map.end();
	}

	/**
	*	@brief	Lookups a binding using variable name
	*/
	auto& operator[](const std::string &name) const {
		auto it = name_map.find(name);
		return access_it<>(it->second);
	}
	/**
	*	@brief	Lookups a binding by array index
	*/
	auto& operator[](std::size_t idx) const {
		return access_it<>(this->bindings.begin() + idx);
	}

	auto size() const { return bindings.size(); }
	auto begin() const { return bindings.begin(); }
	auto end() const { return bindings.end(); }

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
}
