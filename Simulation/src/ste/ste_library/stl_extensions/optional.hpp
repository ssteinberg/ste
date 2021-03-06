// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <copyable_moveale_base.hpp>
#include <types.hpp>

#include <type_traits>

#include <stdexcept>

namespace ste {

struct optional_ctor_inplace {};

class optional_bad_access_exception : public std::runtime_error {
	using Base = std::runtime_error;

public:
	using Base::Base;
	optional_bad_access_exception() : Base("") {}
};

namespace _detail {

template <typename T>
class optional_base {
	using storage_t = typename std::aligned_storage<sizeof(T)>::type;

protected:
	storage_t storage;
	bool has_val{ false };

	T& val() & {
		if (!has_val) {
			throw optional_bad_access_exception();
		}
		return *reinterpret_cast<T*>(&storage);
	}
	T&& val() && {
		if (!has_val) {
			throw optional_bad_access_exception();
		}
		return std::move(*reinterpret_cast<T*>(&storage));
	}
	const T& val() const& {
		if (!has_val) {
			throw optional_bad_access_exception();
		}
		return *reinterpret_cast<const T*>(&storage);
	}

	template <typename... Ts>
	void ctor_val(Ts&&... ts) {
		if (has_val)
			destroy_val();
		::new (&storage) T(std::forward<Ts>(ts)...);
		has_val = true;
	}

	template <typename S>
	void assign_val(S&& s) {
		if (has_val) {
			val() = std::forward<S>(s);
		}
		else {
			ctor_val(std::forward<S>(s));
		}
		has_val = true;
	}

	void destroy_val() noexcept(std::is_nothrow_destructible_v<T>) {
		val().~T();
		has_val = false;
	}

public:
	optional_base() = default;
	template <typename... Args>
	optional_base(optional_ctor_inplace,
			 Args&&... args) {
		ctor_val(std::forward<Args>(args)...);
	}

	optional_base(optional_base&&) = default;
	optional_base(const optional_base&) = default;
	optional_base &operator=(optional_base&&) = default;
	optional_base &operator=(const optional_base&) = default;

	// msvc 14.10.25017 (VC++ 2017 March release) fails with an internal error with conditional noexcept
//	virtual ~optional_base() noexcept(std::is_nothrow_destructible_v<T>) {
	virtual ~optional_base() noexcept {
		if (has_val)
			destroy_val();
	}
};

template <typename T>
class optional_base_moveable_copyable : public optional_base<T> {
	using Base = optional_base<T>;

public:
	using Base::Base;

	optional_base_moveable_copyable() = default;
	optional_base_moveable_copyable(optional_base_moveable_copyable &&other)
		noexcept(std::is_nothrow_move_constructible_v<T>)
	{
		if (other.has_val) {
			ctor_val(std::move(other.val()));
			other.destroy_val();
		}
	}
	optional_base_moveable_copyable &operator=(optional_base_moveable_copyable &&other)
		noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>)
	{
		if (other.has_val) {
			assign_val(std::move(other.val()));
			other.destroy_val();
		}
		else if (has_val) {
			destroy_val();
		}

		return *this;
	}
	optional_base_moveable_copyable(const optional_base_moveable_copyable &other)
		noexcept(std::is_nothrow_copy_constructible_v<T>)
	{
		if (other.has_val) {
			ctor_val(other.val());
		}
	}
	optional_base_moveable_copyable &operator=(const optional_base_moveable_copyable &other)
		noexcept(std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_copy_assignable_v<T>)
	{
		if (other.has_val) {
			ctor_val(other.val());
		}
		else if (has_val) {
			destroy_val();
		}

		return *this;
	}
};

template <typename T>
struct optional_copymove_base
	: optional_base_moveable_copyable<T>,
	copyable_moveale_base<T>
{
	using optional_base_moveable_copyable<T>::optional_base_moveable_copyable;
};

template <typename T>
using optional_ctor_base = optional_copymove_base<T>;

}

template <typename T>
class optional : public _detail::optional_ctor_base<T> {
	using Base = _detail::optional_ctor_base<T>;

public:
	using type = std::remove_reference_t<std::remove_pointer_t<T>>;

private:
	template <typename S>
	friend class optional;

	using reference = type&;
	using pointer = type*;
	using const_pointer = const type *;

private:
	template <typename S = T>
	std::enable_if_t<!std::is_pointer<S>::value, const std::remove_reference_t<S>&>
		get_val() const { return val(); }
	template <typename S = T>
	std::enable_if_t<std::is_pointer<S>::value, const std::remove_reference_t<std::remove_pointer_t<S>>&>
		get_val() const { return *val(); }
	template <typename S = T>
	std::enable_if_t<!std::is_pointer<S>::value, std::remove_reference_t<S>&>
		get_val() { return val(); }
	template <typename S = T>
	std::enable_if_t<std::is_pointer<S>::value, std::remove_reference_t<std::remove_pointer_t<S>>&>
		get_val() { return *val(); }

