
#include <stdafx.hpp>
#include <material.hpp>

using namespace ste;
using namespace ste::graphics;

material::material(const ste_context &ctx,
				   material_layer *head_layer)
	: material_sampler(ctx.device(),
					   gl::sampler_parameter::anisotropy(16),
					   gl::sampler_parameter::address_mode(gl::sampler_address_mode::repeat,
														   gl::sampler_address_mode::repeat,
														   gl::sampler_address_mode::repeat),
					   gl::sampler_parameter::filtering(gl::sampler_filter::linear,
														gl::sampler_filter::linear,
														gl::sampler_mipmap_mode::linear))
{
	set_head_layer(head_layer);
}

void material::set_head_layer(material_layer *head_layer) {
	descriptor.head_layer_id = material_layer_none;
	this->head_layer = nullptr;

	if (head_layer != nullptr) {
		auto id = head_layer->resource_index_in_storage();
		assert(id >= 0);

		if (id >= 0) {
			descriptor.head_layer_id = id;
			this->head_layer = head_layer;
		}
	}
}

void material::attach_layer_stack(material_layer *layer) {
	set_head_layer(head_layer);
	Base::notify();
}
