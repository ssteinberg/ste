
#include <stdafx.hpp>
#include <primary_renderer.hpp>
#include <host_read_buffer.hpp>
#include <random>

using namespace ste;
using namespace ste::graphics;

namespace ste::graphics::_detail {

template <typename F>
void primary_renderer_atom(gl::profiler::profiler *profiler,
						   gl::command_recorder &recorder,
						   lib::string name,
						   gl::pipeline_stage stages,
						   F &&f) {
	if (profiler) {
		auto atom = profiler->start_atom(recorder,
										 name,
										 stages);
		f();
	}
	else {
		f();
	}
}
template <typename F>
void primary_renderer_atom(gl::profiler::profiler *profiler,
						   gl::command_recorder &recorder,
						   lib::string name,
						   F &&f) {
	primary_renderer_atom(profiler, recorder, name, gl::pipeline_stage::bottom_of_pipe, std::forward<F>(f));
}

}

primary_renderer::primary_renderer(const ste_context &ctx,
								   gl::framebuffer_layout &&fb_layout,
								   const camera_t *cam,
								   scene *s,
								   const atmospherics_properties<double> &atmospherics_prop,
								   voxels_configuration voxel_config,
								   gl::profiler::profiler *profiler)
	: Base(ctx),
	profiler(profiler),

	cam(cam),
	s(s),

	buffers(ctx,
			ctx.device().get_surface().extent(),
			s,
			this->acquire_storage<atmospherics_lut_storage>(),
			atmospherics_prop,
			voxel_config),
	framebuffers(ctx,
				 ctx.device().get_surface().extent()),

	composer(ctx,
			 *this,
			 &buffers.voxels.get()),
	hdr(ctx,
		*this,
		ctx.device().get_surface().extent(),
		gl::framebuffer_layout(framebuffers.fxaa_input_fb.get_layout())),
	fxaa(ctx,
		 *this,
		 std::move(fb_layout)),

	voxelizer(ctx,
			  *this,
			  &buffers.voxels.get(),
			  this->s),

	downsample_depth(ctx,
					 *this,
					 &buffers.gbuffer.get()),
	prepopulate_backface_depth(ctx,
							   *this,
							   this->s),
	scene_write_gbuffer(ctx,
						*this,
						this->s, &buffers.gbuffer.get()),
	scene_geo_cull(ctx,
				   *this,
				   this->s, &this->s->properties().lights_storage()),

	linked_light_list_generator(ctx,
								*this,
								&buffers.linked_light_list_storage.get()),
	light_preprocess(ctx,
					 *this,
					 &this->s->properties().lights_storage(),
					 this->cam->get_projection_model())
{
	// Attach a connection to swapchain's surface resize signal
	resize_signal_connection = make_connection(ctx.device().get_queues_and_surface_recreate_signal(), [this, &ctx](auto) {
		// Resize buffers and framebuffers
		buffers.resize(ctx.device().get_surface().extent());
		framebuffers.resize(ctx.device().get_surface().extent());

		// Send resize signal to interested fragments
		hdr->resize(device().get_surface().extent());

		// Reattach resized framebuffers and input images
		reattach_framebuffers();
	});

	// Attach a connection to camera's projection change signal
	camera_projection_change_signal = make_connection(cam->get_projection_change_signal(), [this](auto, auto projection) {
		// Update buffers and fragments that rely on projection data
		buffers.invalidate_projection_buffer();

		light_preprocess->update_projection_planes(projection);
	});

	// Attach framebuffers
	reattach_framebuffers();
}

void primary_renderer::reattach_framebuffers() {
	// Attach gbuffer framebuffers
	prepopulate_backface_depth->attach_framebuffer(buffers.gbuffer->get_depth_backface_fbo());
	scene_write_gbuffer->attach_framebuffer(buffers.gbuffer->get_fbo());

	// Attach composer and hdr outputs
	composer->attach_framebuffer(framebuffers.hdr_input_fb);
	hdr->attach_framebuffer(framebuffers.fxaa_input_fb);
	hdr->set_input_image(&framebuffers.hdr_input_image.get());
	fxaa->set_input_image(&framebuffers.fxaa_input_image.get());
}

