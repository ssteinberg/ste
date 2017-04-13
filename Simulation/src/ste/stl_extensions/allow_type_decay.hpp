// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <utility>
#include <type_traits>

namespace StE {

namespace _detail {

struct allow_type_decay_conversion_empty {};

template <
	typename CRTP,
	typename DecayT,
	typename NextDecayT,
	bool only_const,
	int Depth
>
struct allow_type_decay_conversion_recursive;

template <typename T>
decltype(T::_type_decay_conversion_only_const, std::true_type{}) _allows_decay(int);
template <typename T>
std::false_type _allows_decay(...) {}
template <typename T>
using _allows_decay_tester = decltype(_allows_decay<T>(0));
template <typename DecayT>
struct allow_recursive_decay {
	static constexpr bool value = _allows_decay_tester<DecayT>{};
};

template <typename CRTP, typename DecayT, typename NextDecayT, bool only_const, int Depth, bool decays>
class recursive_type_decay_t : public allow_type_decay_conversion_recursive<
	CRTP,
	typename DecayT::_type_decay_conversion_decay_t,
	NextDecayT,
	DecayT::_type_decay_conversion_only_const && only_const,
	Depth + 1
> {};
template <typename CRTP, typename DecayT, typename NextDecayT, bool only_const, int Depth>
class recursive_type_decay_t<CRTP, DecayT, NextDecayT, only_const, Depth, false> : public allow_type_decay_conversion_empty {};

template <typename NextDecayT, int Depth>
struct class_recusive_decay_getter {
	template <typename T>
	auto& operator()(T&& t) {
		auto getter = class_recusive_decay_getter<typename NextDecayT::_type_decay_conversion_decay_t, Depth - 1>();
		return getter(static_cast<NextDecayT&>(std::forward<T>(t).get()));
	}
	template <typename T>
	const auto& operator()(const T& t) {
		auto getter = class_recusive_decay_getter<typename NextDecayT::_type_decay_conversion_decay_t, Depth - 1>();
		return getter(static_cast<const NextDecayT&>(t.get()));
	}
};
template <typename NextDecayT>
struct class_recusive_decay_getter<NextDecayT, 0> {
	template <typename T>
	auto& operator()(T&& t) {
		return static_cast<NextDecayT&>(std::forward<T>(t).get());
	}
	template <typename T>
	const auto& operator()(const T& t) {
		return static_cast<const NextDecayT&>(t.get());
	}
};

template <
	typename CRTP,
	typename DecayT,
	typename NextDecayT,
	bool only_const,
	int Depth
>
struct allow_type_decay_conversion_recursive :
	public recursive_type_decay_t<CRTP, DecayT, NextDecayT, only_const, Depth, _detail::allow_recursive_decay<DecayT>::value>
{
	virtual ~allow_type_decay_conversion_recursive() noexcept {}

	template <
		bool b = !only_const,
		typename = typename std::enable_if<b>::type
	>
	operator DecayT&() { return class_recusive_decay_getter<NextDecayT, Depth>()(*dynamic_cast<CRTP*>(this)); }
	template <
		typename S = DecayT,
		typename = typename std::enable_if<!std::is_abstract_v<S> && (std::is_copy_assignable_v<S> || std::is_copy_constructible_v<S>)>::type
	>
	operator DecayT() const { return class_recusive_decay_getter<NextDecayT, Depth>()(*dynamic_cast<const CRTP*>(this)); }
	operator const DecayT&() const { return class_recusive_decay_getter<NextDecayT, Depth>()(*dynamic_cast<const CRTP*>(this)); }
};

template <typename T>
using decayed_type_t = std::conditional_t<
	std::is_abstract_v<T>,
	T&,
	T
>;

}

template <
	typename CRTP,
	typename DecayT,
	bool only_const = true
>
class allow_type_decay_conversion :
	public _detail::allow_type_decay_conversion_recursive<CRTP, _detail::decayed_type_t<DecayT>, _detail::decayed_type_t<DecayT>, only_const, 0>
{
public:
	virtual ~allow_type_decay_conversion() noexcept {}

	using _type_decay_conversion_decay_t = _detail::decayed_type_t<DecayT>;
	static constexpr bool _type_decay_conversion_only_const = only_const;
};

template <
	typename CRTP,
	typename DecayT,
	bool only_const = true
>
class allow_type_decay : public allow_type_decay_conversion<CRTP, DecayT, only_const> {
public:
	virtual ~allow_type_decay() noexcept {}

	template <bool b = !only_const>
	typename std::enable_if<b, DecayT*>::type operator->() {
		return &dynamic_cast<CRTP*>(this)->get();
	}
	const DecayT* operator->() const { return &dynamic_cast<const CRTP*>(this)->get(); }

	template <bool b = !only_const>
	typename std::enable_if<b, DecayT&>::type operator*() {
		return dynamic_cast<CRTP*>(this)->get();
	}
	const DecayT& operator*() const { return dynamic_cast<const CRTP*>(this)->get(); }
};

}
