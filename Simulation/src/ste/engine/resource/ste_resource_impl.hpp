//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_resource_creator.hpp>
#include <ste_resource_creation_policy.hpp>

#include <type_traits>
#include <memory>
#include <atomic>
#include <forward_capture.hpp>
#include <tuple_call.hpp>

namespace ste {

namespace _detail {

// Deferred resource base
template <typename T, class resource_deferred_policy>
class ste_resource_base_deferred {
	using policy_t = typename resource_deferred_policy::template policy<T>;

private:
	void block() const {
		// consume() is a thread-safe blocker
		policy.consume(res);
		wait_func_ptr.store(&ste_resource_base_deferred::noop, std::memory_order_release);
	}
	void noop() const {}

private:
	using wait_func_ptr_t = decltype(&ste_resource_base_deferred::block);

	mutable T res;
	mutable std::atomic<wait_func_ptr_t> wait_func_ptr{ &ste_resource_base_deferred::block };
	mutable policy_t policy;

private:
	void wait() const {
		auto ptr = wait_func_ptr.load(std::memory_order_acquire);
		(this->*ptr)();
	}
	static T wait_and_copy(ste_resource_base_deferred &o) {
		o.wait();
		return o.res;
	}

protected:
	virtual ~ste_resource_base_deferred() noexcept {}

	auto& get_resource() & {
		wait();
		return res;
	}
	auto&& get_resource() && {
		wait();
		return std::move(res);
	}
	auto& get_resource() const& {
		wait();
		return res;
	}

public:
	template <typename L, typename ... Params>
	ste_resource_base_deferred(const ste_context &ctx,
							   L&& l,
							   Params&&... params)
		: res(std::forward<Params>(params)...),
		policy(ctx, std::forward<L>(l))
	{}
	template <typename ... Params>
	ste_resource_base_deferred(ste_resource_dont_defer,
							   Params&&... params)
		: res(std::forward<Params>(params)...),
		wait_func_ptr(&ste_resource_base_deferred::noop)
	{}

	template<class U = T, typename = std::enable_if_t<std::is_move_constructible<U>::value>>
	ste_resource_base_deferred(ste_resource_base_deferred &&o) noexcept
		: res(std::move(o.res)),
		wait_func_ptr(o.wait_func_ptr.load(std::memory_order_acquire)),
		policy(std::move(o.policy)) {}
	template<class U = T, typename = std::enable_if_t<std::is_move_assignable<U>::value>>
	ste_resource_base_deferred &operator=(ste_resource_base_deferred &&o) noexcept {
		res = std::move(o.res);
		wait_func_ptr = o.wait_func_ptr;
		policy = std::move(o.policy);
		return *this;
	}
	template<class U = T, typename = std::enable_if_t<std::is_copy_constructible<U>::value>>
	ste_resource_base_deferred(const ste_resource_base_deferred &o)
		: res(wait_and_copy(o)),
		wait_func_ptr(&ste_resource_base_deferred::noop) {}
	template<class U = T, typename = std::enable_if_t<std::is_copy_assignable<U>::value>>
	ste_resource_base_deferred &operator=(const ste_resource_base_deferred &o) {
		res = wait_and_copy(o);
		wait_func_ptr.store(&ste_resource_base_deferred::noop);
		return *this;
	}
};

// Resource impl for a non-deferred wrapper
template <typename T>
struct ste_resource_wrapper {
private:
	T res;

protected:
	~ste_resource_wrapper() noexcept {}

public:
	template <typename ... Params>
	ste_resource_wrapper(Params&&... params)
		: res(std::forward<Params>(params)...)
	{}
	template <typename ... Params>
	ste_resource_wrapper(ste_resource_dont_defer,
						 Params&&... params)
		: res(std::forward<Params>(params)...)
	{}
	template <typename L>
	ste_resource_wrapper(ste_resource_create_with_lambda,
						 const ste_context &ctx,
						 L&& l)
		: res(l())
	{}
	template <typename L>
	ste_resource_wrapper(ste_resource_create_with_lambda,
						 ste_resource_dont_defer,
						 L&& l)
		: res(l())
	{}

	ste_resource_wrapper(ste_resource_wrapper&&) = default;
	ste_resource_wrapper &operator=(ste_resource_wrapper&&) = default;

	auto& get() & { return res; }
	auto&& get() && { return std::move(res); }
	auto& get() const& { return res; }
};

// Deferred resource loader impl for move-assignable and default-constructible types
template <typename T, class resource_deferred_policy>
struct ste_resource_deferred_move_assignable_default_constructible : public ste_resource_base_deferred<T, resource_deferred_policy> {
	using Base = ste_resource_base_deferred<T, resource_deferred_policy>;

