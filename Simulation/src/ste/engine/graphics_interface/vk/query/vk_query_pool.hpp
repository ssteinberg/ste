//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>

#include <optional.hpp>

#include <vk_query.hpp>
#include <string>

namespace StE {
namespace GL {

class vk_query_pool {
private:
	optional<VkQueryPool> query_pool;
	const vk_logical_device &device;
	std::uint32_t size;

protected:
	vk_query_pool(const vk_logical_device &device,
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
		vk_result res = vkCreateQueryPool(device, &create_info, nullptr, &query_pool);
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
	vk_query_pool &operator=(vk_query_pool &&) = default;
	vk_query_pool(const vk_query_pool &) = delete;
	vk_query_pool &operator=(const vk_query_pool &) = delete;

	void destroy_query_pool() {
		if (query_pool) {
			vkDestroyQueryPool(device, *this, nullptr);
			query_pool = none;
		}
	}

	std::string read_results(std::size_t data_size,
							 std::uint32_t first_query,
							 std::uint32_t queries_count,
							 std::uint64_t stride,
							 VkQueryResultFlags flags = 0) const {
		std::string data;
		data.resize(data_size);

		vk_result res = vkGetQueryPoolResults(device, *this,
											  first_query, queries_count,
											  data_size, &data[0], stride,
											  flags);
		if (!res) {
			throw vk_exception(res);
		}

		return data;
	}

	auto& get_creating_device() const { return device; }
	auto& get_query_pool() const { return query_pool.get(); }

	vk_query operator[](std::uint32_t idx) const {
		assert(idx < size);
		return vk_query(*this, idx);
	}

	operator VkQueryPool() const { return get_query_pool(); }
};

class vk_occlusion_query_pool : public vk_query_pool {
public:
	vk_occlusion_query_pool(const vk_logical_device &device,
							std::uint32_t size) : vk_query_pool(device,
																VK_QUERY_TYPE_OCCLUSION,
																size) {}
};

class vk_pipeline_statistics_query_pool : public vk_query_pool {
public:
	vk_pipeline_statistics_query_pool(const vk_logical_device &device,
									  std::uint32_t size,
									  VkQueryPipelineStatisticFlags pipeline_statistic_flags)
		: vk_query_pool(device,
						VK_QUERY_TYPE_PIPELINE_STATISTICS,
						size,
						pipeline_statistic_flags) {}
};

class vk_timestamp_query_pool : public vk_query_pool {
public:
	vk_timestamp_query_pool(const vk_logical_device &device,
							std::uint32_t size) : vk_query_pool(device,
																VK_QUERY_TYPE_TIMESTAMP,
																size) {}
};

}
}
