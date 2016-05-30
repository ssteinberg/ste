
#include "stdafx.hpp"
#include "hdr_create_histogram_task.hpp"

using namespace StE::Graphics;
using namespace StE::Core;

void hdr_create_histogram_task::set_context_state() const {
	0_atomic_idx = p->histogram;
	0_image_idx = (*p->hdr_lums)[0];
	2_storage_idx = p->hdr_bokeh_param_buffer;

	p->hdr_create_histogram.get().bind();
}

void hdr_create_histogram_task::dispatch() const {
	std::uint32_t zero = 0;
	p->histogram.clear(gli::FORMAT_R32_UINT_PACK32, &zero);

	GL::gl_current_context::get()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	GL::gl_current_context::get()->dispatch_compute(p->luminance_size.x / 32,
													p->luminance_size .y / 32,
													1);
}
