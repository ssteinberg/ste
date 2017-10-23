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
#include <std430.hpp>
#include <std140.hpp>

#include <array>
#include <lib/unique_ptr.hpp>
#include <type_traits>
#include <atomic>

namespace ste {
namespace graphics {

constexpr std::size_t max_active_lights_per_frame = 24;
constexpr std::size_t max_active_directional_lights_per_frame = 4;
constexpr std::size_t total_max_active_lights_per_frame = max_active_lights_per_frame + max_active_directional_lights_per_frame;

class light_storage : public gl::resource_storage_dynamic<light_descriptor::buffer_data> {
	using Base = gl::resource_storage_dynamic<light_descriptor::buffer_data>;

public:
	using shaped_light_point = shaped_light::shaped_light_point_type;

	static constexpr std::size_t max_ll_buffer_size = 64 * 1024 * 1024;

private:
	using lights_ll_type = gl::stable_vector<gl::std430<std::uint32_t>, max_ll_buffer_size>;
	using shaped_lights_points_storage_type = gl::stable_vector<shaped_light_point>;

private:
	alias<const ste_context> ctx;

	gl::array<gl::std430<std::uint32_t>> active_lights_ll_counter;

	lights_ll_type active_lights_ll;
	shaped_lights_points_storage_type shaped_lights_points_storage;

	std::array<std::atomic<const directional_light*>, directional_light_cascades> active_directional_lights;

	std::atomic_flag active_lights_ll_resize{ ATOMIC_FLAG_INIT };

private:
	static std::array<gl::std140<float>, directional_light_cascades> cascades_depths_uniform_buffer_initial_data(const std::array<float, directional_light_cascades> &cascades_depths) {
		std::array<gl::std140<float>, directional_light_cascades> initial_data;
		for (int i=0;i<directional_light_cascades;++i)
			initial_data[i].get<0>() = cascades_depths[i];

		return initial_data;
	}

public:
	light_storage(const ste_context &ctx)
		: Base(ctx, 
			   gl::buffer_usage::storage_buffer,
			   "light_storage"),
		ctx(ctx),
		active_lights_ll_counter(ctx, 
								 1, 
								 gl::buffer_usage::storage_buffer,
								 "active_lights_ll_counter"),
		active_lights_ll(ctx,
						 gl::buffer_usage::storage_buffer,
						 "active_lights_ll"),
		shaped_lights_points_storage(ctx, 
									 gl::buffer_usage::storage_buffer,
									 "shaped_lights_points_storage")
	{
		// Initialize array of active directional lights to nulls
		for (auto &val : active_directional_lights)
			val.store(nullptr);
	}

	template <typename ... Ts>
	auto allocate_virtual_light(Ts&&... args) {
		auto res = Base::allocate_resource<virtual_light>(std::forward<Ts>(args)...);
		active_lights_ll_resize.clear(std::memory_order_release);

		return std::move(res);
	}

	template <typename ... Ts>
	auto allocate_sphere_light(Ts&&... args) {
		auto res = Base::allocate_resource<sphere_light>(std::forward<Ts>(args)...);
		active_lights_ll_resize.clear(std::memory_order_release);

		return std::move(res);
	}

	template <typename ... Ts>
	auto allocate_directional_light(Ts&&... args) {
		// Allocate a directional light resource
		auto res = Base::allocate_resource<directional_light>(std::forward<Ts>(args)...);

		// Find an empty slot in active directional lights array
		int cascade_idx;
		for (cascade_idx = 0; cascade_idx < active_directional_lights.size(); ++cascade_idx) {
			// Try to take ownership of slot
			const directional_light *expected = nullptr;
			if (active_directional_lights[cascade_idx].compare_exchange_strong(expected,
																			   res.get()))
				break;
		}
		if (cascade_idx == max_active_directional_lights_per_frame) {
			assert(false && "Can not create any more directional lights");
			return lib::unique_ptr<directional_light>(nullptr);
		}

		active_lights_ll_resize.clear(std::memory_order_release);

		// Write light's slot
		res->set_cascade_idx(cascade_idx);

		return std::move(res);
	}

	template <typename Light, typename ... Ts>
	auto allocate_shaped_light(Ts&&... args) {
		static_assert(std::is_base_of<shaped_light, Light>::value, "Light must be a shaped_light derived type");

		// For shaped light need to pass pointer to points storage
		shaped_light::shaped_light_points_storage_info info{ &shaped_lights_points_storage };

		auto res = Base::allocate_resource<Light>(std::forward<Ts>(args)..., info);
		active_lights_ll_resize.clear(std::memory_order_release);

		return std::move(res);
	}

	virtual void erase_resource(const Base::resource_type *res) override {
		// Find slot owned by light, if any, and mark it free.
		for (auto &slot : active_directional_lights) {
			const directional_light* expected = reinterpret_cast<const directional_light*>(res);
			if (slot.compare_exchange_strong(expected,
											 nullptr))
				break;
		}

		// Deallocate light resource
		Base::erase_resource(res);
	}

	void erase_light(const light *l) {
		erase_resource(l);
	}

	void clear_active_ll(gl::command_recorder &recorder) {
		const gl::std430<std::uint32_t> zero = std::make_tuple<std::uint32_t>(0);
		recorder << active_lights_ll_counter.overwrite_cmd(0, zero);
	}

	void update(gl::command_recorder &recorder) override final {
		if (!active_lights_ll_resize.test_and_set(std::memory_order_acquire))
			recorder << active_lights_ll.resize_cmd(ctx.get(),
													Base::size());

		Base::update(recorder);
	}

	auto& get_active_ll_counter() const { return active_lights_ll_counter; }
	auto& get_active_ll() const { return active_lights_ll; }

	auto& get_shaped_lights_points_buffer() const { return shaped_lights_points_storage; }
};

}
}
