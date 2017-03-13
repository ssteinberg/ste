
#include <stdafx.hpp>
#include <ste_device.hpp>

using namespace StE::GL;

thread_local ste_device::presentation_next_image_t ste_device::acquired_presentation_image;
