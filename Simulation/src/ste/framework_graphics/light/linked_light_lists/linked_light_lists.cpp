
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
	  extent(extent),
	  lll(ctx,
		  get_linked_light_list_required_size(),
		  gl::buffer_usage::storage_buffer,
		  "lll"),
	  lll_counter(ctx,
				  1,
				  gl::buffer_usage::storage_buffer,
				  "lll_counter"),

	  lll_heads(ctx,
				resource::surface_factory::image_empty_2d<gl::format::r32_sfloat>(ctx,
																				  gl::image_usage::storage,
																				  gl::image_layout::general,
																				  "lll_heads",
																				  extent)),
	  lll_size(ctx,
			   resource::surface_factory::image_empty_2d<gl::format::r8_uint>(ctx,
																			  gl::image_usage::storage,
																			  gl::image_layout::general,
																			  "lll_size",
																			  extent)) {}

std::size_t linked_light_lists::get_linked_light_list_required_size() const {
	return
		static_cast<std::size_t>(extent.x) *
		static_cast<std::size_t>(extent.y) *
		static_cast<std::size_t>(total_max_active_lights_per_frame);
}

void linked_light_lists::resize(const glm::uvec2 &extent) {
	const auto t = extent / lll_image_res_multiplier;
	if (t.x <= 0 || t.y <= 0 || t == this->extent)
		return;

	this->extent = t;
	std::atomic_thread_fence(std::memory_order_release);

	// Resize images
	lll_heads = ste_resource<gl::texture<gl::image_type::image_2d>>(ctx,
																	resource::surface_factory::image_empty_2d<gl::format::r32_sfloat>(ctx,
																																	  gl::image_usage::storage,
																																	  gl::image_layout::general,
																																	  "lll_heads",
																																	  extent));
	lll_size = ste_resource<gl::texture<gl::image_type::image_2d>>(ctx,
																   resource::surface_factory::image_empty_2d<gl::format::r8_uint>(ctx,
																																  gl::image_usage::storage,
																																  gl::image_layout::general,
																																  "lll_size",
																																  extent));

	// Resize LLL buffer
	lll = gl::array<lll_element>(ctx,
								 get_linked_light_list_required_size(),
								 gl::buffer_usage::storage_buffer,
								 "lll");

	// Notify storage was modified
	resize_signal.emit();
}
