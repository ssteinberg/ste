
#include <stdafx.hpp>
#include <ste_gl_device_queue.hpp>

using namespace StE::GL;

thread_local vk_command_buffers *ste_gl_device_queue::static_command_buffers_ptr = nullptr;
thread_local vk_queue *ste_gl_device_queue::static_queue_ptr = nullptr;
thread_local std::uint32_t ste_gl_device_queue::static_queue_index = 0xFFFFFFFF;
