// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "llr_resource.h"
#include "shader_layout_bindable_resource.h"

namespace StE {
namespace LLR {

class llr_resource_stub_binder {
public:
	static void bind() {}
	static void unbind() {}
};

template <class A, class B, typename... BindingArgs>
class bindable_resource : public llr_resource<A> {
protected:
	using Binder = B;

	bindable_resource() {}

public:
	bindable_resource(bindable_resource &&m) = default;
	bindable_resource& operator=(bindable_resource &&m) = default;

	void bind(const BindingArgs&... args) const { Binder::bind(id, args...); }
	void unbind(const BindingArgs&... args) const { Binder::unbind(args...); };
};

class bindable_generic_resource : virtual public GenericResource {
public:
	virtual void bind() const = 0;
	virtual void unbind() const = 0;
};

template <class A, class B>
class bindable_resource<A, B> : public llr_resource<A>, public virtual bindable_generic_resource{
protected:
	using Binder = B;

	bindable_resource() {}

public:
	bindable_resource(bindable_resource &&m) = default;
	bindable_resource& operator=(bindable_resource &&m) = default;

	void bind() const override { Binder::bind(id); }
	void unbind() const override { Binder::unbind(); };
};

}
}