	template <typename S = T>
	std::enable_if_t<!std::is_pointer<S>::value, const std::remove_reference_t<S>*>
		get_ref() const { return &val(); }
	template <typename S = T>
	std::enable_if_t<std::is_pointer<S>::value, const std::remove_reference_t<S>>
		get_ref() const { return val(); }
	template <typename S = T>
	std::enable_if_t<!std::is_pointer<S>::value, std::remove_reference_t<S>*>
		get_ref() { return &val(); }
	template <typename S = T>
	std::enable_if_t<std::is_pointer<S>::value, std::remove_reference_t<S>>
		get_ref() { return val(); }

public:
	optional() {}
	optional(const none_t &) : optional() {}

	optional(optional<T> &&) = default;
	optional(const optional<T> &) = default;
	optional &operator=(optional<T> &&) = default;
	optional &operator=(const optional<T> &) = default;

	template <typename Placeholder, typename... Args>
	optional(std::enable_if_t<std::is_same_v<Placeholder, optional_ctor_inplace>, optional_ctor_inplace>,
			 Args&&... args)
		: Base(std::forward<Args>(args)...)
	{}

	template <typename S, typename =
		typename std::enable_if_t<std::is_constructible_v<T, S> || std::is_convertible_v<S, T>>
	>
	optional(optional<S> &&other) noexcept {
		if (other.has_val) {
			ctor_val(std::move(other.val()));
			other.destroy_val();
		}
	}
	template <typename S, typename = 
		typename std::enable_if_t<std::is_constructible_v<T, const S&> || std::is_convertible_v<const S&, T>>
	>
	optional(const optional<S> &other) {
		if (other.has_val) {
			ctor_val(other.val());
		}
	}

	template <typename S, typename =
		typename std::enable_if_t<std::is_constructible_v<T, S> || std::is_convertible_v<S, T>>
	>
	optional(S &&v) {
		ctor_val(std::move(v));
	}
	template <typename S, typename =
		typename std::enable_if_t<std::is_constructible_v<T, const S&> || std::is_convertible_v<const S&, T>>
	>
	optional(const S &v) {
		ctor_val(v);
	}

	template <typename S, typename =
		typename std::enable_if_t<(std::is_constructible_v<T, S> && std::is_assignable_v<T, S>) || std::is_convertible_v<S, T>>
	>
	optional &operator=(S&& v) {
		assign_val(std::move(v));
		return *this;
	}
	template <typename S, typename =
		typename std::enable_if_t<(std::is_constructible_v<T, const S&> && std::is_assignable_v<T, const S&>) || std::is_convertible_v<const S&, T>>
	>
	optional &operator=(const S &v) {
		assign_val(v);
		return *this;
	}

	template <typename S, typename =
		typename std::enable_if_t<(std::is_constructible_v<T, S> && std::is_assignable_v<T, S>) || std::is_convertible_v<S, T>>
	>
	optional &operator=(optional<S> &&other) noexcept {
		if (other.has_val) {
			assign_val(std::move(other.val()));
			other.destroy_val();
		}
		else if (has_val) {
			destroy_val();
		}

		return *this;
	}
	template <typename S, typename =
		typename std::enable_if_t<(std::is_constructible_v<T, const S&> && std::is_assignable_v<T, const S&>) || std::is_convertible_v<const S&, T>>
	>
	optional &operator=(const optional<S> &other) {
		if (other.has_val) {
			assign_val(other.val());
		}
		else if (has_val) {
			destroy_val();
		}

		return *this;
	}

	optional &operator=(none_t) {
		if (has_val)
			destroy_val();

		return *this;
	}

	~optional() noexcept {}

	template <typename ... Ts>
	void emplace(Ts&&... args) {
		ctor_val(std::forward<Ts>(args)...);
	}

	const std::remove_reference_t<T>& get() const& { return val(); }
	std::remove_reference_t<T>& get() & { return val(); }
	std::remove_reference_t<T>&& get() && { return std::move(*this).val(); }

	const reference operator*() const { return get_val(); }
	reference operator*() { return get_val(); }

	const_pointer operator->() const { return get_ref(); }
	pointer operator->() { return get_ref(); }

	explicit operator bool() const { return has_val; }
	bool operator!() const { return !has_val; }
};

template <typename T>
bool operator==(const optional<T> &opt, none_t) {
	return !opt;
}
template <typename T>
bool operator!=(const optional<T> &opt, none_t) {
	return !!opt;
}

