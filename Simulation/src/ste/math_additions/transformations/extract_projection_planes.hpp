// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

namespace ste {

namespace _detail {

inline glm::vec4 compute_distance_and_normal(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2) {
	auto u = p2 - p0;
	auto v = p1 - p0;
	auto n = glm::normalize(glm::cross(u, v));
	return { n.x, n.y, n.z, glm::dot(n, p0) };
}

}

inline void extract_projection_frustum_planes(float ffar,
											  float fnear,
											  float fovy,
											  float aspect,
											  glm::vec4 *np,
											  glm::vec4 *fp,
											  glm::vec4 *rp,
											  glm::vec4 *lp,
											  glm::vec4 *tp,
											  glm::vec4 *bp) {
	const glm::vec3 p = { 0, 0, 0 };
	const glm::vec3 d = { 0, 0, -1 };
	const glm::vec3 up = { 0, 1, 0 };
	const glm::vec3 right = { 1, 0, 0 };

	float Hnear = 2.f * fnear * glm::tan(fovy * .5f);
	float Hfar = 2.f * ffar * glm::tan(fovy * .5f);
	float Wnear = Hnear * aspect;
	float Wfar = Hfar * aspect;

	auto fc = p + d * ffar;
	auto ftl = fc + (up * Hfar * .5f) - (right * Wfar * .5f);
	auto ftr = fc + (up * Hfar * .5f) + (right * Wfar * .5f);
	auto fbl = fc - (up * Hfar * .5f) - (right * Wfar * .5f);
	auto fbr = fc - (up * Hfar * .5f) + (right * Wfar * .5f);
	auto nc = p + d * fnear;
	auto ntl = nc + (up * Hnear * .5f) - (right * Wnear * .5f);
	auto ntr = nc + (up * Hnear * .5f) + (right * Wnear * .5f);
	auto nbl = nc - (up * Hnear * .5f) - (right * Wnear * .5f);
	// auto nbr = nc - (up * Hnear * .5f) + (right * Wnear * .5f);

	*np = { 0, 0, -1, fnear };
	*fp = { 0, 0, 1, ffar };
	*rp = _detail::compute_distance_and_normal(ntr, fbr, ftr);
	*lp = _detail::compute_distance_and_normal(ntl, ftl, fbl);
	*tp = _detail::compute_distance_and_normal(ntl, ftr, ftl);
	*bp = _detail::compute_distance_and_normal(nbl, fbl, fbr);
}

}
