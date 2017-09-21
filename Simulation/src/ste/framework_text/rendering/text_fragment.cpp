
#include <stdafx.hpp>
#include <text_fragment.hpp>
#include <text_manager.hpp>

using namespace ste;
using namespace ste::text;

text_fragment::text_fragment(const ste_context &ctx,
							 text_manager *tm)
	: fragment(ctx),
	  tm(tm),
	  vertex_buffer(tm->context,
					gl::buffer_usage::vertex_buffer,
					"text_fragment vertex buffer") {
	draw_task.attach_pipeline(tm->pipeline);
	draw_task.attach_vertex_buffer(vertex_buffer);
}