	template <typename ... Params>
	using ctor = ste_resource_is_constructible<T, Params...>;

private:
	// If a T ctor accepts 'const ste_context &' as first argument, try to add ctx to the parameters, this'd 
	// allow consumers to only pass the context once.
	template <typename CtorParam = const ste_context &>
	auto deferred_create_lambda_noargs(const ste_context &ctx,
									   std::enable_if_t<ctor<CtorParam>::value>* = nullptr) {
		return ([this,
				pack = forward_capture_pack(ctx)]() mutable
		{
			ste_resource_creator<T> creator;
			auto res = tuple_call(&creator,
								  &ste_resource_creator<T>::template operator() < const ste_context & > ,
								  std::move(pack));
			return std::move(res);
		});
	}
	template <typename CtorParam = const ste_context &>
	auto deferred_create_lambda_noargs(const ste_context &,
									   std::enable_if_t<!ctor<CtorParam>::value && ctor<void>::value>* = nullptr) {
		return ([this]() mutable
		{
			ste_resource_creator<T> creator;
			auto res = creator();
			return std::move(res);
		});
	}
	template <typename CtorParam = const ste_context &>
	auto deferred_create_lambda_noargs(const ste_context &,
									   std::enable_if_t<!ctor<CtorParam>::value && !ctor<void>::value>* = nullptr) {
		static_assert(ctor<CtorParam>::value || ctor<void>::value, "T can not be constructed with those parameters");
		return []() {};
	}
	template <typename ... Params>
	auto deferred_create_lambda(const ste_context &ctx,
								Params&&... params,
								std::enable_if_t<ctor<const ste_context &, Params...>::value>* = nullptr) {
		return ([this,
				pack = forward_capture_pack(ctx, std::forward<Params>(params)...)]() mutable
		{
			ste_resource_creator<T> creator;
			auto res = tuple_call(&creator,
								  &ste_resource_creator<T>::template operator() < const ste_context &, Params... > ,
								  std::move(pack));
			return std::move(res);
		});
	}
	template <typename ... Params>
	auto deferred_create_lambda(const ste_context &,
								Params&&... params,
								std::enable_if_t<!ctor<const ste_context &, Params...>::value &&
								ctor<Params...>::value>* = nullptr) {
		return ([this,
				pack = forward_capture_pack(std::forward<Params>(params)...)]() mutable
		{
			ste_resource_creator<T> creator;
			auto res = tuple_call(&creator,
								  &ste_resource_creator<T>::template operator() < Params... > ,
								  std::move(pack));
			return std::move(res);
		});
	}
	template <typename ... Params>
	auto deferred_create_lambda(const ste_context &,
								Params&&...,
								std::enable_if_t<!ctor<const ste_context &, Params...>::value &&
								!ctor<Params...>::value>* = nullptr) {
		static_assert(ctor<const ste_context &, Params...>::value ||
					  ctor<Params...>::value, "T can not be constructed with those parameters");
		return []() {};
	}

protected:
	~ste_resource_deferred_move_assignable_default_constructible() noexcept {}

public:
	ste_resource_deferred_move_assignable_default_constructible(const ste_context &ctx)
		: Base(ctx,
			   deferred_create_lambda_noargs<>(ctx))
	{}
	template <typename ... Params>
	ste_resource_deferred_move_assignable_default_constructible(const ste_context &ctx,
																Params&&... params)
		: Base(ctx,
			   deferred_create_lambda<Params...>(ctx, std::forward<Params>(params)...))
	{}
	template <typename ... Params>
	ste_resource_deferred_move_assignable_default_constructible(ste_resource_dont_defer,
																Params&&... params)
		: Base(ste_resource_dont_defer(),
			   std::forward<Params>(params)...)
	{}
	template <typename L>
	ste_resource_deferred_move_assignable_default_constructible(ste_resource_create_with_lambda,
																const ste_context &ctx,
																L&& l)
		: Base(ctx,
			   std::forward<L>(l))
	{}
	template <typename L>
	ste_resource_deferred_move_assignable_default_constructible(ste_resource_create_with_lambda,
																ste_resource_dont_defer,
																L&& l)
		: Base(ste_resource_dont_defer(),
			   l())
	{}

	ste_resource_deferred_move_assignable_default_constructible(ste_resource_deferred_move_assignable_default_constructible&&) = default;
	ste_resource_deferred_move_assignable_default_constructible &operator=(ste_resource_deferred_move_assignable_default_constructible&&) = default;

	auto& get() & { return Base::get_resource(); }
	auto&& get() && { return std::move(Base::get_resource()); }
	auto& get() const& { return Base::get_resource(); }
};

// Deferred resource loader impl for unique_ptr wrapped types
template <typename T, class resource_deferred_policy>
struct ste_resource_deferred_ptr_wrap : public ste_resource_base_deferred<std::unique_ptr<T>, resource_deferred_policy> {
	using Base = ste_resource_base_deferred<std::unique_ptr<T>, resource_deferred_policy>;

