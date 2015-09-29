// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <atomic>
#include <vector>
#include <type_traits>

namespace StE {

template <typename T, int N = 3>
class concurrent_pointer_recycler {
private:
	std::array<std::atomic<T*>, N> pointers;

	static_assert(std::is_move_assignable<T>::value, "T must be move-assignable");

public:
	~concurrent_pointer_recycler() {
		for (auto &slot : pointers) {
			T* ptr = slot.load(std::memory_order_acquire);
			if (ptr)
				delete ptr;
		}
	}

	void __fastcall release(T *ptr) {
		T* null = nullptr;
		for (auto &slot : pointers)
			if (slot.compare_exchange_strong(null, ptr, std::memory_order_relaxed)) {
				return;
			}
		delete ptr;
	}

	template <typename ... Ts>
	T* __fastcall claim(Ts&&... args) {
		T* ptr;
		for (auto &slot : pointers) {
			ptr = slot.load(std::memory_order_relaxed);
			if (ptr && slot.compare_exchange_strong(ptr, nullptr, std::memory_order_relaxed)) {
				*ptr = T(std::forward<Ts>(args)...);
				return ptr;
			}
		}

		return new T(std::forward<Ts>(args)...);
	}
};

}
