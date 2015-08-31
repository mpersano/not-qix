#pragma once

#include <string>
#include <lua5.2/lua.hpp>

#include <ggl/noncopyable.h>

class script_thread : private ggl::noncopyable
{
public:
	script_thread(lua_State *thread, const std::string& name);
	~script_thread();

	template <typename... Args>
	void call(const std::string& func, Args... args)
	{
		setup_call(func);

		push_args(args...);
		lua_pcall(thread_, sizeof...(args), 0, 0);

		cleanup_call();
	}

private:
	void setup_call(const std::string& func);
	void cleanup_call();

	template <typename T, typename... Args>
	void push_args(T first, Args... args)
	{
		push(first);
		push_args(args...);
	}

	void
	push_args()
	{ }

	void push(float value);
	void push(const std::string& value);
	void push(void *value);

	lua_State *thread_;
	std::string name_;
};

std::unique_ptr<script_thread>
create_script_thread(const std::string& script_path);

void
init_script_interface();
