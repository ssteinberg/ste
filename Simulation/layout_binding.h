// StE
// © Shlomi Steinberg, 2015

#pragma once

namespace StE {
namespace LLR {

template <typename BindingType>
class shader_layout_bindable_resource;

template <typename T>
class layout_binding {
public:
	using BindingType = T;

private:
	int index;

public:
	explicit layout_binding(unsigned long long int i) : index(static_cast<int>(i)) {}

	int binding_index() const { return index; }
	operator int() const { return binding_index(); }

	void inline operator=(const shader_layout_bindable_resource<BindingType> &binable_shader_resource) const;
};

template <typename BindingType>
class layout_binding_none : public layout_binding<BindingType> {
public:
	layout_binding_none() : layout_binding(-1) {}
};

template <typename BindingType>
bool inline operator==(const layout_binding<BindingType>& lhs, const layout_binding<BindingType>& rhs) { return lhs.binding_index() == rhs.binding_index(); }
template <typename BindingType>
bool inline operator!=(const layout_binding<BindingType>& lhs, const layout_binding<BindingType>& rhs) { return lhs.binding_index() != rhs.binding_index(); }

}
}

#include "shader_layout_bindable_resource.h"

namespace StE {
namespace LLR {

template <typename BindingType>
void inline layout_binding<BindingType>::operator=(const shader_layout_bindable_resource<BindingType> &binable_shader_resource) const {
	binable_shader_resource.bind(*this); 
}

}
}