	template <typename ... Params>
	using ctor = ste_resource_is_constructible<T, Params...>;

private:
	// If a T ctor accepts 'const ste_context &' as first argument, try to add ctx to the parameters, this'd 
	// allow consumers to only pass the context once.
	template <typename CtorParam = const ste_context &>
	auto deferred_create_lambda_noargs(const ste_context &ctx,
									   std::enable_if_t<ctor<CtorParam>::value>* = nullptr) {
		return ([this,
				pack = forward_capture_pack(ctx)]() mutable
		{
			ste_resource_creator<T> creator;
			auto res_ptr = std::make_unique<T>(tuple_call(&creator,
														  &ste_resource_creator<T>::template operator() < const ste_context & > ,
														  std::move(pack)));
			return std::move(res_ptr);
		});
	}
	template <typename CtorParam = const ste_context &>
	auto deferred_create_lambda_noargs(const ste_context &,
									   std::enable_if_t<!ctor<CtorParam>::value && ctor<void>::value>* = nullptr) {
		return ([this]() mutable
		{
			ste_resource_creator<T> creator;
			auto res_ptr = std::make_unique<T>(creator());
			return std::move(res_ptr);
		});
	}
	template <typename CtorParam = const ste_context &>
	auto deferred_create_lambda_noargs(const ste_context &,
									   std::enable_if_t<!ctor<CtorParam>::value && !ctor<void>::value>* = nullptr) {
		static_assert(ctor<CtorParam>::value || ctor<void>::value, "T can not be constructed with those parameters");
		return []() {};
	}
	template <typename ... Params>
	auto deferred_create_lambda(const ste_context &ctx,
								Params&&... params,
								std::enable_if_t<ctor<const ste_context &, Params...>::value>* = nullptr) {
		return ([this,
				pack = forward_capture_pack(ctx, std::forward<Params>(params)...)]() mutable
		{
			ste_resource_creator<T> creator;
			auto res_ptr = std::make_unique<T>(tuple_call(&creator,
														  &ste_resource_creator<T>::template operator() < const ste_context &, Params... > ,
														  std::move(pack)));
			return std::move(res_ptr);
		});
	}
	template <typename ... Params>
	auto deferred_create_lambda(const ste_context &,
								Params&&... params,
								std::enable_if_t<!ctor<const ste_context &, Params...>::value &&
								ctor<Params...>::value>* = nullptr) {
		return ([this,
				pack = forward_capture_pack(std::forward<Params>(params)...)]() mutable
		{
			ste_resource_creator<T> creator;
			auto res_ptr = std::make_unique<T>(tuple_call(&creator,
														  &ste_resource_creator<T>::template operator() < Params... > ,
														  std::move(pack)));
			return std::move(res_ptr);
		});
	}
	template <typename ... Params>
	auto deferred_create_lambda(const ste_context &,
								Params&&...,
								std::enable_if_t<!ctor<const ste_context &, Params...>::value &&
								!ctor<Params...>::value>* = nullptr) {
		static_assert(ctor<Params...>::value, "T can not be constructed with those parameters");
		return []() {};
	}

protected:
	~ste_resource_deferred_ptr_wrap() noexcept {}

public:
	ste_resource_deferred_ptr_wrap(const ste_context &ctx)
		: Base(ctx,
			   deferred_create_lambda_noargs<>(ctx))
	{}
	template <typename ... Params>
	ste_resource_deferred_ptr_wrap(const ste_context &ctx,
								   Params&&... params)
		: Base(ctx,
			   deferred_create_lambda<Params...>(ctx, std::forward<Params>(params)...))
	{}
	template <typename ... Params>
	ste_resource_deferred_ptr_wrap(ste_resource_dont_defer,
								   Params&&... params)
		: Base(ste_resource_dont_defer(),
			   std::make_unique<T>(std::forward<Params>(params)...))
	{}
	template <typename L>
	ste_resource_deferred_ptr_wrap(ste_resource_create_with_lambda,
								   const ste_context &ctx,
								   L&& l)
		: Base(ctx,
			   [l = std::forward<L>(l)]() mutable { return std::make_unique<T>(l()); })
	{}
	template <typename L>
	ste_resource_deferred_ptr_wrap(ste_resource_create_with_lambda,
								   ste_resource_dont_defer,
								   L&& l)
		: Base(ste_resource_dont_defer(),
			   std::make_unique<T>(l()))
	{}

	ste_resource_deferred_ptr_wrap(ste_resource_deferred_ptr_wrap&&) = default;
	ste_resource_deferred_ptr_wrap&operator=(ste_resource_deferred_ptr_wrap&&) = default;

	auto& get() { return *Base::get_resource(); }
	auto& get() const { return *Base::get_resource(); }
};

}

}
