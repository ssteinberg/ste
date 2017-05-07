// TODO
#include <stdafx.hpp>
//#include <deferred_gbuffer.hpp>
//
//using namespace ste::graphics;
//using namespace ste::Core;
//
//void deferred_gbuffer::resize(glm::ivec2 size) {
//	if (size.x <= 0 || size.y <= 0)
//		return;
//
//	this->size = size;
//
//	gbuffer = std::make_unique<Core::texture_2d_array>(gli::format::FORMAT_RGBA32_SFLOAT_PACK32, glm::ivec3{ size.x, size.y, 2 });
//	depth_target = std::make_unique<Core::texture_2d>(gli::format::FORMAT_D32_SFLOAT_PACK32, glm::ivec2{ size });
//	downsampled_depth_target = std::make_unique<Core::texture_2d>(gli::format::FORMAT_RG32_SFLOAT_PACK32, glm::ivec2{ size } / 2, depth_buffer_levels - 1);
//
//	fbo.depth_binding_point() = *depth_target;
//	fbo[0] = (*gbuffer)[0];
//	fbo[1] = (*gbuffer)[1];
//	
//	backface_depth_target = std::make_unique<Core::texture_2d>(gli::format::FORMAT_D32_SFLOAT_PACK32, glm::ivec2{ size });
//	backface_fbo.depth_binding_point() = *backface_depth_target;
//
//	depth_target_modified_signal.emit();
//}
//
//void deferred_gbuffer::clear() {
//	glm::vec4 zero = { 0,0,0,0 };
//	gbuffer->clear(&zero);
//}