void primary_renderer::update(gl::command_recorder &recorder) {
	recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::vertex_shader | gl::pipeline_stage::fragment_shader | gl::pipeline_stage::compute_shader,
															  gl::pipeline_stage::transfer,
															  gl::buffer_memory_barrier(s->properties().lights_storage().buffer(),
																						gl::access_flags::shader_read,
																						gl::access_flags::transfer_write),
															  gl::buffer_memory_barrier(s->properties().materials_storage().buffer(),
																						gl::access_flags::shader_read,
																						gl::access_flags::transfer_write),
															  gl::buffer_memory_barrier(s->properties().material_layers_storage().buffer(),
																						gl::access_flags::shader_read,
																						gl::access_flags::transfer_write),
															  gl::buffer_memory_barrier(buffers.transform_buffers.get_view_buffer(),
																						gl::access_flags::shader_read,
																						gl::access_flags::transfer_write)));

	// Update scene, light storage and objects
	s->update_scene(recorder);

	// Update buffers
	buffers.update(recorder, cam);

	recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::transfer,
															  gl::pipeline_stage::vertex_shader | gl::pipeline_stage::fragment_shader | gl::pipeline_stage::compute_shader,
															  gl::buffer_memory_barrier(s->properties().lights_storage().buffer(),
																						gl::access_flags::transfer_write,
																						gl::access_flags::shader_read | gl::access_flags::shader_write),
															  gl::buffer_memory_barrier(s->properties().materials_storage().buffer(),
																						gl::access_flags::transfer_write,
																						gl::access_flags::shader_read),
															  gl::buffer_memory_barrier(s->properties().material_layers_storage().buffer(),
																						gl::access_flags::transfer_write,
																						gl::access_flags::shader_read),
															  gl::buffer_memory_barrier(buffers.transform_buffers.get_view_buffer(),
																						gl::access_flags::transfer_write,
																						gl::access_flags::shader_read)));

	// Update atmospheric properties (if needed)
	auto atmoshperics_update = atmospherics_properties_update.get();
	if (atmoshperics_update) {
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::fragment_shader | gl::pipeline_stage::compute_shader,
																  gl::pipeline_stage::transfer,
																  gl::buffer_memory_barrier(buffers.atmospheric_buffer.get(),
																							gl::access_flags::shader_read,
																							gl::access_flags::transfer_write)));
		buffers.update_atmospheric_properties(recorder,
											  atmoshperics_update.get());
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::transfer,
																  gl::pipeline_stage::fragment_shader | gl::pipeline_stage::compute_shader,
																  gl::buffer_memory_barrier(buffers.atmospheric_buffer.get(),
																							gl::access_flags::transfer_write,
																							gl::access_flags::shader_read)));
	}

	// Update common binding set
	buffers.update_common_binding_set(s);
}

