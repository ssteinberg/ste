//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <global_memory_barrier.hpp>
#include <buffer_memory_barrier.hpp>
#include <image_memory_barrier.hpp>

#include <pipeline_stage.hpp>

#include <vector>

namespace ste {
namespace gl {

class pipeline_barrier {
private:
	pipeline_stage src_stage;
	pipeline_stage dst_stage;
	std::vector<global_memory_barrier> global_memory_barriers;
	std::vector<buffer_memory_barrier> buffer_barriers;
	std::vector<image_memory_barrier>  image_barriers;

public:
	pipeline_barrier(const pipeline_stage &src_stage,
					 const pipeline_stage &dst_stage)
		: src_stage(src_stage), dst_stage(dst_stage)
	{}
	pipeline_barrier(const pipeline_stage &src_stage,
					 const pipeline_stage &dst_stage,
					 const std::vector<global_memory_barrier> &global_memory_barriers,
					 const std::vector<buffer_memory_barrier> &buffer_barriers,
					 const std::vector<image_memory_barrier>  &image_barriers)
		: src_stage(src_stage), dst_stage(dst_stage),
		global_memory_barriers(global_memory_barriers),
		buffer_barriers(buffer_barriers),
		image_barriers(image_barriers)
	{}
	pipeline_barrier(const pipeline_stage &src_stage,
					 const pipeline_stage &dst_stage,
					 const std::vector<global_memory_barrier> &global_memory_barriers)
		: src_stage(src_stage), dst_stage(dst_stage),
		global_memory_barriers(global_memory_barriers)
	{}
	pipeline_barrier(const pipeline_stage &src_stage,
					 const pipeline_stage &dst_stage,
					 const std::vector<buffer_memory_barrier> &buffer_barriers)
		: src_stage(src_stage), dst_stage(dst_stage),
		buffer_barriers(buffer_barriers)
	{}
	pipeline_barrier(const pipeline_stage &src_stage,
					 const pipeline_stage &dst_stage,
					 const std::vector<image_memory_barrier> &image_barriers)
		: src_stage(src_stage), dst_stage(dst_stage),
		image_barriers(image_barriers)
	{}
	pipeline_barrier(const pipeline_stage &src_stage,
					 const pipeline_stage &dst_stage,
					 const global_memory_barrier &global_memory_barrier)
		: src_stage(src_stage), dst_stage(dst_stage),
		global_memory_barriers({ global_memory_barrier })
	{}
	pipeline_barrier(const pipeline_stage &src_stage,
					 const pipeline_stage &dst_stage,
					 const buffer_memory_barrier &buffer_barrier)
		: src_stage(src_stage), dst_stage(dst_stage),
		buffer_barriers({ buffer_barrier })
	{}
	pipeline_barrier(const pipeline_stage &src_stage,
					 const pipeline_stage &dst_stage,
					 const image_memory_barrier &image_barrier)
		: src_stage(src_stage), dst_stage(dst_stage),
		image_barriers({ image_barrier })
	{}
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
