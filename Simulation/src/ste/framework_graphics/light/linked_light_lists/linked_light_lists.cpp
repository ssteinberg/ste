
#include <stdafx.hpp>
#include <linked_light_lists.hpp>

#include <light_storage.hpp>
#include <surface_factory.hpp>

using namespace ste;
using namespace ste::graphics;

constexpr unsigned linked_light_lists::lll_image_res_multiplier;

linked_light_lists::linked_light_lists(const ste_context &ctx, 
									   const glm::uvec2 &extent)
	: ctx(ctx),
	lll(ctx, gl::buffer_usage::storage_buffer),
	lll_counter(ctx, lib::vector<lll_counter_element>(1), gl::buffer_usage::storage_buffer),
	lll_heads(ctx, resource::surface_factory::image_empty_2d<gl::format::r32_sfloat>(ctx, gl::image_usage::storage, gl::image_layout::general, extent)),
	lll_size(ctx, resource::surface_factory::image_empty_2d<gl::format::r8_uint>(ctx, gl::image_usage::storage, gl::image_layout::general, extent)),
	lll_low_detail_heads(ctx, resource::surface_factory::image_empty_2d<gl::format::r32_sfloat>(ctx, gl::image_usage::storage, gl::image_layout::general, extent)),
	lll_low_detail_size(ctx, resource::surface_factory::image_empty_2d<gl::format::r8_uint>(ctx, gl::image_usage::storage, gl::image_layout::general, extent)),
	extent(extent)
{
	up_to_date.test_and_set(std::memory_order_release);
}

void linked_light_lists::resize(const glm::uvec2 &extent) {
	auto t = extent / lll_image_res_multiplier;
	if (t.x <= 0 || t.y <= 0 || t == this->extent)
		return;

	up_to_date.clear(std::memory_order_release);
	this->extent = t;
	std::atomic_thread_fence(std::memory_order_release);

	lll_heads = ste_resource<gl::texture<gl::image_type::image_2d>>(ctx, resource::surface_factory::image_empty_2d<gl::format::r32_sfloat>(ctx, gl::image_usage::storage, gl::image_layout::general, extent));
	lll_size = ste_resource<gl::texture<gl::image_type::image_2d>>(ctx, resource::surface_factory::image_empty_2d<gl::format::r8_uint>(ctx, gl::image_usage::storage, gl::image_layout::general, extent));
	lll_low_detail_heads = ste_resource<gl::texture<gl::image_type::image_2d>>(ctx, resource::surface_factory::image_empty_2d<gl::format::r32_sfloat>(ctx, gl::image_usage::storage, gl::image_layout::general, extent));
	lll_low_detail_size = ste_resource<gl::texture<gl::image_type::image_2d>>(ctx, resource::surface_factory::image_empty_2d<gl::format::r8_uint>(ctx, gl::image_usage::storage, gl::image_layout::general, extent));
}

void linked_light_lists::resize_internal(gl::command_recorder &recorder) {
	std::atomic_thread_fence(std::memory_order_acquire);

	recorder << lll.resize_cmd(extent.x * extent.y * total_max_active_lights_per_frame);
}
