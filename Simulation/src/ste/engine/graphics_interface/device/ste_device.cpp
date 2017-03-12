
#include <stdafx.hpp>
#include <ste_device.hpp>

using namespace StE::GL;

thread_local ste_presentation_surface::acquire_next_image_return_t ste_device::acquired_presentation_image;
