//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <memory>
#include <functional>

namespace StE {
namespace GL {

template <class Pool, template<class> class resource_reclamation_policy>
class ste_resource_pool_resource {
	friend Pool;

private:
	using resource_t = typename Pool::value_type;
	using pool_ptr_t = typename Pool::pool_ptr_t;
	using pointer = resource_t*;
	using const_pointer = const resource_t*;
	using reference = resource_t&;

private:
	std::unique_ptr<resource_t> resource;
	pool_ptr_t pool_ptr;

	ste_resource_pool_resource(const pool_ptr_t &pool_ptr, std::unique_ptr<resource_t> &&resource)
		: resource(std::move(resource)), pool_ptr(pool_ptr)
	{}

public:
	~ste_resource_pool_resource() noexcept {
		release();
	}

	ste_resource_pool_resource(ste_resource_pool_resource&&) = default;
	ste_resource_pool_resource &operator=(ste_resource_pool_resource&&) = default;

	void release() {
		if (resource != nullptr) {
			pool_ptr->push(std::move(resource));
			resource = nullptr;
		}
	}

	template <typename S = resource_t>
	typename std::enable_if<resource_reclamation_policy<S>::allow_non_const_resource, reference>::type get() {
		return *resource;
	}
	const reference get() const { return *resource; }

	template <typename S = resource_t>
	typename std::enable_if<resource_reclamation_policy<S>::allow_non_const_resource, pointer>::type operator->() {
		return &get(); 
	}
	const_pointer operator->() const { return &get(); }

	template <typename S = resource_t>
	typename std::enable_if<resource_reclamation_policy<S>::allow_non_const_resource, reference>::type operator*() {
		return get(); 
	}
	const reference operator*() const { return get(); }

	template <
		typename S = resource_t,
		typename = typename std::enable_if<std::is_copy_assignable_v<S> || std::is_copy_constructible_v<S>>::type
	>
	operator S() const { return get(); }
};

}
}
