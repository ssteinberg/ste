// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <range.hpp>
#include <deque>

#include <initializer_list>

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

	void split(typename list_t::iterator it, 
			   const T &splitter) {
		auto itend = it->start + it->length;
		auto end = splitter.start + splitter.length;
		auto next = std::next(it);

		if (end <= it->start)
			return;
		if (itend <= splitter.start)
			return;

		// Splitter overlaps it

		if (splitter.start <= it->start)
			// Overlap start
			list.erase(it);
		else
			// splitter.start > it->start
			it->length = splitter.start - it->start;

		if (end < itend)
			list.insert(next,
						T(end, itend - end));
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
	void remove(const T &r) {
		auto rend = r.start + r.length;
		for (auto it = list.begin(); it != list.end() && it->start < rend;) {
			auto split_it = it;
			++it;
			split(split_it, r);
		}
	}
	/**
	*	@brief	Remove range
	*/
	void remove(typename list_t::iterator it) {
		list.erase(it);
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
	auto front() const { return list.front(); }
	auto back() const { return list.back(); }

	auto size() const { return list.size(); }
};

template <typename T = std::size_t, class Allocator = std::allocator<T>>
using range_list = range_list_custom<range<T>>;

}
