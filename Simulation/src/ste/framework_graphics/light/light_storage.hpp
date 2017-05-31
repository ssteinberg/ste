//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <light.hpp>
#include <light_cascade_descriptor.hpp>

#include <directional_light.hpp>
#include <virtual_light.hpp>
#include <sphere_light.hpp>
#include <shaped_light.hpp>

#include <resource_storage_dynamic.hpp>
#include <gstack_stable.hpp>

#include <shader_storage_buffer.hpp>
#include <atomic_counter_buffer_object.hpp>
#include <uniform_buffer_object.hpp>

#include <Camera.hpp>

#include <array>
#include <lib/unique_ptr.hpp>
#include <type_traits>

namespace ste {
namespace graphics {

constexpr std::size_t max_active_lights_per_frame = 24;
constexpr std::size_t max_active_directional_lights_per_frame = 4;
constexpr std::size_t total_max_active_lights_per_frame = max_active_lights_per_frame + max_active_directional_lights_per_frame;

class light_storage : public Core::resource_storage_dynamic<light_descriptor> {
	using Base = Core::resource_storage_dynamic<light_descriptor>;

public:
	using shaped_light_point = shaped_light::shaped_light_point_type;

private:
	static constexpr Core::BufferUsage::buffer_usage usage = static_cast<Core::BufferUsage::buffer_usage>(Core::BufferUsage::BufferUsageSparse);
	static constexpr std::size_t pages = 1024;

	using lights_ll_type = Core::shader_storage_buffer<std::uint32_t, usage>;
	using directional_lights_cascades_type = Core::uniform_buffer_object<light_cascades_descriptor, Core::BufferUsage::BufferUsageDynamic>;
	using shaped_lights_points_storage_type = Core::gstack_stable<shaped_light_point>;

private:
	Core::atomic_counter_buffer_object<> active_lights_ll_counter;

	lights_ll_type active_lights_ll;
	directional_lights_cascades_type directional_lights_cascades_buffer;
	shaped_lights_points_storage_type shaped_lights_points_storage;

	std::array<directional_light*, directional_light_cascades> active_directional_lights;
	std::array<float, directional_light_cascades> cascades_depths;

private:
	void build_cascade_depth_array();

public:
	light_storage() : active_lights_ll_counter(1),
					  active_lights_ll(pages * std::max<std::size_t>(65536, lights_ll_type::page_size()) / 2),
					  directional_lights_cascades_buffer(max_active_directional_lights_per_frame) {
		build_cascade_depth_array();

		for (auto &val : active_directional_lights)
			val = nullptr;
	}

	template <typename ... Ts>
	auto allocate_virtual_light(Ts&&... args) {
		auto res = Base::allocate_resource<virtual_light>(std::forward<Ts>(args)...);
		active_lights_ll.commit_range(0, Base::size());

		return std::move(res);
	}

	template <typename ... Ts>
	auto allocate_sphere_light(Ts&&... args) {
		auto res = Base::allocate_resource<sphere_light>(std::forward<Ts>(args)...);
		active_lights_ll.commit_range(0, Base::size());

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
		active_lights_ll.commit_range(0, Base::size());

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
		active_lights_ll.commit_range(0, Base::size());

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

	void clear_active_ll() {
		std::uint32_t zero = 0;
		active_lights_ll_counter.clear(gli::FORMAT_R32_UINT_PACK32, &zero);
	}

	void update_directional_lights_cascades_buffer(const camera &cam, float projection_near, float projection_fovy, float projection_aspect);

	void bind_lights_buffer(int idx) const { Base::buffer().bind_range(Core::shader_storage_layout_binding(idx), 0, Base::size()); }
	auto& get_active_ll_counter() const { return active_lights_ll_counter; }
	auto& get_active_ll() const { return active_lights_ll; }

	auto& get_directional_lights_cascades_buffer() const { return directional_lights_cascades_buffer; }
	auto& get_shaped_lights_points_buffer() const { return shaped_lights_points_storage.get_buffer(); }

	auto& get_cascade_depths_array() const { return cascades_depths; }
};

}
}