void primary_renderer::render(gl::command_recorder &recorder) {
	// Update data
	_detail::primary_renderer_atom(profiler, recorder, "update", 
								   [this, &recorder]() {
		update(recorder);
	});

	// Render

	_detail::primary_renderer_atom(profiler, recorder, "-> preprocess_light",
								   [this, &recorder]() {
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader | gl::pipeline_stage::fragment_shader,
																  gl::pipeline_stage::compute_shader,
																  gl::buffer_memory_barrier(s->properties().lights_storage().get_active_ll(),
																							gl::access_flags::shader_read,
																							gl::access_flags::shader_read | gl::access_flags::shader_write)));
	});

	// Light preprocess
	record_light_preprocess_fragment(recorder);

	_detail::primary_renderer_atom(profiler, recorder, "-> geo_cull",
								   [this, &recorder]() {
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader,
																  gl::pipeline_stage::compute_shader,
																  gl::buffer_memory_barrier(s->properties().lights_storage().get_active_ll(),
																							gl::access_flags::shader_read | gl::access_flags::shader_write,
																							gl::access_flags::shader_read),
																  gl::buffer_memory_barrier(s->properties().lights_storage().get_active_ll_counter(),
																							gl::access_flags::shader_read | gl::access_flags::shader_write,
																							gl::access_flags::shader_read),
																  gl::buffer_memory_barrier(s->properties().lights_storage().buffer(),
																							gl::access_flags::shader_read | gl::access_flags::shader_write,
																							gl::access_flags::shader_read)));
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::draw_indirect,
																  gl::pipeline_stage::compute_shader,
																  gl::buffer_memory_barrier(s->get_idb().get(),
																							gl::access_flags::indirect_command_read,
																							gl::access_flags::shader_write)));
	});

	// Scene geometry cull
	record_scene_geometry_cull_fragment(recorder);

	_detail::primary_renderer_atom(profiler, recorder, "-> scene",
								   [this, &recorder]() {
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader,
																  gl::pipeline_stage::draw_indirect,
																  gl::buffer_memory_barrier(s->get_idb().get(),
																							gl::access_flags::shader_write,
																							gl::access_flags::indirect_command_read)));
	});

	// Draw scene to gbuffer
	record_scene_fragment(recorder);

	// Voxelize scene
	{
		// TODO: Dynamic voxelization for dynamic parts of the scene
		static bool voxelized = false;
		if (!voxelized)
			record_voxelizer_fragment(recorder);
		voxelized = true;
	}

	_detail::primary_renderer_atom(profiler, recorder, "-> downsample_depth",
								   [this, &recorder]() {
		// TODO: Event
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader,
																  gl::pipeline_stage::compute_shader,
																  gl::image_memory_barrier(buffers.gbuffer.get().get_downsampled_depth_target().get_image(),
																						   gl::image_layout::shader_read_only_optimal,
																						   gl::image_layout::general,
																						   gl::access_flags::shader_read,
																						   gl::access_flags::shader_write)));
	});

	// Downsample depth
	record_downsample_depth_fragment(recorder);

	_detail::primary_renderer_atom(profiler, recorder, "-> lll",
								   [this, &recorder]() {
		// TODO: Event
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader | gl::pipeline_stage::fragment_shader,
																  gl::pipeline_stage::compute_shader,
																  gl::image_memory_barrier(buffers.gbuffer.get().get_downsampled_depth_target().get_image(),
																						   gl::image_layout::general,
																						   gl::image_layout::shader_read_only_optimal,
																						   gl::access_flags::shader_write,
																						   gl::access_flags::shader_read),
																  gl::buffer_memory_barrier(buffers.linked_light_list_storage.get().linked_light_lists_buffer(),
																							gl::access_flags::shader_read,
																							gl::access_flags::shader_write),
																  gl::image_memory_barrier(buffers.linked_light_list_storage.get().linked_light_lists_heads_map().get_image(),
																						   gl::image_layout::general,
																						   gl::image_layout::general,
																						   gl::access_flags::shader_read,
																						   gl::access_flags::shader_read | gl::access_flags::shader_write),
																  gl::image_memory_barrier(buffers.linked_light_list_storage.get().linked_light_lists_size_map().get_image(),
																						   gl::image_layout::general,
																						   gl::image_layout::general,
																						   gl::access_flags::shader_read,
																						   gl::access_flags::shader_read | gl::access_flags::shader_write)));
	});

	// Linked-light-list generator
	record_linked_light_list_generator_fragment(recorder);

	// Prepopulate back-face depth buffer
	record_prepopulate_depth_backface_fragment(recorder);

	_detail::primary_renderer_atom(profiler, recorder, "-> deferred",
								   [this, &recorder]() {
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader,
																  gl::pipeline_stage::compute_shader | gl::pipeline_stage::fragment_shader,
																  gl::buffer_memory_barrier(buffers.linked_light_list_storage.get().linked_light_lists_buffer(),
																							gl::access_flags::shader_read,
																							gl::access_flags::shader_write)));
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader,
																  gl::pipeline_stage::compute_shader | gl::pipeline_stage::fragment_shader,
																  gl::image_memory_barrier(buffers.linked_light_list_storage.get().linked_light_lists_heads_map().get_image(),
																						   gl::image_layout::general,
																						   gl::image_layout::general,
																						   gl::access_flags::shader_write,
																						   gl::access_flags::shader_read),
																  gl::image_memory_barrier(buffers.linked_light_list_storage.get().linked_light_lists_size_map().get_image(),
																						   gl::image_layout::general,
																						   gl::image_layout::general,
																						   gl::access_flags::shader_write,
																						   gl::access_flags::shader_read)));

		// TODO: Event
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader,
																  gl::pipeline_stage::fragment_shader,
																  gl::buffer_memory_barrier(buffers.voxels->voxels_buffer(),
																							gl::access_flags::shader_write,
																							gl::access_flags::shader_read)));
	});

	// Deferred compose
	record_deferred_composer_fragment(recorder);

	// Post-process, HDR tonemapping and FXAA
	_detail::primary_renderer_atom(profiler, recorder, "hdr",
								   [this, &recorder]() {
		recorder << hdr.get();
	});
	_detail::primary_renderer_atom(profiler, recorder, "fxaa",
								   [this, &recorder]() {
		recorder << fxaa.get();
	});

	if (profiler)
		profiler->end_segment();
}

