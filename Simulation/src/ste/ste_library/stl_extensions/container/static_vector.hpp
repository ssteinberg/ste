// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <type_traits>
#include <memory>
#include <cassert>
#include <vector>
#include <algorithm>
#include <utility>

namespace ste {

template <typename T, template <class> typename Allocator = std::allocator>
class static_vector {
public:
	using allocator_type = Allocator<T>;
	using value_type = typename allocator_type::value_type;
	using reference = typename allocator_type::reference;
	using const_reference = typename allocator_type::const_reference;
	using pointer = typename allocator_type::pointer;
	using const_pointer = typename allocator_type::const_pointer;
	using difference_type = typename allocator_type::difference_type;
	using size_type = typename allocator_type::size_type;

private:
	template <typename value_type, typename reference, typename pointer>
	class iterator_impl {
		friend static_vector;

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
		difference_type operator-(const iterator_impl &it) const { return static_cast<difference_type>(idx) - static_cast<difference_type>(it.idx); }

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
	struct default_alignment { constexpr static int align = 8; };
	struct T_alignment { constexpr static int align = alignof(T); };
	using align = std::conditional_t <
		default_alignment::align < T_alignment::align,
		T_alignment,
		default_alignment
		>;

	using storage_t = typename std::aligned_storage<sizeof(T), align::align>::type;

	constexpr auto storage_elements(size_type n) {
		static constexpr auto S = sizeof(storage_t);
		return static_cast<size_type>((n * sizeof(T) + S - 1) / S);
	}

private:
	storage_t* storage{ nullptr };
	// Use std::vector<bool> efficient specialization
	std::vector<bool, Allocator<bool>> allocated;

public:
	static_vector(size_type size)
		: storage(Allocator<storage_t>().allocate(storage_elements(size)))
	{
		allocated.resize(size, false);
	}
	template <typename... Ts>
	static_vector(size_type size, Ts&&... ts)
		: static_vector(size)
	{
		for (auto it = begin(); it != end(); ++it)
			emplace(it, std::forward<Ts>(ts)...);
	}
	~static_vector() noexcept(std::is_nothrow_destructible_v<T>) {
		if (!storage)
			return;

		for (auto it = begin(); it != end(); ++it) {
			if (is_valid(it))
				destroy(it);
		}

		auto n = storage_elements(size());

		Allocator<storage_t>().deallocate(storage, n);
	}

	static_vector(static_vector &&o) noexcept : storage(o.storage), allocated(std::move(o.allocated)) {
		o.storage = nullptr;
	}
	static_vector &operator=(static_vector &&o) noexcept {
		storage = o.storage;
		allocated = std::move(o.allocated);
		o.storage = nullptr;

		return *this;
	}

	size_type size() const { return allocated.size(); }

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

	template <typename... Ts>
	void emplace(const_iterator it, Ts&&... ts) {
		assert(it.idx < size());

		if (allocated[it.idx])
			destroy(it);

		::new (&at(it.idx)) T(std::forward<Ts>(ts)...);
		allocated[it.idx] = true;
	}
	void destroy(const_iterator it) {
		assert(it.idx < size() && allocated[it.idx]);

		data()[it.idx].~T();
		allocated[it.idx] = false;
	}
	bool is_valid(const_iterator it) const {
		return allocated[it.idx];
	}

	void swap(static_vector& o) noexcept {
		std::swap(storage, o.storage);
		std::swap(allocated, o.allocated);
	}
};

}
