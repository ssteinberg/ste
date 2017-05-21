//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <global_memory_barrier.hpp>
#include <buffer_memory_barrier.hpp>
#include <image_memory_barrier.hpp>

#include <pipeline_stage.hpp>

#include <lib/vector.hpp>

namespace ste {
namespace gl {

namespace _internal {

struct pipeline_barrier_barriers_expander {
	template <typename Barrier, typename... Barriers>
	void operator()(lib::vector<global_memory_barrier> &global_memory_barriers,
					lib::vector<buffer_memory_barrier> &buffer_barriers,
					lib::vector<image_memory_barrier>  &image_barriers,
					Barrier&& barrier, Barriers&&... barriers) {
		(*this)(global_memory_barriers,
				buffer_barriers,
				image_barriers,
				std::forward<Barrier>(barrier));
		(*this)(global_memory_barriers,
				buffer_barriers,
				image_barriers,
				std::forward<Barriers>(barriers)...);
	}
	template <typename Barrier>
	void operator()(lib::vector<global_memory_barrier> &global_memory_barriers,
					lib::vector<buffer_memory_barrier> &buffer_barriers,
					lib::vector<image_memory_barrier>  &image_barriers,
					Barrier&& barrier,
					std::enable_if_t<std::is_constructible_v<global_memory_barrier, Barrier>>* = nullptr) {
		global_memory_barriers.emplace_back(std::forward<Barrier>(barrier));
	}
	template <typename Barrier>
	void operator()(lib::vector<global_memory_barrier> &global_memory_barriers,
					lib::vector<buffer_memory_barrier> &buffer_barriers,
					lib::vector<image_memory_barrier>  &image_barriers,
					Barrier&& barrier,
					std::enable_if_t<std::is_constructible_v<buffer_memory_barrier, Barrier>>* = nullptr) {
		buffer_barriers.emplace_back(std::forward<Barrier>(barrier));
	}
	template <typename Barrier>
	void operator()(lib::vector<global_memory_barrier> &global_memory_barriers,
					lib::vector<buffer_memory_barrier> &buffer_barriers,
					lib::vector<image_memory_barrier>  &image_barriers,
					Barrier&& barrier,
					std::enable_if_t<std::is_constructible_v<image_memory_barrier, Barrier>>* = nullptr) {
		image_barriers.emplace_back(std::forward<Barrier>(barrier));
	}
};

}

class pipeline_barrier {
private:
	pipeline_stage src_stage;
	pipeline_stage dst_stage;
	lib::vector<global_memory_barrier> global_memory_barriers;
	lib::vector<buffer_memory_barrier> buffer_barriers;
	lib::vector<image_memory_barrier>  image_barriers;

public:
	pipeline_barrier(const pipeline_stage &src_stage,
					 const pipeline_stage &dst_stage)
		: src_stage(src_stage), dst_stage(dst_stage)
	{}
	pipeline_barrier(const pipeline_stage &src_stage,
					 const pipeline_stage &dst_stage,
					 const lib::vector<global_memory_barrier> &global_memory_barriers,
					 const lib::vector<buffer_memory_barrier> &buffer_barriers,
					 const lib::vector<image_memory_barrier>  &image_barriers)
		: src_stage(src_stage), dst_stage(dst_stage),
		global_memory_barriers(global_memory_barriers),
		buffer_barriers(buffer_barriers),
		image_barriers(image_barriers)
	{}
	template <typename... Barriers>
	pipeline_barrier(const pipeline_stage &src_stage,
					 const pipeline_stage &dst_stage,
					 Barriers&&... barriers)
		: src_stage(src_stage), dst_stage(dst_stage) 
	{
		_internal::pipeline_barrier_barriers_expander()(global_memory_barriers,
														buffer_barriers,
														image_barriers,
														std::forward<Barriers>(barriers)...);
	}
	~pipeline_barrier() noexcept {}

	pipeline_barrier(pipeline_barrier &&) = default;
	pipeline_barrier &operator=(pipeline_barrier &&) = default;
	pipeline_barrier(const pipeline_barrier &) = default;
	pipeline_barrier &operator=(const pipeline_barrier &) = default;

	auto& get_src_stage() const { return src_stage; }
	auto& get_dst_stage() const { return dst_stage; }
	auto& get_global_memory_barriers() const { return global_memory_barriers; }
	auto& get_buffer_barriers() const { return buffer_barriers; }
	auto& get_image_barriers() const { return image_barriers; }
};

}
}
