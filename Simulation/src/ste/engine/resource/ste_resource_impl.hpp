//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_resource_type_traits.hpp>
#include <ste_resource_creator.hpp>

#include <type_traits>
#include <future>
#include <memory>
#include <forward_capture.hpp>
#include <tuple_call.hpp>

namespace StE {

namespace _detail {

template <typename T, bool async>
class ste_resource_base {};

// Non-async resource base
template <typename T>
class ste_resource_base<T, false> {
private:
	T res;

protected:
	virtual ~ste_resource_base() noexcept {}
	auto& get_resource() & { return res; }
	auto&& get_resource() && { return std::move(res); }
	auto& get_resource() const& { return res; }

public:
	template <typename ... Params>
	ste_resource_base(Params&&... params) : res(std::forward<Params>(params)...) {}

	ste_resource_base(ste_resource_base &&) = default;
	ste_resource_base &operator=(ste_resource_base &&) = default;
	ste_resource_base(const ste_resource_base &) = delete;
	ste_resource_base &operator=(const ste_resource_base &) = delete;
};
// Async resource base
template <typename T>
class ste_resource_base<T, true> {
	using future_t = std::future<void>;
	using this_t = ste_resource_base<T, true>;

private:
	void block() const {
		future.wait();
		wait_func_ptr = &this_t::noop;
	}
	void noop() const {}

private:
	using wait_func_ptr_t = decltype(&this_t::block);

	T res;
	mutable wait_func_ptr_t wait_func_ptr{ &this_t::block };
	future_t future;

private:
	void wait() const {
		(this->*wait_func_ptr)();
	}
	static T wait_and_copy(ste_resource_base &o) {
		o.wait();
		return o.res;
	}

protected:
	virtual ~ste_resource_base() noexcept {}
	void set_resource(T&& res) {
		this->res = std::move(res);
	}
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
	template <typename ... Params>
	ste_resource_base(future_t &&future,
					  Params&&... params)
		: res(std::forward<Params>(params)...),
		future(std::move(future))
	{}

	template<class U = T, typename = std::enable_if_t<std::is_move_constructible<U>::value>>
	ste_resource_base(ste_resource_base &&o)
		: res(std::move(o)), wait_func_ptr(o.wait_func_ptr), future(std::move(o.future)) {}
	template<class U = T, typename = std::enable_if_t<std::is_move_assignable<U>::value>>
	ste_resource_base &operator=(ste_resource_base &&o) {
		res = std::move(o.res);
		wait_func_ptr = o.wait_func_ptr;
		future = std::move(o.future);
		return *this;
	}
	template<class U = T, typename = std::enable_if_t<std::is_copy_constructible<U>::value>>
	ste_resource_base(const ste_resource_base &o)
		: res(wait_and_copy(o)), wait_func_ptr(&this_t::noop) {}
	template<class U = T, typename = std::enable_if_t<std::is_copy_assignable<U>::value>>
	ste_resource_base &operator=(const ste_resource_base &o) {
		res = wait_and_copy(o);
		wait_func_ptr = &this_t::noop;
		return *this;
	}
};

// Resource impl for non-async wrapper
template <typename T>
struct ste_resource_wrapper : public ste_resource_base<T, false> {
	using Base = ste_resource_base<T, false>;

protected:
	~ste_resource_wrapper() noexcept {}

public:
	template <typename ... Params>
	ste_resource_wrapper(const ste_context &ctx,
						 Params&&... params) : Base(std::forward<Params>(params)...) {}

	auto& get() & { return Base::get_resource(); }
	auto&& get() && { return std::move(Base::get_resource()); }
	auto& get() const& { return Base::get_resource(); }
	auto operator->() { return &get(); }
	auto operator->() const { return &get(); }
};

// Resource impl for async move-assignable and default-constructible types
template <typename T>
struct ste_resource_async_move_assignable_default_constructible : public ste_resource_base<T, true> {
	using Base = ste_resource_base<T, true>;

	template <typename ... Params>
	using ctor = typename ste_resource_creator<T>::template is_constructible_with<Params...>;

private:
	// If a T ctor accepts 'const ste_context &' as first argument, add ctx to the parameters.
	// This allows consumers to only pass the context once.
	template <typename ... Params>
	auto schedule_async_create(const ste_context &ctx,
							   Params&&... params,
							   std::enable_if_t<ctor<const ste_context &, Params...>::value>* = nullptr) {
		return ctx.engine().task_scheduler().schedule_now([this,
														  pack = forward_capture_pack(ctx, std::forward<Params>(params)...)]() mutable
		{
			ste_resource_creator<T> creator;
			auto res = tuple_call(&creator,
								  &ste_resource_creator<T>::template operator()<const ste_context &, Params...>,
								  std::move(pack));
			this->set_resource(std::move(res));
		});
	}
	template <typename ... Params>
	auto schedule_async_create(const ste_context &ctx,
							   Params&&... params,
							   std::enable_if_t<!ctor<const ste_context &, Params...>::value &&
							   ctor<Params...>::value>* = nullptr) {
		return ctx.engine().task_scheduler().schedule_now([this,
														  pack = forward_capture_pack(std::forward<Params>(params)...)]() mutable
		{
			ste_resource_creator<T> creator;
			auto res = tuple_call(&creator,
								  &ste_resource_creator<T>::template operator()<Params...>,
								  std::move(pack));
			this->set_resource(std::move(res));
		});
	}
	template <typename ... Params>
	auto schedule_async_create(const ste_context &ctx,
							   Params&&... params,
							   std::enable_if_t<!ctor<const ste_context &, Params...>::value &&
							   !ctor<Params...>::value>* = nullptr) {
		static_assert(ctor<Params...>::value, "T can not be constructed with those parameters");
		return task_future<void>();
	}

protected:
	~ste_resource_async_move_assignable_default_constructible() noexcept {}

public:
	template <typename ... Params>
	ste_resource_async_move_assignable_default_constructible(const ste_context &ctx,
															 Params&&... params)
		: Base(schedule_async_create<Params...>(ctx, std::forward<Params>(params)...).get_future())
	{}

