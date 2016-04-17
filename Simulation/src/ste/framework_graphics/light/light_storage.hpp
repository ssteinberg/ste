// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "light.hpp"

#include "gstack.hpp"

#include "range.hpp"

#include <vector>
#include <memory>

namespace StE {
namespace Graphics {

class light_storage {
private:
	std::vector<std::shared_ptr<light>> lights;
	Core::gstack<light::light_descriptor> stack;
	Core::gstack<glm::vec4> transformed_buffer_stack;

	mutable std::vector<range<>> ranges_to_lock;

public:
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
		transformed_buffer_stack.push_back(glm::vec4(1.f));
		l->clear_dirty();
	}

	void add_lights(const std::vector<std::shared_ptr<light>> &ls) {
		for (auto &l : ls)
			add_light(l);
	}

	void bind_buffers(int first) const {
		stack.get_buffer().bind_range(Core::shader_storage_layout_binding(first), 0, lights.size());
		transformed_buffer_stack.get_buffer().bind_range(Core::shader_storage_layout_binding(first + 1), 0, lights.size());
	}

	auto& get_lights() const { return lights; }
};

}
}
