// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "light.hpp"

#include "resource_storage_dynamic.hpp"

#include "ShaderStorageBuffer.hpp"
#include "AtomicCounterBufferObject.hpp"

#include <vector>
#include <memory>
#include <functional>

namespace StE {
namespace Graphics {

constexpr std::size_t max_active_lights_per_frame = 32;

class light_storage : public Core::resource_storage_dynamic<light_descriptor> {
	using Base = Core::resource_storage_dynamic<light_descriptor>;

private:
	static constexpr Core::BufferUsage::buffer_usage usage = static_cast<Core::BufferUsage::buffer_usage>(Core::BufferUsage::BufferUsageSparse);
	static constexpr std::size_t pages = 1024;

	using lights_ll_type = Core::ShaderStorageBuffer<std::uint16_t, usage>;

private:
	std::vector<std::unique_ptr<light>> lights;

	lights_ll_type active_lights_ll;
	Core::AtomicCounterBufferObject<> active_lights_ll_counter;

public:
	light_storage() : active_lights_ll(pages * std::max(65536, lights_ll_type::page_size()) / 2),
					  active_lights_ll_counter(1) {}

	template <typename LightType, typename ... Ts>
	LightType* allocate_light(Ts&&... args) {
		auto res = Base::allocate_resource<LightType>(std::forward<Ts>(args)...);
		auto ptr = res.get();
		lights.push_back(std::move(res));

		active_lights_ll.commit_range(0, size());

		return ptr;
	}
	void erase_light(const light *l) {
		erase_resource(l);
	}

	void clear_active_ll() {
		std::uint32_t zero = 0;
		active_lights_ll_counter.clear(gli::FORMAT_R32_UINT_PACK32, &zero);
	}

	void bind_lights_buffer(int idx) const { Base::buffer().bind_range(Core::shader_storage_layout_binding(idx), 0, lights.size()); }
	auto& get_active_ll_counter() const { return active_lights_ll_counter; }
	auto& get_active_ll() const { return active_lights_ll; }

	std::size_t size() const { return lights.size(); }
	auto& get_lights() const { return lights; }
};

}
}
