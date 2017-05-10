
#include <stdafx.hpp>
#include <text_fragment.hpp>
#include <text_manager.hpp>

#include <attributed_string.hpp>

using namespace ste::text;

text_fragment::text_fragment(text_manager *tm)
	: tm(tm),
	vertex_buffer(tm->context, gl::buffer_usage::vertex_buffer) 
{
	draw_task.attach_pipeline(tm->pipeline);
	draw_task.attach_vertex_buffer(vertex_buffer);
}
