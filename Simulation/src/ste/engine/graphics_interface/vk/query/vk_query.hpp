//	StE
// � Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_host_allocator.hpp>

#include <lib/string.hpp>
#include <allow_type_decay.hpp>

namespace ste {
namespace gl {

namespace vk {

template <typename>
class vk_query_pool;

template <typename host_allocator = vk_host_allocator<>>
class vk_query : public allow_type_decay<vk_query<host_allocator>, vk_query_pool<host_allocator>> {
private:
	const vk_query_pool<host_allocator> &pool;
	const std::uint32_t index;

public:
	vk_query(const vk_query_pool<host_allocator> &pool,
			 std::uint32_t index) : pool(pool), index(index) {}
	~vk_query() noexcept {}

	vk_query(vk_query &&) = default;
	vk_query &operator=(vk_query &&) = default;
	vk_query(const vk_query &) = delete;
	vk_query &operator=(const vk_query &) = delete;

	/*
	 *	@brief	Attempts to read query result.
	 *			If query is unavailable, returns empty data.
	 *			
	 *	@return	Query data if available, empty data otherwise.
	 *	
	 *	@throws	vk_exception	On Vulkan exception
	 */
	lib::string read_results(byte_t data_size,
							 VkQueryResultFlags flags = 0) const {
		lib::string data;
		data.resize(static_cast<std::size_t>(data_size));
		if (!pool.read_results(data.data(),
							   index, 1,
							   data_size,
							   flags)) {
			return lib::string{};
		}

		return data;
	}

	auto& get_pool() const { return pool; }
	auto ge_query_index() const { return index; }
};

}

}
}
