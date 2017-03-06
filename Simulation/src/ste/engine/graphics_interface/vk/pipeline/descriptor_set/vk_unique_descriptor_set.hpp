//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <ste.hpp>

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>
#include <vk_descriptor_set.hpp>
#include <vk_descriptor_pool.hpp>

namespace StE {
namespace GL {

class vk_unique_descriptor_set {
private:
	using binding_t = vk_descriptor_set_layout_binding;
	using binding_set_t = std::vector<vk_descriptor_set_layout_binding>;

private:
	vk_descriptor_pool pool;
	vk_descriptor_set set;

private:
	static auto descriptor_set_layouts_from_binding_sets(const vk_logical_device &device, const std::vector<binding_set_t> &binding_sets) {
		std::vector<vk_descriptor_set_layout> layouts;
		layouts.reserve(binding_sets.size());
		for (int i=0; i<binding_sets.size(); ++i) {
			layouts.emplace_back(device, binding_sets[i]);
		}

		return layouts;
	}

	static auto join_binding_sets(const std::vector<binding_set_t> &binding_sets) {
		int size = 0;
		for (auto &s : binding_sets)
			size += s.size();

		std::vector<binding_t> v;
		v.reserve(size);
		for (auto &s : binding_sets)
			v.insert(end(v), begin(s), end(s));

		return v;
	}

public:
	vk_unique_descriptor_set(const vk_logical_device &device,
							 const std::vector<binding_set_t> &binding_sets)
		: pool(device, 1, join_binding_sets(binding_sets), false),
		set(pool.allocate_descriptor_set(descriptor_set_layouts_from_binding_sets(device, binding_sets)))
	{}

	~vk_unique_descriptor_set() noexcept {}

	vk_unique_descriptor_set(vk_unique_descriptor_set &&) = default;
	vk_unique_descriptor_set &operator=(vk_unique_descriptor_set &&) = default;
	vk_unique_descriptor_set(const vk_unique_descriptor_set &) = delete;
	vk_unique_descriptor_set &operator=(const vk_unique_descriptor_set &) = delete;

	auto& get() const { return set; }
	auto& get() { return set; }
	auto& operator*() const { return get(); }
	auto& operator*() { return get(); }
	auto* operator->() const { return &get(); }
	auto* operator->() { return &get(); }
};

}
}
