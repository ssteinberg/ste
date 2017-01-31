
#include "stdafx.hpp"
#include "light_storage.hpp"

using namespace StE::Graphics;

void light_storage::build_cascade_depth_array() {
	// The near plane is just a suggestion, the first cascade will cover the area from the real near clip 
	// plane to the end of the cascade.
	float n = 5.f;
	// Something very large... No directional shadows after that. On earth, theoretical visibility is
	// limited to <300km on the cleanest possible day.
	float f = 300e+3f;

	// Split the view frustum into cascades
	float iflt = 1.f;
	float t = 1.f / float(directional_light_cascades);
	for (int i = 0; i<directional_light_cascades; ++i, ++iflt) {
		cascades_depths[i] = //n * glm::pow(f/n, iflt*t);
			glm::mix(n + iflt * t * (f - n),
					 n * glm::pow(f / n, iflt*t),
					 .996f);
	}
}
