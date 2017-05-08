//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <memory>

#include <allow_type_decay.hpp>

namespace ste {
namespace gl {

template <class Pool, template<class> class resource_reclamation_policy>
class ste_resource_pool_resource :
	public allow_type_decay<
		ste_resource_pool_resource<Pool, resource_reclamation_policy>,
		typename Pool::value_type,
		!resource_reclamation_policy<typename Pool::value_type>::allow_non_const_resource
	>
{
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
};

}
}
