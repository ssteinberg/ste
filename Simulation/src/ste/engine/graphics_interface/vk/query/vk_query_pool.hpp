//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_host_allocator.hpp>
#include <vk_logical_device.hpp>
#include <vk_query.hpp>

#include <optional.hpp>

#include <alias.hpp>
#include <allow_type_decay.hpp>

namespace ste {
namespace gl {

namespace vk {

template <typename host_allocator = vk_host_allocator<>>
class vk_query_pool : public allow_type_decay<vk_query_pool<host_allocator>, VkQueryPool> {
private:
	optional<VkQueryPool> query_pool;
	alias<const vk_logical_device<host_allocator>> device;
	std::uint32_t size;

protected:
	vk_query_pool(const vk_logical_device<host_allocator> &device,
				  const VkQueryType &type,
				  std::uint32_t size,
				  VkQueryPipelineStatisticFlags pipeline_statistic_flags = 0) : device(device), size(size) {
		VkQueryPoolCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = 0;
		create_info.queryType = type;
		create_info.queryCount = size;
		create_info.pipelineStatistics = pipeline_statistic_flags;

		VkQueryPool query_pool;
		const vk_result res = vkCreateQueryPool(device, &create_info, &host_allocator::allocation_callbacks(), &query_pool);
		if (!res) {
			throw vk_exception(res);
		}

		this->query_pool = query_pool;
	}

public:
	virtual ~vk_query_pool() noexcept {
		destroy_query_pool();
	}

	vk_query_pool(vk_query_pool &&) = default;
	vk_query_pool &operator=(vk_query_pool &&o) noexcept {
		destroy_query_pool();

		query_pool = std::move(o.query_pool);
		device = std::move(o.device);
		size = o.size;

		return *this;
	}
	vk_query_pool(const vk_query_pool &) = delete;
	vk_query_pool &operator=(const vk_query_pool &) = delete;

	void destroy_query_pool() {
		if (query_pool) {
			vkDestroyQueryPool(device.get(), *this, &host_allocator::allocation_callbacks());
			query_pool = none;
		}
	}

	template <typename T>
	void read_results(T *output,
					  std::uint32_t first_query,
					  std::uint32_t queries_count,
					  std::uint64_t stride,
					  VkQueryResultFlags flags = 0) const {
		const vk_result res = vkGetQueryPoolResults(device.get(), *this,
													first_query, queries_count,
													sizeof(T) * queries_count, output,
													stride, flags);
		if (!res) {
			throw vk_exception(res);
		}
	}

	auto& get_creating_device() const { return device.get(); }
	auto& get() const { return query_pool.get(); }
	
	auto operator[](std::uint32_t idx) const {
		assert(idx < size);
		return vk_query<host_allocator>(*this, idx);
	}
};

template <typename host_allocator = vk_host_allocator<>>
class vk_occlusion_query_pool : public vk_query_pool<host_allocator> {
public:
	vk_occlusion_query_pool(const vk_logical_device<host_allocator> &device,
							std::uint32_t size)
		: vk_query_pool(device,
						VK_QUERY_TYPE_OCCLUSION,
						size)
	{}
};

template <typename host_allocator = vk_host_allocator<>>
class vk_pipeline_statistics_query_pool : public vk_query_pool<host_allocator> {
public:
	vk_pipeline_statistics_query_pool(const vk_logical_device<host_allocator> &device,
									  std::uint32_t size,
									  VkQueryPipelineStatisticFlags pipeline_statistic_flags)
		: vk_query_pool(device,
						VK_QUERY_TYPE_PIPELINE_STATISTICS,
						size,
						pipeline_statistic_flags) 
	{}
};

template <typename host_allocator = vk_host_allocator<>>
class vk_timestamp_query_pool : public vk_query_pool<host_allocator> {
public:
	vk_timestamp_query_pool(const vk_logical_device<host_allocator> &device,
							std::uint32_t size)
		: vk_query_pool(device,
						VK_QUERY_TYPE_TIMESTAMP,
						size)
	{}
};

}

}
}