template <typename T>
bool operator==(none_t, const optional<T> &opt) {
	return !opt;
}
template <typename T>
bool operator!=(none_t, const optional<T> &opt) {
	return !!opt;
}

template <typename T>
bool operator==(const optional<T> &opt, const T &t) {
	return opt.get() == t;
}
template <typename T>
bool operator!=(const optional<T> &opt, const T &t) {
	return opt.get() != t;
}
template <typename T>
bool operator==(const T &t, const optional<T> &opt) {
	return t == opt.get();
}
template <typename T>
bool operator!=(const T &t, const optional<T> &opt) {
	return t != opt.get();
}
template <typename T>
bool operator==(const optional<T> &opt0, const optional<T> &opt1) {
	if (!opt0 != !opt1)
		return false;
	if (!opt0 && !opt1)
		return true;
	return opt0.get() == opt1.get();
}
template <typename T>
bool operator!=(const optional<T> &opt0, const optional<T> &opt1) {
	return opt0.get() != opt1.get();
}

template <typename T>
bool operator<(const optional<T> &opt, const T &t) {
	return opt.get() < t;
}
template <typename T>
bool operator>(const optional<T> &opt, const T &t) {
	return opt.get() > t;
}
template <typename T>
bool operator<(const T &t, const optional<T> &opt) {
	return t < opt.get();
}
template <typename T>
bool operator>(const T &t, const optional<T> &opt) {
	return t > opt.get();
}
template <typename T>
bool operator<(const optional<T> &opt0, const optional<T> &opt1) {
	return opt0.get() < opt1.get();
}
template <typename T>
bool operator>(const optional<T> &opt0, const optional<T> &opt1) {
	return opt0.get() > opt1.get();
}

template <typename T>
bool operator<=(const optional<T> &opt, const T &t) {
	return opt.get() <= t;
}
template <typename T>
bool operator>=(const optional<T> &opt, const T &t) {
	return opt.get() >= t;
}
template <typename T>
bool operator<=(const T &t, const optional<T> &opt) {
	return t <= opt.get();
}
template <typename T>
bool operator>=(const T &t, const optional<T> &opt) {
	return t >= opt.get();
}
template <typename T>
bool operator<=(const optional<T> &opt0, const optional<T> &opt1) {
	return opt0.get() <= opt1.get();
}
template <typename T>
bool operator>=(const optional<T> &opt0, const optional<T> &opt1) {
	return opt0.get() >= opt1.get();
}

template <typename T>
auto operator+(const optional<T> &opt, const T &t) {
	return opt.get() + t;
}
template <typename T>
auto operator-(const optional<T> &opt, const T &t) {
	return opt.get() - t;
}
template <typename T>
auto operator*(const optional<T> &opt, const T &t) {
	return opt.get() * t;
}
template <typename T>
auto operator/(const optional<T> &opt, const T &t) {
	return opt.get() / t;
}
template <typename T>
auto operator+=(const optional<T> &opt, const T &t) {
	return opt.get() += t;
}
template <typename T>
auto operator-=(const optional<T> &opt, const T &t) {
	return opt.get() -= t;
}
template <typename T>
auto operator*=(const optional<T> &opt, const T &t) {
	return opt.get() *= t;
}
template <typename T>
auto operator/=(const optional<T> &opt, const T &t) {
	return opt.get() /= t;
}

template <typename T>
auto operator+(const T &t, const optional<T> &opt) {
	return t + opt.get();
}
template <typename T>
auto operator-(const T &t, const optional<T> &opt) {
	return t - opt.get();
}
template <typename T>
auto operator*(const T &t, const optional<T> &opt) {
	return t * opt.get();
}
template <typename T>
auto operator/(const T &t, const optional<T> &opt) {
	return t / opt.get();
}
template <typename T>
auto operator+=(const T &t, const optional<T> &opt) {
	return t += opt.get() ;
}
template <typename T>
auto operator-=(const T &t, const optional<T> &opt) {
	return t -= opt.get() ;
}
template <typename T>
auto operator*=(const T &t, const optional<T> &opt) {
	return t *= opt.get() ;
}
template <typename T>
auto operator/=(const T &t, const optional<T> &opt) {
	return t /= opt.get() ;
}


template <typename T, typename... Ts>
auto make_optional(Ts&&... ts) {
	return optional<T>(T(std::forward<Ts>(ts)...));
}

template <typename T>
auto make_optional(T&& t) {
	return optional<T>(std::forward<T>(t));
}

template <typename To, typename From>
auto optional_dynamic_cast(const optional<From> &from) {
	static_assert(std::is_pointer_v<From>, "From must be a pointer type");
	static_assert(std::is_pointer_v<To>, "To must be a pointer type");

	if (from)
		return optional<To>(dynamic_cast<To>(from.get()));
	return optional<To>();
}

}
