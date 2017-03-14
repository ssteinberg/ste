
#include <stdafx.hpp>
#include <ste_device_queue.hpp>

using namespace StE::GL;

thread_local ste_device_queue *ste_device_queue::static_device_queue_ptr = nullptr;
thread_local const vk_command_pool *ste_device_queue::static_command_pool_ptr = nullptr;
thread_local const vk_command_pool *ste_device_queue::static_command_pool_transient_ptr = nullptr;
thread_local vk_command_buffers *ste_device_queue::static_command_buffers_ptr = nullptr;
thread_local const vk_queue *ste_device_queue::static_queue_ptr = nullptr;
thread_local std::uint32_t ste_device_queue::static_queue_index = 0xFFFFFFFF;
