//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_device_queue_batch.hpp>

#include <ste_device_queue_command_buffer.hpp>
#include <ste_device_queue_command_buffer_multishot.hpp>
#include <ste_device_queue_command_pool.hpp>

#include <memory>

namespace StE {
namespace GL {

template <typename UserData = void>
class ste_device_queue_batch_multishot : public _detail::ste_device_queue_batch_custom<UserData> {
	using Base = _detail::ste_device_queue_batch_custom<UserData>;

	friend class ste_device_queue;

public:
	using command_buffer_t = ste_device_queue_command_buffer<VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT>;
	using user_command_buffer_t = ste_device_queue_command_buffer_multishot;

protected:
	std::vector<command_buffer_t> command_buffers;
	std::vector<user_command_buffer_t> user_command_buffers;

	auto begin() const { return command_buffers.begin(); }
	auto end() const { return command_buffers.end(); }

	bool is_batch_complete() const override final {
		return false;
	}

public:
	template <typename... Ts>
	ste_device_queue_batch_multishot(Ts&&... ts)
		: Base(std::forward<Ts>(ts)...)
	{}
	virtual ~ste_device_queue_batch_multishot() noexcept {}

	ste_device_queue_batch_multishot(ste_device_queue_batch_multishot&&) = default;
	ste_device_queue_batch_multishot &operator=(ste_device_queue_batch_multishot&&) = default;

	auto& acquire_command_buffer() {
		command_buffers.emplace_back(Base::pool, vk_command_buffer_type::primary);
		return command_buffers.back();
	}

	auto& acquire_user_command_buffer(const vk_command_buffer_type &type) {
		user_command_buffers.emplace_back(Base::pool, type);
		return user_command_buffers.back();
	}
};

template <typename BatchT>
class ste_device_queue_batch_multishot_auditor {
private:
	using batch_t = BatchT;

	batch_t batch;

public:
	template <typename... Ts>
	ste_device_queue_batch_multishot_auditor(Ts&&... ts)
		: batch(std::forward<Ts>(ts)...)
	{}
	~ste_device_queue_batch_multishot_auditor() noexcept {}

	ste_device_queue_batch_multishot_auditor(ste_device_queue_batch_multishot_auditor&&) = default;
	ste_device_queue_batch_multishot_auditor &operator=(ste_device_queue_batch_multishot_auditor&&) = default;

	auto& acquire_command_buffer() {
		return batch.acquire_command_buffer();
	}

	auto& acquire_user_command_buffer(const vk_command_buffer_type &type) {
		return batch.acquire_user_command_buffer(type);
	}

	auto&& finalize() {
		return std::move(batch);
	}
};

}
}
