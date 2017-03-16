
#include <stdafx.hpp>
#include <ste_device_queue.hpp>

using namespace StE::GL;

thread_local ste_device_queue *ste_device_queue::static_device_queue_ptr = nullptr;
thread_local const vk_queue *ste_device_queue::static_queue_ptr = nullptr;
thread_local std::uint32_t ste_device_queue::static_queue_index = 0xFFFFFFFF;
