// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "resource.hpp"
#include "shader_layout_bindable_resource.hpp"

namespace StE {
namespace LLR {

class llr_resource_stub_binder {
public:
	static void bind() {}
	static void unbind() {}
};

class bindable_generic_resource : virtual public GenericResource {
protected:
	virtual ~bindable_generic_resource() noexcept {}

public:
	virtual void bind() const = 0;
	virtual void unbind() const = 0;
};

template <class A, class B, typename... BindingArgs>
class bindable_resource : public resource<A> {
protected:
	using Binder = B;

	bindable_resource() {}
	explicit bindable_resource(const bindable_resource &res) : resource<A>(res) {}
	~bindable_resource() noexcept {}

	using resource<A>::resource;

public:
	bindable_resource(bindable_resource &&m) = default;
	bindable_resource& operator=(bindable_resource &&m) = default;

	void bind(const BindingArgs&... args) const { Binder::bind(resource<A>::get_resource_id(), args...); }
	void unbind(const BindingArgs&... args) const { Binder::unbind(args...); };
};

template <class A, class B>
class bindable_resource<A, B> : public resource<A>, public bindable_generic_resource{
protected:
	using Binder = B;

	bindable_resource() {}
	explicit bindable_resource(const bindable_resource &res) : resource<A>(res) {}
	~bindable_resource() noexcept {}

	using resource<A>::resource;

public:
	bindable_resource(bindable_resource &&m) = default;
	bindable_resource& operator=(bindable_resource &&m) = default;

	void bind() const override { Binder::bind(resource<A>::get_resource_id()); }
	void unbind() const override { Binder::unbind(); };
};

}
}