void primary_renderer::record_light_preprocess_fragment(gl::command_recorder &recorder) {
	_detail::primary_renderer_atom(profiler, recorder, "clear ll",
								   [this, &recorder]() {
		// Clear ll counter
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader | gl::pipeline_stage::fragment_shader,
																  gl::pipeline_stage::transfer,
																  gl::buffer_memory_barrier(s->properties().lights_storage().get_active_ll_counter(),
																							gl::access_flags::shader_read,
																							gl::access_flags::transfer_write)));
		s->properties().lights_storage().clear_active_ll(recorder);
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::transfer,
																  gl::pipeline_stage::compute_shader,
																  gl::buffer_memory_barrier(s->properties().lights_storage().get_active_ll_counter(),
																							gl::access_flags::transfer_write,
																							gl::access_flags::shader_read | gl::access_flags::shader_write)));
	});

	// Preprocess lights
	_detail::primary_renderer_atom(profiler, recorder, "preprocess_light",
								   [this, &recorder]() {
		recorder << light_preprocess.get();
	});
}

void primary_renderer::record_scene_geometry_cull_fragment(gl::command_recorder &recorder) {
	_detail::primary_renderer_atom(profiler, recorder, "geo_cull",
								   [this, &recorder]() {
		recorder << scene_geo_cull.get();
	});
}

void primary_renderer::record_voxelizer_fragment(gl::command_recorder &recorder) {
	_detail::primary_renderer_atom(profiler, recorder, "clear voxels",
								   [this, &recorder]() {
		// Clear voxel buffer
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader | gl::pipeline_stage::fragment_shader,
																  gl::pipeline_stage::transfer,
																  gl::buffer_memory_barrier(buffers.voxels->voxels_buffer(),
																							gl::access_flags::shader_read,
																							gl::access_flags::transfer_write),
																  gl::buffer_memory_barrier(buffers.voxels->voxels_counter_buffer(),
																							gl::access_flags::shader_read,
																							gl::access_flags::transfer_write),
																  gl::buffer_memory_barrier(buffers.voxels->voxel_list_counter_buffer(),
																							gl::access_flags::shader_read,
																							gl::access_flags::transfer_write)));
		buffers.voxels.get().clear(recorder);
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::transfer,
																  gl::pipeline_stage::compute_shader,
																  gl::buffer_memory_barrier(buffers.voxels->voxels_buffer(),
																							gl::access_flags::transfer_write,
																							gl::access_flags::shader_read | gl::access_flags::shader_write),
																  gl::buffer_memory_barrier(buffers.voxels->voxels_counter_buffer(),
																							gl::access_flags::transfer_write,
																							gl::access_flags::shader_read | gl::access_flags::shader_write)));
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::transfer,
																  gl::pipeline_stage::fragment_shader,
																  gl::buffer_memory_barrier(buffers.voxels->voxel_list_counter_buffer(),
																							gl::access_flags::transfer_write,
																							gl::access_flags::shader_read | gl::access_flags::shader_write)));
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader | gl::pipeline_stage::fragment_shader,
																  gl::pipeline_stage::fragment_shader,
																  gl::buffer_memory_barrier(buffers.voxels->voxel_list_buffer(),
																							gl::access_flags::shader_read,
																							gl::access_flags::shader_write)));
	});

	// Voxelize
	_detail::primary_renderer_atom(profiler, recorder, "voxelizer",
									[this, &recorder]() {
		recorder << voxelizer.get();
	});

	recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::fragment_shader | gl::pipeline_stage::compute_shader,
															  gl::pipeline_stage::transfer,
															  gl::buffer_memory_barrier(buffers.voxels->voxels_buffer(),
																						gl::access_flags::shader_write,
																						gl::access_flags::transfer_read),
															  gl::buffer_memory_barrier(buffers.voxels->voxels_counter_buffer(),
																						gl::access_flags::shader_write,
																						gl::access_flags::transfer_read),
															  gl::buffer_memory_barrier(buffers.voxels->voxel_list_buffer(),
																						gl::access_flags::shader_write,
																						gl::access_flags::transfer_read),
															  gl::buffer_memory_barrier(buffers.voxels->voxel_list_counter_buffer(),
																						gl::access_flags::shader_write,
																						gl::access_flags::transfer_read)));
}

