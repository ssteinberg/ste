// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <functional>
#include <memory>

#include <types.hpp>

#include <functional>
#include <type_traits>

namespace StE {

template <typename T>
class optional {
public:
	using type = std::remove_reference_t<std::remove_pointer_t<T>>;

private:
	template <typename S>
	friend class optional;

	using reference = type&;
	using pointer = type*;
	using const_pointer = type const*;

private:
	bool has_val{ false };
	T val;

	template <typename S = T>
	std::enable_if_t<!std::is_pointer<S>::value, const std::remove_reference_t<S>&>
		get_val() const { return val; }
	template <typename S = T>
	std::enable_if_t<std::is_pointer<S>::value, const std::remove_reference_t<std::remove_pointer_t<S>>&>
		get_val() const { return *val; }
	template <typename S = T>
	std::enable_if_t<!std::is_pointer<S>::value, std::remove_reference_t<S>&>
		get_val() { return val; }
	template <typename S = T>
	std::enable_if_t<std::is_pointer<S>::value, std::remove_reference_t<std::remove_pointer_t<S>>&>
		get_val() { return *val; }

	template <typename S = T>
	std::enable_if_t<!std::is_pointer<S>::value, const std::remove_reference_t<S>*>
		get_ref() const { return &val; }
	template <typename S = T>
	std::enable_if_t<std::is_pointer<S>::value, const std::remove_reference_t<S>>
		get_ref() const { return val; }
	template <typename S = T>
	std::enable_if_t<!std::is_pointer<S>::value, std::remove_reference_t<S>*>
		get_ref() { return &val; }
	template <typename S = T>
	std::enable_if_t<std::is_pointer<S>::value, std::remove_reference_t<S>>
		get_ref() { return val; }

public:
	optional() {}
	optional(const none_t &) : optional() {}

	template <typename S, typename = 
		typename std::enable_if_t<std::is_pointer<T>::value && std::is_pointer<S>::value && std::is_base_of<std::remove_pointer_t<S>, std::remove_pointer_t<T>>::value>>
	optional(optional<S> &&other) noexcept : has_val(other.has_val), val(other.has_val ? dynamic_cast<T>(other.val) : T()) {
		other.has_val = false;
	}
	template <typename S, typename =
		typename std::enable_if_t<std::is_pointer<T>::value && std::is_pointer<S>::value && std::is_base_of<std::remove_pointer_t<S>, std::remove_pointer_t<T>>::value>>
	optional(const optional<S> &other) : has_val(other.has_val), val(other.has_val ? dynamic_cast<T>(other.val) : T()) {}

	optional(optional<T> &&other) noexcept : has_val(other.has_val), val(std::move(other.val)) {
		other.has_val = false;
	}
	optional(const optional<T> &other) : has_val(other.has_val), val(other.val) {}

	optional(const T &v) : has_val(true), val(v) {}
	template <typename S = T, typename = typename std::enable_if_t<std::is_move_constructible<S>::value>>
	optional(T &&v) : has_val(true), val(std::forward<T>(v)) {}

	template <typename S, typename =
		typename std::enable_if_t < std::is_pointer<T>::value && std::is_pointer<S>::value && std::is_base_of<std::remove_pointer_t<S>, std::remove_pointer_t<T>>::value >>
	optional &operator=(optional<S> &&other) noexcept {
		has_val = other.has_val;
		val = has_val ? dynamic_cast<T>(other.val) : T();
		other.has_val = false;
		return *this;
	}
	template <typename S, typename =
		typename std::enable_if_t < std::is_pointer<T>::value && std::is_pointer<S>::value && std::is_base_of<std::remove_pointer_t<S>, std::remove_pointer_t<T>>::value >>
	optional &operator=(const optional<S> &other) {
		has_val = other.has_val;
		val = has_val ? dynamic_cast<T>(other.val) : T();
		return *this;
	}

	template <typename S,
		typename = typename std::enable_if_t<!std::is_same_v<std::remove_cv_t<S>, optional<T>>>>
	optional &operator=(S&& v) {
		has_val = true;
		val = T(std::forward<S>(v));
		return *this;
	}

	optional &operator=(optional<T> &&other) noexcept {
		has_val = other.has_val;
		val = std::move(other.val);
		other.has_val = false;
		return *this;
	}
	optional &operator=(const optional<T> &other) {
		has_val = other.has_val;
		val = other.val;
		return *this;
	}

	optional &operator=(none_t) {
		has_val = false;
		val = T();
		return *this;
	}

	bool operator==(none_t) const {
		return !has_val;
	}
	bool operator!=(none_t) const {
		return has_val;
	}

	template <typename ... Ts>
	void emplace(Ts&&... args) {
		has_val = true;
		val = T(std::forward<Ts>(args)...);
	}

	const std::remove_reference_t<T>& get() const { return val; }
	std::remove_reference_t<T>& get() { return val; }

	const reference operator*() const { return get_val(); }
	reference operator*() { return get_val(); }

	const_pointer operator->() const { return get_ref(); }
	pointer operator->() { return get_ref(); }

	explicit operator bool() const { return has_val; }
	bool operator!() const { return !has_val; }
};

}
