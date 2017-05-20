// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <type_traits>
#include <memory>
#include <cassert>
#include <algorithm>
#include <utility>
#include <cstring>

namespace ste {

namespace _detail {

template <typename T, typename Allocator>
class static_vector_impl {
public:
	using allocator_type = Allocator;
	using value_type = typename allocator_type::value_type;
	using reference = typename allocator_type::reference;
	using const_reference = typename allocator_type::const_reference;
	using pointer = typename allocator_type::pointer;
	using const_pointer = typename allocator_type::const_pointer;
	using difference_type = typename allocator_type::difference_type;
	using size_type = typename allocator_type::size_type;

	static_assert(allocator_type::propagate_on_container_move_assignment::value, "Allocator must specify propagate_on_container_move_assignment as std::true_type to qualify for static_vector.");

private:
	template <typename value_type, typename reference, typename pointer>
	class iterator_impl {
		friend static_vector_impl;

	private:
		size_type idx{ 0 };
		value_type *v{ nullptr };

		iterator_impl(size_type idx, value_type *v) : idx(idx), v(v) {}

	public:
		using difference_type = typename allocator_type::difference_type;
		typedef std::random_access_iterator_tag iterator_category;

		iterator_impl() = default;
		iterator_impl(iterator_impl&&) = default;
		iterator_impl(const iterator_impl&) = default;
		iterator_impl& operator=(iterator_impl&&) = default;
		iterator_impl& operator=(const iterator_impl&) = default;

		template <
			typename val = value_type,
			typename ref = typename allocator_type::reference,
			typename ptr = typename allocator_type::pointer,
			typename = typename std::enable_if<!std::is_same_v<ref, reference>>::type
		>
		iterator_impl(const iterator_impl<val, ref, ptr> &o)
			: idx(o.idx), v(o.v)
		{}

		~iterator_impl() noexcept {}

		bool operator==(const iterator_impl&o) const {
			return v == o.v && idx == o.idx;
		}
		bool operator!=(const iterator_impl&o) const {
			return !(*this == o);
		}
		bool operator<(const iterator_impl&o) const {
			return v == o.v && idx < o.idx;
		}
		bool operator>(const iterator_impl&o) const {
			return v == o.v && idx > o.idx;
		}
		bool operator<=(const iterator_impl&o) const {
			return v == o.v && idx <= o.idx;
		}
		bool operator>=(const iterator_impl&o) const {
			return v == o.v && idx >= o.idx;
		}

		iterator_impl& operator++() {
			++idx;
			return *this;
		}
		iterator_impl operator++(int) {
			auto it = *this;
			++(*this);
			return it;
		}
		iterator_impl& operator--() {
			--idx;
			return *this;
		}
		iterator_impl operator--(int) {
			auto it = *this;
			--(*this);
			return it;
		}
		iterator_impl& operator+=(size_type n) {
			idx += n;
			return *this;
		}
		iterator_impl operator+(size_type n) const { return { idx + n,v }; }
		friend iterator_impl operator+(size_type n, const iterator_impl &it) { return { it.idx + n, it.v }; }
		iterator_impl& operator-=(size_type n) {
			idx -= n;
			return *this;
		}
		iterator_impl operator-(size_type n) const { return { idx + n,v }; }
		difference_type operator-(const iterator_impl &it) const { return static_cast<difference_type>(idx - it.idx); }

		reference operator*() const { return *(v + idx); }
		pointer operator->() const { return v + idx; }
		reference operator[](size_type n) const { return *(v + idx + n); }
	};

public:
	using iterator = iterator_impl<value_type, reference, pointer>;
	using const_iterator = iterator_impl<const value_type, const_reference, const_pointer>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
	using storage_allocator_type = typename Allocator::template rebind<std::uint8_t>::other;

protected:
	static constexpr std::size_t pad_data_to_alignment = 64;
	static constexpr std::size_t pad_bitvector_to_alignment = sizeof(std::size_t);

	constexpr auto storage_bitvector_offset(size_type size) {
		return (size * sizeof(T) + pad_data_to_alignment - 1) / pad_data_to_alignment * pad_data_to_alignment;
	}
	constexpr auto storage_bitvector_size(size_type size) {
		return ((size + 7) / 8 + pad_bitvector_to_alignment - 1) / pad_bitvector_to_alignment * pad_bitvector_to_alignment;
	}
	constexpr auto storage_size(size_type size) {
		auto data_size = storage_bitvector_offset(size);
		auto bitvector_size = storage_bitvector_size(size);
		return data_size + bitvector_size;
	}

	void _create(size_type size) {
		auto bytes = storage_size(size);
		auto bitvector_offset = storage_bitvector_offset(size);

		auto ptr = storage_allocator.allocate(bytes);

		storage = reinterpret_cast<T*>(ptr);
		bitvector = reinterpret_cast<std::size_t*>(ptr + bitvector_offset);

		auto bitvector_size = storage_bitvector_size(size);
		std::memset(reinterpret_cast<std::size_t*>(bitvector), 0, bitvector_size);
	}

	void _destroy() {
		if (!storage)
			return;

		for (auto it = begin(); it != end(); ++it) {
			if (is_valid(it))
				destroy(it);
		}

		storage_allocator.deallocate(reinterpret_cast<std::uint8_t*>(storage), 1);		// Elements count is not important, we don't need to destruct.
	}

