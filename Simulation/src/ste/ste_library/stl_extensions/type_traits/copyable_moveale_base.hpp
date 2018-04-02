// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

namespace ste {

template <bool b>
struct move_ctor_deleter {};
template <>
struct move_ctor_deleter<false> {
	move_ctor_deleter() = default;
	move_ctor_deleter(const move_ctor_deleter&) = default;
	move_ctor_deleter &operator=(move_ctor_deleter&&) = default;
	move_ctor_deleter &operator=(const move_ctor_deleter&) = default;
	move_ctor_deleter(move_ctor_deleter&&) = delete;
};

template <bool b>
struct move_assign_deleter {};
template <>
struct move_assign_deleter<false> {
	move_assign_deleter() = default;
	move_assign_deleter(move_assign_deleter&&) = default;
	move_assign_deleter(const move_assign_deleter&) = default;
	move_assign_deleter &operator=(const move_assign_deleter&) = default;
	move_assign_deleter &operator=(move_assign_deleter&&) = delete;
};

template <bool b>
struct copy_ctor_deleter {};
template <>
struct copy_ctor_deleter<false> {
	copy_ctor_deleter() = default;
	copy_ctor_deleter(copy_ctor_deleter&&) = default;
	copy_ctor_deleter &operator=(copy_ctor_deleter&&) = default;
	copy_ctor_deleter &operator=(const copy_ctor_deleter&) = default;
	copy_ctor_deleter(const copy_ctor_deleter&) = delete;
};

template <bool b>
struct copy_assign_deleter {};
template <>
struct copy_assign_deleter<false> {
	copy_assign_deleter() = default;
	copy_assign_deleter(copy_assign_deleter&&) = default;
	copy_assign_deleter(const copy_assign_deleter&) = default;
	copy_assign_deleter &operator=(copy_assign_deleter&&) = default;
	copy_assign_deleter &operator=(const copy_assign_deleter&) = delete;
};

template <typename T>
struct copyable_moveale_base
	: move_ctor_deleter<std::is_move_constructible_v<T>>,
	copy_ctor_deleter<std::is_copy_constructible_v<T>>,
	move_assign_deleter<std::is_move_assignable_v<T>>,
	copy_assign_deleter<std::is_copy_assignable_v<T>>
{
	copyable_moveale_base() = default;
	copyable_moveale_base(copyable_moveale_base&&) = default;
	copyable_moveale_base(const copyable_moveale_base&) = default;
	copyable_moveale_base &operator=(copyable_moveale_base&&) = default;
	copyable_moveale_base &operator=(const copyable_moveale_base&) = default;
};

}
