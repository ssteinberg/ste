
#include <stdafx.hpp>
#include <light_storage.hpp>

#include <limits>

using namespace ste;
using namespace ste::graphics;

//std::array<float, directional_light_cascades> light_storage::build_cascade_depth_array() {
//	std::array<float, directional_light_cascades> cascades;
//
//	// The near plane is just a suggestion, the first cascade will cover the area from the real near clip 
//	// plane to the end of the cascade.
//	static constexpr float n = 5.f;
//	// Something very large... No directional shadows after that. On earth, theoretical visibility is
//	// limited to <300km on the cleanest possible day.
//	static constexpr float f = 300e+3f;
//
//	// Split the view frustum into cascades
//	const float t = 1.f / float(directional_light_cascades);
//	float iflt = 1.f;
//	for (int i = 0; i<directional_light_cascades; ++i, ++iflt) {
//		cascades[i] = //n * glm::pow(f/n, iflt*t);
//			glm::mix(n + iflt * t * (f - n),
//					 n * glm::pow(f / n, iflt*t),
//					 .996f);
//	}
//
//	return cascades;
//}
//
//void light_storage::update_directional_lights_cascades_buffer(gl::command_recorder &recorder,
//															  const glm::dualquat &view_transform,
//															  float projection_near, 
//															  float projection_fovy, 
//															  float projection_aspect) {
//	const auto projection_tan_half_fovy = glm::tan(projection_fovy * .5f);
//	
//	lib::vector<light_cascades_descriptor> update_data;
//	update_data.reserve(active_directional_lights.size());
//
//	for (int i = 0; i < active_directional_lights.size(); ++i) {
//		light_cascades_descriptor descriptor;
//		const auto light = active_directional_lights[i].load(std::memory_order_acquire);
//		if (light == nullptr)
//			continue;
//
//		const auto l = view_transform.real * light->get_direction();
//
//		glm::vec3 x = glm::cross(l, glm::vec3(0, 1, 0));
//		if (glm::dot(x, x) < 1e-5f)
//			x = glm::cross(l, glm::vec3(1, 0, 0));
//		x = glm::normalize(x);
//		const glm::vec3 y = glm::normalize(glm::cross(l, x));
//		x = -x;		// Keep right-handed system
//
//		for (int c = 0; c < directional_light_cascades; ++c) {
//			float near_clip = c == 0 ? projection_near : cascades_depths[c - 1];
//			float far_clip = cascades_depths[c];
//
//			const glm::vec2 top = projection_tan_half_fovy * glm::vec2(near_clip, far_clip);
//			const glm::vec2 right = top * projection_aspect;
//
//			// Create the temporary, untranslated, unprojected transform to cascade space matrix,
//			// and use it to calculate the corrected eye distance and viewport size
//			const glm::mat3x4 M = light_cascade_data::generate(x, y,
//															   l, .0f, glm::vec2(1.f),
//															   near_clip, far_clip);
//
//			// Frustum vertices
//			const glm::vec4 frustum[8] = {
//				{  right.x,  top.x, -near_clip, 1 },
//				{ -right.x,  top.x, -near_clip, 1 },
//				{  right.x, -top.x, -near_clip, 1 },
//				{ -right.x, -top.x, -near_clip, 1 },
//				{  right.y,  top.y, -far_clip,  1 },
//				{ -right.y,  top.y, -far_clip,  1 },
//				{  right.y, -top.y, -far_clip,  1 },
//				{ -right.y, -top.y, -far_clip,  1 }
//			};
//
//			// Calculate cascade viewport limits
//			glm::vec4 t = { 0, 0, -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity() };
//			for (int j = 0; j < 8; ++j) {
//				const glm::vec3 transformed = frustum[j] * M;
//				t = glm::max(t, glm::vec4(glm::abs(transformed.x),
//										  glm::abs(transformed.y),
//										  transformed.z,
//										  -transformed.z));
//			}
//
//			// Viewport size
//			const glm::vec2 vp = glm::vec2{ t.x, t.y } *cascade_viewport_reserve;
//			const glm::vec2 recp_vp = 1.f / vp;
//
//			const float eye_dist = t.z + cascade_projection_eye_distance;
//			const float proj_far_clip = t.w + eye_dist;
//
//			descriptor.cascades()[c].proj_eye_dist() = eye_dist;
//			descriptor.cascades()[c].proj_far_clip() = proj_far_clip;
//			descriptor.cascades()[c].recp_vp() = recp_vp;
//			descriptor.cascades()[c].M() = light_cascade_data::generate(x, y, l, eye_dist,
//																		recp_vp, near_clip, far_clip);
//		}
//
//		update_data.push_back(descriptor);
//	}
//
//	if (update_data.size())
//		recorder << directional_lights_cascades_buffer.overwrite_cmd(0, update_data);
//}
