// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <camera.hpp>
#include <camera_projection_reversed_infinite_perspective.hpp>

namespace ste {
namespace graphics {

using primary_renderer_camera = camera<float, camera_projection_reversed_infinite_perspective>;

}
}
