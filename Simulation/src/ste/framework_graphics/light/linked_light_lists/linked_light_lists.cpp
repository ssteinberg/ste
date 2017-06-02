
#include <stdafx.hpp>
#include <linked_light_lists.hpp>

#include <light_storage.hpp>
#include <surface_factory.hpp>

using namespace ste;
using namespace ste::graphics;

constexpr int linked_light_lists::lll_image_res_multiplier;

linked_light_lists::linked_light_lists(const ste_context &ctx, glm::ivec2 size)
	: ctx(ctx),
	lll(ctx, gl::buffer_usage::storage_buffer),
	lll_counter(ctx, lib::vector<lll_counter_element>(1), gl::buffer_usage::storage_buffer),
	lll_heads(ctx, resource::surface_factory::image_empty_2d<gl::format::r32_sfloat>(ctx, gl::image_usage::storage, gl::image_layout::general, size)),
	lll_size(ctx, resource::surface_factory::image_empty_2d<gl::format::r8_uint>(ctx, gl::image_usage::storage, gl::image_layout::general, size)),
	lll_low_detail_heads(ctx, resource::surface_factory::image_empty_2d<gl::format::r32_sfloat>(ctx, gl::image_usage::storage, gl::image_layout::general, size)),
	lll_low_detail_size(ctx, resource::surface_factory::image_empty_2d<gl::format::r8_uint>(ctx, gl::image_usage::storage, gl::image_layout::general, size)),
	size(size)
{}

void linked_light_lists::resize_internal(gl::command_recorder &recorder) {
	lll_heads = ste_resource<gl::texture<gl::image_type::image_2d>>(ctx, resource::surface_factory::image_empty_2d<gl::format::r32_sfloat>(ctx, gl::image_usage::storage, gl::image_layout::general, size));
	lll_size = ste_resource<gl::texture<gl::image_type::image_2d>>(ctx, resource::surface_factory::image_empty_2d<gl::format::r8_uint>(ctx, gl::image_usage::storage, gl::image_layout::general, size));
	lll_low_detail_heads = ste_resource<gl::texture<gl::image_type::image_2d>>(ctx, resource::surface_factory::image_empty_2d<gl::format::r32_sfloat>(ctx, gl::image_usage::storage, gl::image_layout::general, size));
	lll_low_detail_size = ste_resource<gl::texture<gl::image_type::image_2d>>(ctx, resource::surface_factory::image_empty_2d<gl::format::r8_uint>(ctx, gl::image_usage::storage, gl::image_layout::general, size));

	recorder << lll.resize_cmd(size.x * size.y * total_max_active_lights_per_frame);
}

//void linked_light_lists::bind_lll_buffer(bool low_detail) const {
//	using namespace ste::Core;
//
//	11_storage_idx = lll;
//
//	if (low_detail) {
//		5_image_idx = (*lll_low_detail_size)[0].with_access(Core::image_access_mode::Read);
//		6_image_idx = (*lll_low_detail_heads)[0].with_access(Core::image_access_mode::Read);
//	}
//	else {
//		5_image_idx = (*lll_size)[0].with_access(Core::image_access_mode::Read);
//		6_image_idx = (*lll_heads)[0].with_access(Core::image_access_mode::Read);
//	}
//}
//
//void linked_light_lists::bind_readwrite_lll_buffers() const {
//	using namespace ste::Core;
//
//	11_storage_idx = lll;
//	4_image_idx = (*lll_size)[0].with_access(Core::image_access_mode::Write);
//	5_image_idx = (*lll_low_detail_size)[0].with_access(Core::image_access_mode::Write);
//	6_image_idx = (*lll_heads)[0].with_access(Core::image_access_mode::Write);
//	7_image_idx = (*lll_low_detail_heads)[0].with_access(Core::image_access_mode::Write);
//	7_storage_idx = lll_counter;
//}
