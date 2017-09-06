//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_device_queue_secondary_command_buffer.hpp>

#include <mutex>
#include <lib/aligned_padded_ptr.hpp>

namespace ste {
namespace gl {

template <typename Pool>
class ste_device_queue_secondary_buffer_allocator {
public:
	using buffer_t = ste_device_queue_secondary_command_buffer<ste_device_queue_secondary_buffer_allocator<Pool>, command_buffer_secondary<false>>;

	friend class buffer_t;

private:
	Pool pool;

	mutable lib::aligned_padded_ptr<std::mutex> m;

private:
	void dealloc(buffer_t &&buffer) const {
		std::unique_lock<std::mutex> l(*m);
		{
			auto temp = std::move(buffer);
		}
	}

public:
	ste_device_queue_secondary_buffer_allocator(Pool &&pool)
		: pool(std::move(pool))
	{}

	auto allocate_secondary_buffer() const {
		std::unique_lock<std::mutex> l(*m);
		return buffer_t(this, pool.get().get().allocate_secondary_buffer());
	}

	void reset() {
		std::unique_lock<std::mutex> l(*m);
		pool.get().reset();
	}
	void reset_release() {
		std::unique_lock<std::mutex> l(*m);
		pool.get().reset_release();
	}
};

}
}
