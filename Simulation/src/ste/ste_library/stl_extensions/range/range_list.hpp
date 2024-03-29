//	StE
// � Shlomi Steinberg 2015-2017

#pragma once

#include <range.hpp>
#include <deque>

#include <initializer_list>
#include <algorithm>

namespace ste {

enum class range_list_overlap {
	none,
	full,
	partial,
};

template <typename T, class Allocator = std::allocator<T>>
class range_list_custom {
private:
	using list_t = std::deque<T, Allocator>;

public:
	using value_type = T;

private:
	list_t list;

private:
	void forward_merge(typename list_t::iterator start) {
		auto rend = start->start + start->length;
		auto it = start;
		++it;

		while (it != list.end() &&
			   it->start <= rend) {
			start->length = it->start + it->length - start->start;
			it = list.erase(it);
		}
	}

	auto split(typename list_t::iterator it, 
			   const T &splitter) {
		auto itend = it->start + it->length;
		auto end = splitter.start + splitter.length;
		auto next = std::next(it);

		if (end <= it->start)
			return next;
		if (itend <= splitter.start)
			return next;

		// Splitter overlaps it

		if (splitter.start <= it->start)
			// Overlap start
			next = list.erase(it);
		else
			// splitter.start > it->start
			it->length = splitter.start - it->start;

		if (end < itend)
			next = list.insert(next,
							   T(end, itend - end));

		return next;
	}

public:
	range_list_custom() = default;
	range_list_custom(const std::initializer_list<T> &init) {
		for (auto &e : init)
			add(e);
	}

	/**
	 *	@brief	Checks overlap of a range
	 *	
	 *	@return	range_list_overlap::none		On no overlap
	 *			range_list_overlap::full		On full overlap
	 *			range_list_overlap::partial	On partial overlap
	 */
	auto check(T r) const {
		auto rend = r.start + r.length;
		for (auto it = list.begin(); it != list.end() && it->start < rend; ++it) {
			auto itend = it->start + it->length;

			if (itend < r.start)
				continue;

			// There is overlap
			// Check if it is full
			for (; it != list.end() && it->start < rend; ++it) {
				if (it->start > r.start)
					break;
				r.start = itend;
				r.legnth = rend - r.start;
			}

			return r.length == 0 ?
				range_list_overlap::full :
				range_list_overlap::partial;
		}

		return range_list_overlap::none;
	}

	/**
	*	@brief	Remove range
	*/
	auto remove(const T &r) {
		auto rend = r.start + r.length;
		auto it = std::lower_bound(begin(), end(), r);
		while (it != list.end() && it->start < rend)
			it = split(it, r);
		return it;
	}
	/**
	*	@brief	Remove range
	*/
	auto remove(typename list_t::iterator it) {
		return list.erase(it);
	}

	void pop_back() {
		list.pop_back();
	}
	void pop_front() {
		list.pop_front();
	}

	/**
	 *	@brief	Add range
	 */
	void add(const T &r) {
		auto rend = r.start + r.length;
		for (auto it = list.begin(); it != list.end(); ++it) {
			auto itend = it->start + it->length;

			if (rend < it->start) {
				// No overlap
				list.insert(it, r);
				return;
			}
			if (r.start < it->start &&
				rend >= it->start) {
				if (rend > itend) {
					// It contained in r
					*it = r;
				}
				else {
					// Overlap start
					it->start = r.start;
					it->length = itend - it->start;
				}
				forward_merge(it);

				return;
			}
			if (r.start <= itend) {
				if (rend > itend) {
					// Overlap end
					it->length = rend - it->start;
					forward_merge(it);
				}

				return;
			}

			// Continue if r.start > itend
		}

		// Inset at end
		list.push_back(r);
	}

	auto begin() const { return list.begin(); }
	auto end() const { return list.end(); }
	auto& front() const { return list.front(); }
	auto& back() const { return list.back(); }
	auto begin() { return list.begin(); }
	auto end() { return list.end(); }
	auto& front() { return list.front(); }
	auto& back() { return list.back(); }

	auto size() const { return list.size(); }
};

template <typename T = std::size_t, class Allocator = std::allocator<T>>
using range_list = range_list_custom<range<T>>;

}
