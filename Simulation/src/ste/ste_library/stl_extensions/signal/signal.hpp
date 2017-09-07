//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <connection.hpp>

#include <boost/container/flat_set.hpp>
#include <mutex>

namespace ste {

/**
 *	@brief	Implements a simple, thread-safe, signal-slots idiom. 
 *			Signals are connected to connections (also called subscribers), which are called when signals are emmited.
 *			Connections should be made via 'make_connection()' and the lifetime of the connected connections are tracked automatically.
 */
template <typename ... Ts>
class signal {
private:
	friend connection<Ts...>;

public:
	using connection_type = connection<Ts...>;

private:
	using connection_map_type = boost::container::flat_set<connection_type*>;

	connection_map_type connections;

	mutable std::mutex m;

private:
	void disconnect_all() {
		for (auto it = connections.begin(); it != connections.end(); ++it)
			(*it)->sig = nullptr;
	}

private:
	signal(std::unique_lock<std::mutex> &&lock,
		   signal &&s) noexcept : connections(std::move(s.connections)) {
		for (auto &c : connections)
			c->sig = this;
	}

public:
	signal() = default;
	~signal() noexcept {
		std::unique_lock<std::mutex> l(m);
		disconnect_all();
	}

	signal(const signal &s) = delete;
	signal& operator=(const signal &s) = delete;

	signal(signal &&s) noexcept : signal(std::unique_lock<std::mutex>(s.m), std::move(s)) {}
	signal& operator=(signal &&s) noexcept {
		std::unique_lock<std::mutex> lock1(m, std::defer_lock);
		std::unique_lock<std::mutex> lock2(s.m, std::defer_lock);
		std::lock(lock1, lock2);

		disconnect_all();

		connections = std::move(s.connections);
		for (auto &c : connections)
			c->sig = this;

		return *this;
	}

	/**
	 *	@brief	Emits a signal.
	 */
	void emit(const Ts&...args) const {
		std::unique_lock<std::mutex> l(m);
		for (auto &c : connections) {
			// Emit
			(*c)(args...);
		}
	}

	void connect(connection_type *con) {
		con->sig = this;

		std::unique_lock<std::mutex> l(m);
		connections.insert(con);
	}

	void disconnect(connection_type *con) {
		con->sig = nullptr;

		std::unique_lock<std::mutex> l(m);
		connections.erase(con);
	}
};

/**
*	@brief	Creates a new connection object and connects it to the provided signal object.
*	
*	@param	sig		Signal to connect to
*	@param	lambda	Expression to call on emitted signals
*/
template <typename L, typename ... Ts>
typename signal<Ts...>::connection_type make_connection(signal<Ts...> &sig, L&& lambda) {
	auto con = signal<Ts...>::connection_type(std::forward<L>(lambda));
	sig.connect(&con);

	return con;
}

}
