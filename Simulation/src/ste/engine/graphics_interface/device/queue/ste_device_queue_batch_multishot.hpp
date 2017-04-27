//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_device_queue_batch.hpp>

#include <command_buffer.hpp>
#include <ste_device_queue_command_pool.hpp>

#include <memory>

namespace ste {
namespace gl {

template <typename UserData = void>
class ste_device_queue_batch_multishot : public _detail::ste_device_queue_batch_pool<UserData> {
	using Base = _detail::ste_device_queue_batch_pool<UserData>;

	friend class ste_device_queue;

public:
	using command_buffer_t = command_buffer_primary_multishot<false>;
	using user_command_buffer_t = command_buffer_secondary<false>;

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
		command_buffers.emplace_back(static_cast<command_pool&>(Base::pool).allocate_primary_multishot_buffer());
		return command_buffers.back();
	}

	auto& acquire_secondary_command_buffer(const vk::vk_command_buffer_type &type) {
		user_command_buffers.emplace_back(static_cast<command_pool&>(Base::pool).allocate_secondary_buffer());
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

	auto& acquire_user_command_buffer(const vk::vk_command_buffer_type &type) {
		return batch.acquire_user_command_buffer(type);
	}

	auto&& finalize() {
		return std::move(batch);
	}
};

}
}
