// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <connection.hpp>

#include <boost/container/flat_set.hpp>

namespace ste {

/**
 *	@brief	Implements a simple, non thread-safe, signal-slots idiom. 
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

private:
	void disconnect_all() {
		for (auto it = connections.begin(); it != connections.end(); ++it)
			(*it)->sig = nullptr;
	}

public:
	signal() = default;
	~signal() noexcept {
		disconnect_all();
	}

	signal(const signal &s) = delete;
	signal& operator=(const signal &s) = delete;

	signal(signal &&s) noexcept : connections(std::move(s.connections)) {
		for (auto &c : connections)
			c->sig = this;
	}
	signal& operator=(signal &&s) noexcept {
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
		for (auto &c : connections) {
			// Emit
			(*c)(args...);
		}
	}

	void connect(connection_type *con) {
		con->sig = this;
		connections.insert(con);
	}

	void disconnect(connection_type *con) {
		con->sig = nullptr;
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
