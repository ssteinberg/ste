// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "llr_resource.h"
#include "layout_binding.h"

namespace StE {
namespace LLR {

template <typename BinderType>
class shader_layout_bindable_resource {
protected:
	using LayoutLocationType = layout_binding<BinderType>;
	using EmptyLayoutLocationType = layout_binding_none<BinderType>;

public:
	virtual void bind(const LayoutLocationType &) const = 0;
	virtual void unbind(const LayoutLocationType &) const = 0;
};

}
}