namespace voxels {

using namespace glm;

// (2^Pi)^3 voxels per initial block
static constexpr uint voxel_Pi = 4;
// (2^P)^3 voxels per block
static constexpr uint voxel_P = 2;
// Voxel structure end level index
static constexpr uint voxel_leaf_level = 4;

// Voxel world extent
static constexpr float voxel_world = 3500;


uint *voxel_buffer;
uint voxel_buffer_size;


const uint voxel_lock_pattern = 0xFFFFFFFF;
const uint voxel_root_node = 0;


// Size of voxel block
float voxel_tree_block_extent = 1 << voxel_P;
float voxel_tree_initial_block_extent = 1 << voxel_Pi;
// Resolution of maximal voxel level
float voxel_grid_resolution = voxel_world / ((1 << voxel_Pi) * (1 << voxel_P * (voxel_leaf_level - 1)));

const uint voxel_tree_node_data_size = 0;
const uint voxel_tree_leaf_data_size = 8;

/**
*	@brief	Computes resolution of a given voxel level
*/
uint voxel_resolution(uint level) {
	uint p = voxel_Pi + voxel_P * level;
	return 1 << p;
}

/**
*	@brief	Computes resolution difference between given voxel levels.
*			Expects: level1 >= level0
*/
uint voxel_resolution_difference(uint level0, uint level1) {
	uint p = voxel_P * (level1 - level0);
	return 1 << p;
}

/**
*	@brief	Returns block extent for given voxel level
*/
float voxel_block_extent(uint level) {
	return mix(voxel_tree_block_extent,
			   voxel_tree_initial_block_extent,
			   level == 0);
}

/**
*	@brief	Returns block power (log2 of extent) for given voxel level
*/
uint voxel_block_power(uint level) {
	return mix(voxel_P,
			   voxel_Pi,
			   level == 0);
}

/**
*	@brief	Calculates index of a brick in a block
*/
uint voxel_brick_index(uvec3 brick, uint P) {
	return brick.x + (((brick.z << P) + brick.y) << P);
}

/**
*	@brief	Calculates the address of a brick in the binary map.
*			Returns (word, bit) vector, where word is the 32-bit word index, and bit is the bit index in that word.
*/
uvec2 voxel_binary_map_address(uint brick_index) {
	uint word = brick_index / 32;
	uint bit = brick_index % 32;

	return uvec2(word, bit);
}

/**
*	@brief	Returns the count of children in a node
*/
uint voxel_node_children_count(uint P) {
	return 1 << (3 * P);
}

/**
*	@brief	Returns the binary map size of a node
*/
uint voxel_binary_map_size(uint P) {
	uint map_bits = voxel_node_children_count(P);
	uint map_bytes = map_bits >> 3;
	return map_bytes >> 2;
}
/**
*	@brief	Returns the offset of the binary map in a voxel node.
*			Meaningless for leaf nodes.
*/
uint voxel_node_binary_map_offset(uint P) {
	return 0;
}

/**
*	@brief	Returns the offset of the children data in a voxel node.
*			Meaningless for leaf nodes.
*/
uint voxel_node_children_offset(uint P) {
	return voxel_binary_map_size(P);
}

/**
*	@brief	Returns the offset of the custom data in a voxel node.
*			Meaningless for leaf nodes.
*/
uint voxel_node_data_offset(uint P) {
	return voxel_binary_map_size(P) + voxel_node_children_count(P);
}

/**
*	@brief	Returns a single child size in a voxel node.
*			level must be > 0.
*/
uint voxel_node_size(uint level) {
	uint P = voxel_block_power(level);
	return mix(voxel_node_data_offset(P) + voxel_tree_node_data_size / 4,
			   voxel_tree_leaf_data_size >> 2,
			   level == voxel_leaf_level);
}

template <typename T>
void check(const T &node, unsigned level = 0) {
	using node_t = voxels_configuration::tree_node_t<voxel_P>;

	if (level == 3)
		return;

	auto root_size = ((1 << 3 * (voxel_Pi - 1)) * 33 + voxel_tree_node_data_size) / 4;

	for (int i=0;i<T::bricks_count;++i) {
		auto map_bit = 1ull << (i % 32);
		if ((node.binary_map[i / 32] & map_bit) != 0) {
			assert(node.children[i] >= root_size);
			assert(node.children[i] < voxel_buffer_size);
			check(*reinterpret_cast<const node_t*>(&voxel_buffer[node.children[i]]), level + 1);
		}
	}
}

}