	auto& get() & { return Base::get_resource(); }
	auto&& get() && { return std::move(Base::get_resource()); }
	auto& get() const& { return Base::get_resource(); }
	auto operator->() { return &get(); }
	auto operator->() const { return &get(); }
};

// Resource impl for async unique_ptr wrapped types
template <typename T>
struct ste_resource_async_ptr_wrap : public ste_resource_base<std::unique_ptr<T>, true> {
	using Base = ste_resource_base<std::unique_ptr<T>, true>;

	template <typename ... Params>
	using ctor = typename ste_resource_creator<T>::template is_constructible_with<Params...>;

private:
	// If a T ctor accepts 'const ste_context &' as first argument, add ctx to the parameters.
	// This allows consumers to only pass the context once.
	template <typename ... Params>
	auto schedule_async_create(const ste_context &ctx,
							   Params&&... params,
							   std::enable_if_t<ctor<const ste_context &, Params...>::value>* = nullptr) {
		return ctx.engine().task_scheduler().schedule_now([this,
														  pack = forward_capture_pack(ctx, std::forward<Params>(params)...)]() mutable
		{
			ste_resource_creator<T> creator;
			auto res_ptr = std::make_unique<T>(tuple_call(&creator,
														  &ste_resource_creator<T>::template operator()<const ste_context &, Params...>,
														  std::move(pack)));
			this->set_resource(std::move(res_ptr));
		});
	}
	template <typename ... Params>
	auto schedule_async_create(const ste_context &ctx,
							   Params&&... params,
							   std::enable_if_t<!ctor<const ste_context &, Params...>::value &&
							   ctor<Params...>::value>* = nullptr) {
		static_assert(ctor<Params...>::value, "T can not be constructed with those parameters");

		return ctx.engine().task_scheduler().schedule_now([this,
														  pack = forward_capture_pack(std::forward<Params>(params)...)]() mutable
		{
			ste_resource_creator<T> creator;
			auto res_ptr = std::make_unique<T>(tuple_call(&creator,
														  &ste_resource_creator<T>::template operator()<Params...>,
														  std::move(pack)));
			this->set_resource(std::move(res_ptr));
		});
	}
	template <typename ... Params>
	auto schedule_async_create(const ste_context &ctx,
							   Params&&... params,
							   std::enable_if_t<!ctor<const ste_context &, Params...>::value &&
							   !ctor<Params...>::value>* = nullptr) {
		static_assert(ctor<Params...>::value, "T can not be constructed with those parameters");
		return task_future<void>();
	}

protected:
	~ste_resource_async_ptr_wrap() noexcept {}

public:
	template <typename ... Params>
	ste_resource_async_ptr_wrap(const ste_context &ctx,
								Params&&... params)
		: Base(schedule_async_create<Params...>(ctx, std::forward<Params>(params)...).get_future())
	{}

	auto& get() & { return *Base::get_resource(); }
	auto&& get() && { return std::move(*Base::get_resource()); }
	auto& get() const& { return *Base::get_resource(); }
	auto operator->() { return &get(); }
	auto operator->() const { return &get(); }
};

// Resource base type selector
template <typename T>
struct ste_resource_resource_impl_type {
	static constexpr bool async = ste_resource_conforms_to_async_load<T>::value;
	static constexpr bool move_constructible = std::is_move_constructible<T>::value;
	static constexpr bool move_assignable = std::is_move_assignable<T>::value;
	static constexpr bool default_constructible = std::is_default_constructible<T>::value;

	static_assert(!async || move_constructible, "Resource conforming to ste_resource_load_async_trait must be move contructible");

	using non_async_value = ste_resource_wrapper<T>;
	using async_value = typename std::conditional<
		move_assignable && default_constructible,
		ste_resource_async_move_assignable_default_constructible<T>,
		ste_resource_async_ptr_wrap<T>
	>::type;
	using value = typename std::conditional <
		async,
		async_value,
		non_async_value
	>::type;
};

}

}
