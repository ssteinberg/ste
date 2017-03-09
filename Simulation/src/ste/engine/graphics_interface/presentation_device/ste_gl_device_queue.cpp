
#include <stdafx.hpp>
#include <ste_gl_device_queue.hpp>

using namespace StE::GL;

thread_local vk_command_buffers *ste_gl_device_queue::command_buffers = nullptr;
