//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vk_logical_device.hpp>
#include <vk_descriptor_set_layout.hpp>
#include <pipeline_layout_set_index.hpp>

#include <pipeline_layout_exceptions.hpp>

#include <allow_type_decay.hpp>

#include <lib/string.hpp>
#include <lib/flat_map.hpp>

namespace ste {
namespace gl {

namespace _internal {

/**
*	@brief	Describes the descriptor set layout
*/
template <typename BindingLayout>
class pipeline_binding_set_layout_impl : public allow_type_decay<pipeline_binding_set_layout_impl<BindingLayout>, vk::vk_descriptor_set_layout<>> {
public:
	using bindings_vec_t = lib::vector<BindingLayout>;
	using name_bindings_map_t = lib::flat_map<lib::string, typename bindings_vec_t::const_iterator>;

private:
	bindings_vec_t bindings;
	pipeline_layout_set_index set_idx{ 0 };
	name_bindings_map_t name_map;

	vk::vk_descriptor_set_layout<> vk_layout;

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

	auto generate_vk_layout(const vk::vk_logical_device<> &device) {
		lib::vector<vk::vk_descriptor_set_layout_binding> vk_bindings;
		for (auto it = begin(); it != end(); ++it)
			vk_bindings.push_back(access_it<>(it));

		return vk::vk_descriptor_set_layout<>(device,
											  vk_bindings);
	}

public:
	pipeline_binding_set_layout_impl(const vk::vk_logical_device<> &device,
									 bindings_vec_t &&bindings)
		: bindings(std::move(bindings)),
		vk_layout(generate_vk_layout(device))
	{
		assert(this->bindings.size());
		set_idx = access_it<>(this->bindings.begin()).set_idx();

		// Create name map
		for (auto it = begin(); it != end(); ++it) {
			assert(set_idx == access_it<>(it).set_idx());

			// Check for duplicate names
			auto ret = name_map.try_emplace(access_it<>(it).name(), it);
			if (!ret.second) {
				// Name already exists
				throw pipeline_layout_duplicate_variable_name_exception();
			}
		}
	}
	~pipeline_binding_set_layout_impl() noexcept {}

	pipeline_binding_set_layout_impl(pipeline_binding_set_layout_impl&&) = default;
	pipeline_binding_set_layout_impl &operator=(pipeline_binding_set_layout_impl&&) = default;

	/**
	*	@brief	Checks existance of a variable name
	*/
	bool exists(const lib::string &name) const {
		auto it = name_map.find(name);
		return it != name_map.end();
	}

	/**
	*	@brief	Lookups a binding using variable name
	*/
	auto& operator[](const lib::string &name) const {
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
