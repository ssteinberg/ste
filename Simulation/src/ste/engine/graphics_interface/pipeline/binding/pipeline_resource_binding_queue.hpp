//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vk_descriptor_set_write_resource.hpp>

#include <deque>

namespace StE {
namespace GL {

class pipeline_resource_binding_queue {
private:
	using queue_t = std::deque<vk_descriptor_set_write_resource>;

private:
	queue_t q;

public:
	pipeline_resource_binding_queue() = default;
	~pipeline_resource_binding_queue() noexcept {}

	bool empty() const { return q.empty(); }
	void insert(const vk_descriptor_set_write_resource &w) {
		q.push_back(w);
	}

	auto begin() const { return q.begin(); }
	auto end() const { return q.end(); }
	void clear() { q.clear(); }
};

}
}
