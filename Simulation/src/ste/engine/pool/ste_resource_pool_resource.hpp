//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <memory>
#include <functional>

namespace StE {
namespace GL {

template <class Pool, template<typename> class resource_reclamation_policy>
class ste_resource_pool_resource {
	friend Pool;

private:
	using resource_t = typename Pool::value_type;
	using pointer = resource_t*;
	using const_pointer = const resource_t*;
	using reference = resource_t&;

private:
	std::unique_ptr<resource_t> resource;
	Pool *pool;

	ste_resource_pool_resource(Pool *pool, std::unique_ptr<resource_t> &&resource)
		: resource(std::move(resource)), pool(pool)
	{}

public:
	~ste_resource_pool_resource() noexcept {
		pool->release_to_pool(std::move(resource));
	}

	ste_resource_pool_resource(ste_resource_pool_resource&&) = default;
	ste_resource_pool_resource &operator=(ste_resource_pool_resource&&) = default;

	template <typename S = resource_t>
	typename std::enable_if<resource_reclamation_policy<S>::allow_non_const_resource, reference>::type get() {
		return *resource;
	}
	const reference get() const { return *resource; }

	template <typename S = resource_t>
	typename std::enable_if<resource_reclamation_policy<S>::allow_non_const_resource, pointer>::type operator->() {
		return &get(); }
	const_pointer operator->() const { return &get(); }

	template <typename S = resource_t>
	typename std::enable_if<resource_reclamation_policy<S>::allow_non_const_resource, reference>::type operator*() {
		return get(); }
	const reference operator*() const { return get(); }
};

}
}
