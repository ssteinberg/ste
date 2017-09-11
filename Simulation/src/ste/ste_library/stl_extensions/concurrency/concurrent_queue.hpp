// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <atomic>
#include <memory>
#include <ref_count_ptr.hpp>

namespace ste {

/*
*	@brief	Concurrent lock-free queue.
*	
*			Based on Anthony William's "C++ Concurrency in Action".
*/
template <
	typename T, 
	typename Allocator = std::allocator<T>, 
	typename PointerType = std::unique_ptr<T>
>
class concurrent_queue {
public:
	using allocator_type = Allocator;
	using stored_ptr = PointerType;

private:
	struct node;
	struct node_counter;

	using counted_node_ptr = ref_count_ptr<node*>;

	struct node_counter {
		unsigned internal_count : 30;
		unsigned external_counters : 2;
	};

	struct node {
		std::atomic<node_counter> count;
		std::atomic<counted_node_ptr> next;

		std::atomic<T*> data;

		node() : data(nullptr) {
			node_counter new_counter;
			new_counter.internal_count = 0;
			new_counter.external_counters = 2;
			count.store(new_counter);

			counted_node_ptr new_counted_node_ptr{ 0, nullptr };
			next.store(new_counted_node_ptr);
		}

		// Returns false if node needs to be deallocated.
		bool release_ref() {
			node_counter old_counter = count.load(std::memory_order_relaxed);
			node_counter new_counter;
			do {
				new_counter = old_counter;
				--new_counter.internal_count;
			} while (!count.compare_exchange_strong(old_counter, new_counter, std::memory_order_acquire, std::memory_order_relaxed));

			return new_counter.internal_count || new_counter.external_counters;
		}
	};

	using node_allocator_type = typename Allocator::template rebind<node>::other;

	node* allocate_node() {
		auto ptr = node_allocator.allocate(1);
		::new (ptr) node();

		return ptr;
	}

	void deallocate_node(node *ptr) {
		ptr->~node();
		node_allocator.deallocate(ptr, 1);
	}

protected:
	std::atomic<counted_node_ptr> head, tail;
	allocator_type allocator;
	node_allocator_type node_allocator;

	static void increase_external_count(std::atomic<counted_node_ptr> &counter, counted_node_ptr &old_counter) {
		counted_node_ptr new_counter;
		do {
			new_counter = old_counter;
			new_counter.inc();
		} while (!counter.compare_exchange_strong(old_counter, new_counter, std::memory_order_acquire, std::memory_order_relaxed));

		old_counter.set_counter(new_counter.get_counter());
	}

	void free_external_counter(counted_node_ptr &old_node_ptr) {
		node * const ptr = old_node_ptr.get();
		int count_increase = old_node_ptr.get_counter() - 2;

		node_counter old_counter = ptr->count.load(std::memory_order_relaxed);
		node_counter new_counter;
		do {
			new_counter = old_counter;
			--new_counter.external_counters;
			new_counter.internal_count += count_increase;
		} while (!ptr->count.compare_exchange_strong(old_counter, new_counter, std::memory_order_acquire, std::memory_order_relaxed));

		if (!new_counter.internal_count && !new_counter.external_counters)
			deallocate_node(ptr);
	}

	void set_new_tail(counted_node_ptr &old_tail, counted_node_ptr const &new_tail) {
		node * const current_tail_ptr = old_tail.get();
		while (!tail.compare_exchange_weak(old_tail, new_tail) && old_tail.get() == current_tail_ptr) {}

		if (old_tail.get() == current_tail_ptr) {
			free_external_counter(old_tail);
			return;
		}
		
		if (!current_tail_ptr->release_ref())
			deallocate_node(current_tail_ptr);
	}

public:
	concurrent_queue() {
		counted_node_ptr new_counted_node{ 1, allocate_node() };

		head.store(new_counted_node);
		tail.store(new_counted_node);

		assert(head.is_lock_free() && "head/tail not lock free!");
		assert(new_counted_node.get()->count.is_lock_free() && "count not lock free!");
		assert(new_counted_node.get()->next.is_lock_free() && "next not lock free!");
	}
	~concurrent_queue() {
		while (pop() != nullptr) {}
		deallocate_node(head.load().get());
	}

	concurrent_queue(const concurrent_queue &q) = delete;
	concurrent_queue& operator=(const concurrent_queue &q) = delete;

	stored_ptr pop() {
		counted_node_ptr old_head = head.load(std::memory_order_relaxed);
		for (;;) {
			increase_external_count(head, old_head);
			node * const ptr = old_head.get();
			if (ptr == tail.load().get())
				return nullptr;

			counted_node_ptr next = ptr->next.load();
			if (head.compare_exchange_strong(old_head, next)) {
				T * const res = ptr->data.exchange(nullptr);
				free_external_counter(old_head);
				return stored_ptr(res);
			}

			if (!ptr->release_ref())
				deallocate_node(ptr);
		}
	}

	void push(T &&new_value) {
		auto ptr = allocator.allocate(1);
		::new (ptr) T(std::move(new_value));

		stored_ptr new_data(ptr);

		push(std::move(new_data));
	}

	void push(stored_ptr &&new_data) {
		counted_node_ptr new_next{ 1, allocate_node() };
		counted_node_ptr old_tail = tail.load();
		for (;;) {
			increase_external_count(tail, old_tail);
			T* old_data = nullptr;
			if (old_tail.get()->data.compare_exchange_strong(old_data, new_data.get())) {
				counted_node_ptr old_next = { 0, nullptr };
				if (!old_tail.get()->next.compare_exchange_strong(old_next, new_next)) {
					deallocate_node(new_next.get());
					new_next = old_next;
				}
				set_new_tail(old_tail, new_next);
				new_data.release();
				break;
			}
			else {
				counted_node_ptr old_next = { 0, nullptr };
				if (old_tail.get()->next.compare_exchange_strong(old_next, new_next)) {
					old_next = new_next;
					new_next.set(allocate_node());
				}
				set_new_tail(old_tail, old_next);
			}
		}
	}

	bool is_empty_hint(std::memory_order order = std::memory_order_acquire) const {
		counted_node_ptr h = head.load(order);
		counted_node_ptr t = tail.load(order);
		return h.get() == t.get();
	}
};

}