void primary_renderer::d() {
	auto list_count = gl::host_read_buffer(get_creating_context(), buffers.voxels.get().voxel_list_counter_buffer()).get();
	auto count = gl::host_read_buffer(get_creating_context(), buffers.voxels.get().voxels_counter_buffer()).get();
	auto voxel_list = gl::host_read_buffer(get_creating_context(), buffers.voxels.get().voxel_list_buffer(), list_count[0].get<0>()).get();
	auto voxels = gl::host_read_buffer(get_creating_context(), buffers.voxels.get().voxels_buffer(), count[0].get<0>()).get();

	voxels::voxel_buffer = reinterpret_cast<glm::uint*>(voxels.data());
	voxels::voxel_buffer_size = count[0].get<0>();

	const auto *root = reinterpret_cast<const voxels_configuration::tree_root_t<voxels::voxel_Pi>*>(voxels.data());
	auto get_child = [&](int c) {return *reinterpret_cast<const voxels_configuration::tree_node_t<2>*>(&voxels[root->children[c]]); };

	voxels::check(*root);

	/*for (auto i = 0u; i < list_count[0].get<0>();++i) {
		auto &l = voxel_list[i];
		auto node = l.get<4>();
		assert(voxels[node].get<0>() != 0);
		assert(glm::abs(l.get<0>()) < 1.f);
		assert(glm::abs(l.get<1>()) < 1.f);
		assert(glm::abs(l.get<2>()) < 1.f);
	}*/

	bool bbbbb = false;
}

void primary_renderer::record_downsample_depth_fragment(gl::command_recorder &recorder) {
	_detail::primary_renderer_atom(profiler, recorder, "downsample_depth",
								   [this, &recorder]() {
		recorder << downsample_depth.get();
	});
}

void primary_renderer::record_linked_light_list_generator_fragment(gl::command_recorder &recorder) {
	_detail::primary_renderer_atom(profiler, recorder, "clear lll",
									[this, &recorder]() {
		// Clear lll counter
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader | gl::pipeline_stage::fragment_shader,
																	gl::pipeline_stage::transfer,
																	gl::buffer_memory_barrier(buffers.linked_light_list_storage.get().linked_light_lists_counter_buffer(),
																							gl::access_flags::shader_read,
																							gl::access_flags::transfer_write)));
		buffers.linked_light_list_storage.get().clear(recorder);
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::transfer,
																	gl::pipeline_stage::compute_shader | gl::pipeline_stage::fragment_shader,
																	gl::buffer_memory_barrier(buffers.linked_light_list_storage.get().linked_light_lists_counter_buffer(),
																							gl::access_flags::transfer_write,
																							gl::access_flags::shader_read | gl::access_flags::shader_write)));
	});

	// Preprocess lights
	_detail::primary_renderer_atom(profiler, recorder, "lll",
									[this, &recorder]() {
		recorder << linked_light_list_generator.get();
	});
}

void primary_renderer::record_scene_fragment(gl::command_recorder &recorder) {
	_detail::primary_renderer_atom(profiler, recorder, "gbuffer",
								   [this, &recorder]() {
		recorder << scene_write_gbuffer.get();
	});
}

void primary_renderer::record_prepopulate_depth_backface_fragment(gl::command_recorder &recorder) {
	_detail::primary_renderer_atom(profiler, recorder, "backface_depth",
								   [this, &recorder]() {
		recorder << prepopulate_backface_depth.get();
	});
}

void primary_renderer::record_deferred_composer_fragment(gl::command_recorder &recorder) {
	_detail::primary_renderer_atom(profiler, recorder, "deferred",
								   [this, &recorder]() {
		recorder << composer.get();
	});
}
