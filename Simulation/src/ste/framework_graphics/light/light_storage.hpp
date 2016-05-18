// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "light.hpp"

#include "gstack.hpp"

#include "ShaderStorageBuffer.hpp"
#include "AtomicCounterBufferObject.hpp"

#include "range.hpp"

#include <vector>
#include <memory>

namespace StE {
namespace Graphics {

constexpr std::size_t max_active_lights_per_frame = 32;

class light_storage {
private:
	static constexpr Core::BufferUsage::buffer_usage usage = static_cast<Core::BufferUsage::buffer_usage>(Core::BufferUsage::BufferUsageSparse);
	static constexpr std::size_t pages = 1024;

	using lights_ll_type = Core::ShaderStorageBuffer<std::uint16_t, usage>;

private:
	std::vector<std::shared_ptr<light>> lights;
	Core::gstack<light::light_descriptor> stack;

	lights_ll_type active_lights_ll;
	Core::AtomicCounterBufferObject<> active_lights_ll_counter;

	mutable std::vector<range<>> ranges_to_lock;

public:
	light_storage() : active_lights_ll(pages * std::max(65536, lights_ll_type::page_size()) / 2),
					  active_lights_ll_counter(1) {}

	void update_storage() {
		auto s = sizeof(decltype(stack)::value_type);

		stack.get_buffer().client_wait_for_range({ 0, stack.size() * s });

		for (unsigned i = 0; i < lights.size();++i) {
			if (lights[i]->is_dirty()) {
				lights[i]->clear_dirty();
				stack.overwrite(i, lights[i]->get_descriptor());
				ranges_to_lock.push_back({ s * i, s });
			}
		}
	}

	void lock_ranges() {
		for (auto &r : ranges_to_lock)
			stack.lock_range(r);
		ranges_to_lock.clear();
	}

	void add_light(const std::shared_ptr<light> &l) {
		lights.push_back(l);
		stack.push_back(l->get_descriptor());
		l->clear_dirty();

		active_lights_ll.commit_range(0, lights.size());
	}

	void add_lights(const std::vector<std::shared_ptr<light>> &ls) {
		for (auto &l : ls)
			add_light(l);
	}

	void clear_active_ll() {
		std::uint32_t zero = 0;
		active_lights_ll_counter.clear(gli::FORMAT_R32_UINT_PACK32, &zero);
	}

	void bind_lights_buffer(int idx) const { stack.get_buffer().bind_range(Core::shader_storage_layout_binding(idx), 0, lights.size()); }
	auto& get_active_ll_counter() const { return active_lights_ll_counter; }
	auto& get_active_ll() const { return active_lights_ll; }

	auto size() const { return lights.size(); }
	auto& get_lights() const { return lights; }
};

}
}
