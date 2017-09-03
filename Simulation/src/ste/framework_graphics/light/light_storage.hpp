//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <storage.hpp>

#include <light.hpp>
#include <light_cascade_descriptor.hpp>
#include <camera.hpp>

#include <directional_light.hpp>
#include <virtual_light.hpp>
#include <sphere_light.hpp>
#include <shaped_light.hpp>

#include <resource_storage_dynamic.hpp>
#include <array.hpp>
#include <stable_vector.hpp>

#include <array>
#include <lib/unique_ptr.hpp>
#include <type_traits>
#include <optional.hpp>

namespace ste {
namespace graphics {

constexpr std::size_t max_active_lights_per_frame = 24;
constexpr std::size_t max_active_directional_lights_per_frame = 4;
constexpr std::size_t total_max_active_lights_per_frame = max_active_lights_per_frame + max_active_directional_lights_per_frame;

class light_storage : public gl::resource_storage_dynamic<light_descriptor> {
	using Base = gl::resource_storage_dynamic<light_descriptor>;

public:
	using shaped_light_point = shaped_light::shaped_light_point_type;

	static constexpr std::size_t max_ll_buffer_size = 64 * 1024 * 1024;

private:
	using lights_ll_type = gl::device_buffer_sparse<std::uint32_t>;
	using directional_lights_cascades_type = gl::array<light_cascades_descriptor>;
	using shaped_lights_points_storage_type = gl::stable_vector<shaped_light_point>;

private:
	gl::array<std::uint32_t> active_lights_ll_counter;

	lights_ll_type active_lights_ll;
	directional_lights_cascades_type directional_lights_cascades_buffer;
	shaped_lights_points_storage_type shaped_lights_points_storage;

	std::array<directional_light*, directional_light_cascades> active_directional_lights;
	std::array<float, directional_light_cascades> cascades_depths;

	optional<std::uint64_t> active_lights_ll_resize;

private:
	void build_cascade_depth_array();

public:
	light_storage(const ste_context &ctx)
		: Base(ctx, gl::buffer_usage::storage_buffer),
		active_lights_ll_counter(ctx, 1, gl::buffer_usage::storage_buffer),
		active_lights_ll(ctx, max_ll_buffer_size, gl::buffer_usage::storage_buffer),
		directional_lights_cascades_buffer(ctx, max_active_directional_lights_per_frame, gl::buffer_usage::storage_buffer),
		shaped_lights_points_storage(ctx, gl::buffer_usage::storage_buffer)
	{
		build_cascade_depth_array();

		for (auto &val : active_directional_lights)
			val = nullptr;
	}

	template <typename ... Ts>
	auto allocate_virtual_light(Ts&&... args) {
		auto res = Base::allocate_resource<virtual_light>(std::forward<Ts>(args)...);
		active_lights_ll_resize = Base::size();

		return std::move(res);
	}

	template <typename ... Ts>
	auto allocate_sphere_light(Ts&&... args) {
		auto res = Base::allocate_resource<sphere_light>(std::forward<Ts>(args)...);
		active_lights_ll_resize = Base::size();

		return std::move(res);
	}

	template <typename ... Ts>
	auto allocate_directional_light(Ts&&... args) {
		int cascade_idx;
		for (cascade_idx = 0; cascade_idx < active_directional_lights.size() && active_directional_lights[cascade_idx] != nullptr; ++cascade_idx) {}
		if (cascade_idx == max_active_directional_lights_per_frame) {
			assert(false && "Can not create any more directional lights");
			return lib::unique_ptr<directional_light>(nullptr);
		}

		auto res = Base::allocate_resource<directional_light>(std::forward<Ts>(args)...);
		active_lights_ll_resize = Base::size();

		active_directional_lights[cascade_idx] = res.get();
		res->set_cascade_idx(cascade_idx);

		return std::move(res);
	}

	template <typename Light, typename ... Ts>
	auto allocate_shaped_light(Ts&&... args) {
		static_assert(std::is_base_of<shaped_light, Light>::value, "Light must be a shaped_light derived type");

		// For shaped light need to pass pointer to points storage
		shaped_light::shaped_light_points_storage_info info{ &shaped_lights_points_storage };

		auto res = Base::allocate_resource<Light>(std::forward<Ts>(args)..., info);
		active_lights_ll_resize = Base::size();

		return std::move(res);
	}

	virtual void erase_resource(const Base::resource_type *res) override {
		for (auto &val : active_directional_lights)
			if (val == res) {
				val = nullptr;
				break;
			}
	
		Base::erase_resource(res);
	}

	void erase_light(const light *l) {
		erase_resource(l);
	}

	void clear_active_ll(gl::command_recorder &recorder) {
		std::uint32_t zero = 0;
		recorder << active_lights_ll_counter.overwrite_cmd(0, zero);
	}

	void update(gl::command_recorder &recorder) override final {
		if (active_lights_ll_resize) {
			recorder << active_lights_ll.cmd_bind_sparse_memory({}, { range<std::uint64_t>(0, active_lights_ll_resize.get()) }, {}, {});
			active_lights_ll_resize = none;
		}

		Base::update(recorder);
	}

	/**
	 *	@brief		Updates the directional lights cascades.
	 *				Should be called every frame where a view transform or a projection change occured.
	 *				
	 *	@param	recorder		Command recorder
	 *	@param	view_transform	Camera's view transform dual-quaternion
	 */
	void update_directional_lights_cascades_buffer(gl::command_recorder &recorder,
												   const glm::dualquat &view_transform,
												   float projection_near, 
												   float projection_fovy, 
												   float projection_aspect);

	auto& get_active_ll_counter() const { return active_lights_ll_counter; }
	auto& get_active_ll() const { return active_lights_ll; }

	auto& get_directional_lights_cascades_buffer() const { return directional_lights_cascades_buffer; }
	auto& get_shaped_lights_points_buffer() const { return shaped_lights_points_storage; }

	auto& get_cascade_depths_array() const { return cascades_depths; }
};

}
}
