//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vk_descriptor_set_write_resource.hpp>
#include <pipeline_layout_set_index.hpp>

#include <unordered_map>

namespace StE {
namespace GL {

class pipeline_resource_binding_queue {
private:
	using writes_queue_t = std::vector<vk_descriptor_set_write_resource>;
	using write_sets_t = std::unordered_map<pipeline_layout_set_index, writes_queue_t>;

private:
	write_sets_t q;

public:
	pipeline_resource_binding_queue() = default;
	~pipeline_resource_binding_queue() noexcept {}

	bool empty() const { return q.empty(); }

	auto& operator[](const pipeline_layout_set_index &idx) { return q[idx]; }

	auto begin() const { return q.begin(); }
	auto end() const { return q.end(); }
	void clear() { q.clear(); }
};

}
}
