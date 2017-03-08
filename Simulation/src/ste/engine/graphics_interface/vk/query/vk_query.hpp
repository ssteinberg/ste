//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <ste.hpp>

#include <vulkan/vulkan.h>

namespace StE {
namespace GL {

class vk_query_pool;

class vk_query {
private:
	const vk_query_pool &pool;
	const std::uint32_t index;

public:
	vk_query(const vk_query_pool &pool,
			 std::uint32_t index) : pool(pool), index(index) {}
	~vk_query() noexcept {}

	vk_query(vk_query &&) = default;
	vk_query &operator=(vk_query &&) = default;
	vk_query(const vk_query &) = delete;
	vk_query &operator=(const vk_query &) = delete;

	std::string read_results(std::size_t data_size,
							 VkQueryResultFlags flags = 0) const;

	auto& get_pool() const { return pool; }
	auto ge_query_index() const { return index; }
};

}
}
