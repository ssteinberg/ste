
#include "stdafx.hpp"
#include "material.hpp"

using namespace StE::Graphics;

material::material(material_layer *head_layer) {
	material_sampler.set_wrap_s(Core::texture_wrap_mode::Wrap);
	material_sampler.set_wrap_t(Core::texture_wrap_mode::Wrap);
	material_sampler.set_wrap_r(Core::texture_wrap_mode::Wrap);
	material_sampler.set_min_filter(Core::texture_filtering::Linear);
	material_sampler.set_mag_filter(Core::texture_filtering::Linear);
	material_sampler.set_mipmap_filter(Core::texture_filtering::Linear);
	material_sampler.set_anisotropic_filter(16);
	
	set_head_layer(head_layer);
}

StE::Core::texture_handle material::handle_for_texture(const Core::texture_2d *t) const {
	Core::texture_handle h;
	if (t != nullptr) {
		h = t->get_texture_handle(material_sampler);
		h.make_resident();
	}
	return h;
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
