// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "Camera.hpp"

#include "ring_buffer.hpp"
#include "range.hpp"

namespace StE {
namespace Graphics {

class view_matrix_ring_buffer {
private:
	struct matrices {
		glm::mat4 view_model;
		glm::mat4 transpose_inverse_view_model;
		glm::mat4 projection_view_matrix;
	};

	using ring_buffer_type = Core::ring_buffer<matrices, 3, false>;

private:
	ring_buffer_type rbuffer;
	range<> r{ 0, 0 };

public:
	void update_with_camera(const Camera &c, const glm::mat4& proj) {
		matrices m;
		m.view_model = c.view_matrix();
		m.transpose_inverse_view_model = glm::transpose(glm::inverse(m.view_model));
		m.projection_view_matrix = proj * m.view_model;

		range<> r = rbuffer.commit(m);
		r.start /= sizeof(matrices);
		r.length /= sizeof(matrices);

		this->r = r;
	}

	void bind_buffer(int idx) const {
		rbuffer.get_buffer().bind_range(Core::shader_storage_layout_binding(idx), r.start, r.length);;
	}
};

}
}
