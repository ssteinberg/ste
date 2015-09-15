// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "llr_resource.h"

#include <utility>

namespace StE {
namespace LLR {

class llr_resource_stub_binder {
public:
	static void bind() {}
	static void unbind() {}
};

class bindable_generic_resource : virtual public GenericResource {
public:
	virtual void bind() const = 0;
	virtual void unbind() const = 0;
};

template <class A, class B, typename... BindingArgs>
class bindable_resource : public llr_resource<A> {
protected:
	using Binder = B;

public:
	bindable_resource() {}

	bindable_resource(bindable_resource &&m) = default;
	bindable_resource& operator=(bindable_resource &&m) = default;

	void bind(BindingArgs&&... args) const { Binder::bind(id, std::forward<BindingArgs>(args)...); }
	void unbind(BindingArgs&&... args) const { Binder::unbind(std::forward<BindingArgs>(args)...); };
};

template <class A, class B>
class bindable_resource<A, B> : public llr_resource<A>, public bindable_generic_resource{
protected:
	using Binder = B;

public:
	bindable_resource() {}

	bindable_resource(bindable_resource &&m) = default;
	bindable_resource& operator=(bindable_resource &&m) = default;

	void bind() const override final { Binder::bind(id); }
	void unbind() const override final { Binder::unbind(); };
};

}
}
