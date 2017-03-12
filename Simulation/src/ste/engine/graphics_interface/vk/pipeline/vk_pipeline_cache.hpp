//	StE
// � Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_physical_device_descriptor.hpp>
#include <vk_logical_device.hpp>
#include <vk_result.hpp>
#include <vk_exception.hpp>

#include <optional.hpp>

#include <string>
#include <istream>

namespace StE {
namespace GL {

class vk_pipeline_cache {
private:
	static constexpr std::uint32_t header_magic_and_version = 0xCAC8E001;
	static constexpr int header_uuid_size = VK_UUID_SIZE;

private:
	struct vk_pipeline_cache_header {
		std::uint32_t magic{ header_magic_and_version };
		std::uint32_t vendor_id;
		std::uint32_t device_id;
		std::uint32_t pipeline_cache_uuid_length{ header_uuid_size };
		std::uint8_t  pipeline_cache_uuid[header_uuid_size];
	};

private:
	optional<VkPipelineCache> cache;
	const vk_logical_device &device;

private:
	static optional<std::string> read_stream(const vk_physical_device_descriptor &device_descriptor, std::istream &input_stream) {
		vk_pipeline_cache_header header = { 0, 0, 0, 0 };
		if (!input_stream.read(reinterpret_cast<char*>(&header), sizeof(header))) {
			// Stream invalid
			return none;
		}

		// Check header
		if (header.magic != header_magic_and_version ||
			header.vendor_id != device_descriptor.properties.vendorID ||
			header.device_id != device_descriptor.properties.deviceID ||
			header.pipeline_cache_uuid_length != header_uuid_size) {
			return none;
		}
		// And cache pipeline uuid
		if (memcmp(&header.pipeline_cache_uuid, device_descriptor.properties.pipelineCacheUUID, header_uuid_size) != 0) {
			return none;
		}

		// Read cache data
		std::string data;
		static constexpr int buffer_size = 4096;
		char buffer[buffer_size];
		while (input_stream.read(buffer, buffer_size)) {
			data.append(buffer, buffer + buffer_size);
		}
		if (!input_stream.eof()) {
			// EOF not reached
			return none;
		}

		return data;
	}

	void create(const VkPipelineCacheCreateInfo &create_info) {
		VkPipelineCache pipeline_cache;
		vk_result res = vkCreatePipelineCache(device, &create_info, nullptr, &pipeline_cache);
		if (!res) {
			throw vk_exception(res);
		}

		this->cache = pipeline_cache;
	}

public:
	vk_pipeline_cache(const vk_logical_device &device,
					  const std::string &initial_data) : device(device) {
		VkPipelineCacheCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = 0;
		create_info.initialDataSize = initial_data.length();
		create_info.pInitialData = initial_data.length() ? initial_data.data() : nullptr;

		create(create_info);
	}
	vk_pipeline_cache(const vk_logical_device &device, std::istream &input_stream) : device(device) {
		auto data = read_stream(device.get_physical_device_descriptor(), input_stream);

		VkPipelineCacheCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = 0;
		create_info.initialDataSize = data ? data.get().length() : 0;
		create_info.pInitialData = data ? data.get().data() : nullptr;

		create(create_info);
	}
	vk_pipeline_cache(const vk_logical_device &device, 
					  const std::vector<std::reference_wrapper<const vk_pipeline_cache>> &src) : vk_pipeline_cache(device) {
		assert(src.size() && "Must provide at least a single source cache");

		std::vector<VkPipelineCache> ids;
		for (auto &c : src)
			ids.push_back(c.get());
		vkMergePipelineCaches(device, *this, ids.size(), &ids[0]);
	}
	vk_pipeline_cache(const vk_logical_device &device) : vk_pipeline_cache(device, std::string()) {}
	~vk_pipeline_cache() noexcept {
		destroy_pipeline_cache();
	}

	vk_pipeline_cache(vk_pipeline_cache &&) = default;
	vk_pipeline_cache &operator=(vk_pipeline_cache &&) = default;
	vk_pipeline_cache(const vk_pipeline_cache &) = delete;
	vk_pipeline_cache &operator=(const vk_pipeline_cache &) = delete;

	void destroy_pipeline_cache() {
		if (cache) {
			vkDestroyPipelineCache(device, *this, nullptr);
			cache = none;
		}
	}

	auto read_raw_cache_data() const {
		std::string data;
		std::size_t size;

		{
			// Read data size
			vk_result res = vkGetPipelineCacheData(device, *this, &size, nullptr);
			if (!res) {
				throw vk_exception(res);
			}
		}

		{
			// Read data
			data.resize(size);
			vk_result res = vkGetPipelineCacheData(device, *this, &size, &data[0]);
			if (!res) {
				throw vk_exception(res);
			}
		}

		return data;
	}

	auto& get_pipeline_cache() const { return cache.get(); }

	operator VkPipelineCache() const { return get_pipeline_cache(); }

	friend std::ostream& operator<<(std::ostream &stream, const vk_pipeline_cache& cache) {
		const vk_physical_device_descriptor &device_descriptor = cache.device.get_physical_device_descriptor();

		// Write header
		vk_pipeline_cache_header header;
		header.device_id = device_descriptor.properties.deviceID;
		header.vendor_id = device_descriptor.properties.vendorID;
		memcpy(header.pipeline_cache_uuid, device_descriptor.properties.pipelineCacheUUID, header_uuid_size);
		stream.write(reinterpret_cast<const char*>(&header), sizeof(header));

		// Write cache data
		std::string data = cache.read_raw_cache_data();
		stream.write(data.data(), data.length());

		return stream;
	}
};

}
}
