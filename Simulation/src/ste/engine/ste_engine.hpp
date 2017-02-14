//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <task_scheduler.hpp>
#include <lru_cache.hpp>
#include <ste_engine_storage_protocol.hpp>
#include <ste_engine_hid.hpp>
#include <ste_gl.hpp>

namespace StE {

template <typename TaskScheduler, typename Cache, typename GLContext, typename HID, typename Storage>
class ste_engine_impl {
	
};

using ste_engine = ste_engine_impl<task_scheduler, lru_cache<std::string>, GL::ste_gl_context, ste_engine_hid, ste_engine_storage_protocol>;

}
