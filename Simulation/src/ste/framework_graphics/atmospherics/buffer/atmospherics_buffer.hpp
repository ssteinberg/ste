// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <atmospherics_descriptor.hpp>

#include <ring_buffer_old.hpp>
#include <range.hpp>

namespace ste {
namespace Graphics {

class atmospherics_buffer {
private:
	using descriptor = atmospherics_descriptor;
	using buffer_type = Core::ring_buffer_old<descriptor, 3>;

private:
	buffer_type buffer;
	range<> r{ 0, 0 };

public:
	atmospherics_buffer(const descriptor::Properties &p) {
		update_data(p);
	}

	void update_data(const descriptor::Properties &p) {
		auto d = descriptor(p);
		range<> r = buffer.commit(d);

		r.start /= sizeof(d);
		r.length /= sizeof(d);

		this->r = r;
	}

	void bind_buffer(int idx) const {
		buffer.get_buffer().bind_range(Core::shader_storage_layout_binding(idx), r.start, r.length);
	}
};

}
}
