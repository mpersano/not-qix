#pragma once

#include <vector>
#include <memory>
#include <algorithm>
#include <functional>

// event system more or less stolen from ChilliSource:
//
// https://github.com/ChilliWorks/ChilliSource
//
// in fact, i probably should be using ChilliSource instead.

namespace ggl {

class event_connection;
using event_connection_ptr = std::unique_ptr<event_connection>;

template <typename HandlerType>
class connectable_event
{
public:
	virtual ~connectable_event() = default;
	virtual event_connection_ptr connect(const HandlerType& handler) = 0;
};

class disconnectable_event
{
public:
	virtual ~disconnectable_event() = default;
	virtual void disconnect(event_connection *conn) = 0;
};

class event_connection
{
public:
	event_connection(disconnectable_event& owner);
	~event_connection();

private:
	disconnectable_event& owner_;
};

template <typename HandlerType>
class event : public connectable_event<HandlerType>, public disconnectable_event
{
public:
	event_connection_ptr connect(const HandlerType& handler) override
	{
		event_connection_ptr conn(new event_connection { *this });
		connections_.push_back({ handler, conn.get() });
		return conn;
	}

	void disconnect(event_connection *conn)
	{
		connections_.erase(
			std::remove_if(
				std::begin(connections_),
				std::end(connections_),
				[=](connection_desc& c) { return c.conn == conn; }),
			std::end(connections_));
	}

	template <typename... Args>
	void notify(Args&&... args)
	{
		for (auto& c : connections_)
			c.handler(std::forward<Args>(args)...);
	}

private:
	struct connection_desc
	{
		HandlerType handler;
		event_connection *conn;
	};

	std::vector<connection_desc> connections_;
};

}
