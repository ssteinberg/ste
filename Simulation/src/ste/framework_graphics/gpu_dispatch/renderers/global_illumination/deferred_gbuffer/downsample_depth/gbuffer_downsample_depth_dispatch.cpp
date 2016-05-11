
#include "stdafx.hpp"
#include "gbuffer_downsample_depth_dispatch.hpp"

using namespace StE::Graphics;

void gbuffer_downsample_depth_dispatch::set_context_state() const {

	program->bind();
}

void gbuffer_downsample_depth_dispatch::dispatch() const {

}