	void _move(static_vector_impl &&o) {
		this->storage = o.storage;
		this->bitvector = o.bitvector;
		this->count = o.count;

		o.storage = nullptr;
	}
	template <typename U = T>
	void _copy(static_vector_impl &&o, std::enable_if_t<!std::is_copy_constructible_v<U>>* = nullptr) {
		this->count = o.count;
		_create(count);

		for (int idx = 0; idx < count; ++idx) {
			auto oit = o.begin() + idx;
			if (o.is_valid(oit))
				emplace(begin() + idx, *oit);
		}

		auto bitvector_size = storage_bitvector_size(count);
		std::memcpy(this->bitvector, o.bitvector, bitvector_size);
	}

	void set_bit(size_type i) {
		auto idx = i / (8 * sizeof(std::size_t));
		auto offset = i % (8 * sizeof(std::size_t));
		bitvector[idx] |= static_cast<std::size_t>(1) << offset;
	}
	void reset_bit(size_type i) {
		auto idx = i / (8 * sizeof(std::size_t));
		auto offset = i % (8 * sizeof(std::size_t));
		bitvector[idx] &= ~(static_cast<std::size_t>(1) << offset);
	}

	bool _is_bitset(size_type i) const {
		auto idx = i / (8 * sizeof(std::size_t));
		auto offset = i % (8 * sizeof(std::size_t));
		return (bitvector[idx] >> offset) & static_cast<std::size_t>(1);
	}

protected:
	T* storage{ nullptr };
	std::size_t* bitvector;
	std::size_t count;

	storage_allocator_type storage_allocator;

public:
	static_vector_impl(size_type size) : count(size) {
		_create(size);
	}
	template <typename... Ts>
	static_vector_impl(size_type size, Ts&&... ts)
		: static_vector_impl(size)
	{
		for (auto it = begin(); it != end(); ++it)
			emplace(it, std::forward<Ts>(ts)...);
	}
//	virtual ~static_vector_impl() noexcept(std::is_nothrow_destructible_v<T>) {
	virtual ~static_vector_impl() noexcept {
		_destroy();
	}

	static_vector_impl(static_vector_impl &&o) noexcept {
		_move(std::move(o));
	}
	static_vector_impl(const static_vector_impl &o) {
		_copy(o);
	}
	static_vector_impl &operator=(static_vector_impl &&o) noexcept {
		_destroy();
		_move(std::move(o));

		return *this;
	}
	static_vector_impl &operator=(const static_vector_impl &o) {
		_destroy();
		_copy(o);

		return *this;
	}

	size_type size() const { return count; }

	pointer data() { return reinterpret_cast<pointer>(storage); }
	const_pointer data() const { return reinterpret_cast<const_pointer>(storage); }
	reference operator[](int i) { return *reinterpret_cast<pointer>(data() + i); }
	const_reference operator[](int i) const { return *reinterpret_cast<const_pointer>(data() + i); }
	reference at(size_type i) { return data()[i]; }
	const_reference at(size_type i) const { return data()[i]; }

	iterator begin() { return iterator(0, data()); }
	const_iterator begin() const { return const_iterator(0, data()); }
	const_iterator cbegin() const { return const_iterator(0, data()); }
	iterator end() { return iterator(size(), data()); }
	const_iterator end() const { return const_iterator(size(), data()); }
	const_iterator cend() const { return const_iterator(size(), data()); }
	reverse_iterator rbegin() { return reverse_iterator(0, data()); }
	const_reverse_iterator rbegin() const { return const_reverse_iterator(0, data()); }
	const_reverse_iterator crbegin() const { return const_reverse_iterator(0, data()); }
	reverse_iterator rend() { return reverse_iterator(size(), data()); }
	const_reverse_iterator rend() const { return const_reverse_iterator(size(), data()); }
	const_reverse_iterator crend() const { return const_reverse_iterator(size(), data()); }

	reference front() { return data()[0]; }
	const_reference front() const { return data()[0]; }
	reference back() { return data()[size() - 1]; }
	const_reference back() const { return data()[size() - 1]; }

	void swap(static_vector_impl& o) noexcept {
		std::swap(storage, o.storage);
		std::swap(bitvector, o.bitvector);
		std::swap(count, o.count);
	}

	template <typename... Ts>
	void emplace(const_iterator it, Ts&&... ts) {
		assert(it.idx < count);

		if (is_valid(it))
			destroy(it);

		::new (&at(it.idx)) T(std::forward<Ts>(ts)...);
		set_bit(it.idx);
	}
	void destroy(const_iterator it) {
		assert(it.idx < count && is_valid(it));

		at(it.idx).~T();
		reset_bit(it.idx);
	}

	bool is_valid(const_iterator it) const {
		return _is_bitset(it.idx);
	}
};

}

template <typename T, typename Allocator = std::allocator<T>>
class static_vector : public _detail::static_vector_impl<T, Allocator> {
	using Base = _detail::static_vector_impl<T, Allocator>;

public:
	using Base::Base;

	static_vector(static_vector&&o) = default;
	static_vector &operator=(static_vector&&) = default;
	static_vector(const static_vector&) = default;
	static_vector &operator=(const static_vector&) = default;
};

}
