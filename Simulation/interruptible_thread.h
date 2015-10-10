// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <memory>
#include <thread>
#include <future>

namespace StE {

class interruptible_thread {
private:
	class interruptible_thread_flag {
	private:
		std::atomic<bool> flag{ false };

	public:
		void set() { flag = true; }
		bool is_set() const { return flag; }
	};

public:
	static thread_local interruptible_thread_flag *interruption_flag;

private:
	std::unique_ptr<interruptible_thread_flag> flag;
	std::unique_ptr<std::promise<bool>> promise;
	std::thread t;

public:
	template <typename Functor>
	interruptible_thread(Functor&& func) : flag(new interruptible_thread_flag), promise(new std::promise<bool>), t([f = std::move(func), ptr_flag = flag.get(), ptr_promise = promise.get()]() {
		interruption_flag = ptr_flag;
		f();
		ptr_promise->set_value(true);
	}) {}
	void interrupt() { flag->set(); }
	void join() { t.join(); }
	bool joinable() const { return t.joinable(); }
	void detach() { t.detach(); }
	std::future<bool> get_future() { return promise->get_future(); }

	std::thread &get_thread() { return t; }
};

}
