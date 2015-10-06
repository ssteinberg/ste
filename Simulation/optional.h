// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <functional>

#include "types.h"

namespace StE {

template <typename T>
class optional {
private:
	T val;
	bool has_val{ false };

public:
	optional() {}
	optional(const none_t &) : optional() {}
	template <typename S>
	optional(optional<S> &&other) : has_val(other.has_val), val(std::move(has_val ? other.val : T())) {}
	template <typename S>
	optional(const optional<S> &other) : has_val(other.has_val), val(has_val ? other.val : T()) {}
	optional(const T &v) : has_val(true), val(v) {}

	template <typename S>
	optional &operator=(optional<S> &&other) {
		has_val = other.has_val;
		val = std::move(has_val ? other.val : T());
	}
	template <typename S>
	optional &operator=(const optional<S> &other) {
		has_val = other.has_val;
		val = has_val ? other.val : T();
	}

	template <typename S>
	optional &operator=(S &&v) {
		has_val = true;
		val = std::move(v);
	}
	template <typename S>
	optional &operator=(const S &v) {
		has_val = true;
		val = v;
	}

	template <typename ... Ts>
	void emplace(Ts&&... args) {
		has_val = true;
		val = T(std::forward<Ts>(args)...);
	}

	const T& get() const { return val; }
	T& get() { return val; }

	const T& operator*() const { return val; }
	T& operator*() { return val; }

	const T* operator->() const { return &val; }
	T* operator->() { return &val; }

	explicit operator bool() const { return has_val; }
	bool operator!() const { return !has_val; }
};

}
