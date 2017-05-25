
#include <stdafx.hpp>
#include <storage.hpp>
#include <rendering_system.hpp>

using namespace ste::gl;

storage_base::~storage_base() noexcept {
	rs->remove_storage(tag);
}
