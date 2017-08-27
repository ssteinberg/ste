// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <atomic>
#include <type_traits>

namespace ste {

template <typename T>
class atomic_pod {
	static_assert(std::is_pod_v<T>, "T must be a POD");
	static_assert(sizeof std::atomic<T> == sizeof T, "std::atomic<T> differs in size from T. Possibly not lock-free implementation of T.");

	using atomic_t = std::atomic<T>;

private:
	T data;

public:
	atomic_pod() = default;
	atomic_pod(const T &t)
		: data(t)
	{}

	auto& get() { return *reinterpret_cast<atomic_t*>(&data); }
	auto& get() const { return *reinterpret_cast<const atomic_t*>(&data); }
};

}
