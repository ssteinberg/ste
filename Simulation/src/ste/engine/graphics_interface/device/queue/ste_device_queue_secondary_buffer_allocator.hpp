//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_device_queue_secondary_command_buffer.hpp>

#include <mutex>

namespace ste {
namespace gl {

template <typename Pool>
class ste_device_queue_secondary_buffer_allocator {
public:
	using buffer_t = ste_device_queue_secondary_command_buffer<ste_device_queue_secondary_buffer_allocator<Pool>, command_buffer_secondary<false>>;

	friend class buffer_t;

private:
	struct shared_data_t {
		alignas(std::hardware_destructive_interference_size) mutable std::mutex m;
	};

private:
	Pool pool;

	shared_data_t shared_data;

private:
	void dealloc(buffer_t &&buffer) const {
		std::unique_lock<std::mutex> l(shared_data.m);
		{
			// Move into temporary
			auto temp = std::move(buffer);
		}
	}

public:
	ste_device_queue_secondary_buffer_allocator(Pool &&pool)
		: pool(std::move(pool))
	{}

	auto allocate_secondary_buffer() const {
		std::unique_lock<std::mutex> l(shared_data.m);
		return buffer_t(this, pool.get().get().allocate_secondary_buffer());
	}

	void reset() {
		std::unique_lock<std::mutex> l(shared_data.m);
		pool.get().reset();
	}
	void reset_release() {
		std::unique_lock<std::mutex> l(shared_data.m);
		pool.get().reset_release();
	}
};

}
}
