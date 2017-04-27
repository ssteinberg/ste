// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <memory>
#include <unordered_map>

#include <connection.hpp>

namespace ste {

template <typename ... Ts>
class connection;

template <typename ... Ts>
class signal {
private:
	friend class connection<Ts...>;

public:
	using connection_type = connection<Ts...>;

private:
	using connection_map_type = std::unordered_map<int, std::weak_ptr<connection_type>>;

	static int pool;
	mutable connection_map_type connections;

	void disconnect(int id) const {
		connections.erase(id);
	}

public:
	signal() {}
	~signal() {
		for (auto it = connections.begin(); it != connections.end(); ++it)
			if (auto sptr = it->second.lock())
				sptr->break_connection();
	}

	signal(const signal &s) = delete;
	signal& operator=(const signal &s) = delete;
	signal(signal &&s) = default;
	signal& operator=(signal &&s) = default;

	void emit(const Ts&...args) {
		for (auto it = connections.begin(); it != connections.end(); ++it)
			if (auto sptr = it->second.lock()) 
				(*sptr)(args...);
	}

	void connect(std::shared_ptr<connection_type> &con) const {
		con->id = pool++;
		con->sig = this;
		connections.emplace(std::make_pair(con->id, con));
	}
	
	void disconnect(std::shared_ptr<connection_type> &con) const {
		disconnect(con->id);
		con->sig = nullptr;
	}
};

template <typename ... Ts>
int signal<Ts...>::pool;

}
