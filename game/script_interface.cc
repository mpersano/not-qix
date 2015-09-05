#include <memory>
#include <cassert>

#include "phys_foe.h"
#include "boss.h"
#include "script_interface.h"

//
// idea:
//   * each script has on its own lua thread
//   * each script exposes a common interface, the functions listed under interface_functions[]
//   * when each script is loaded, interface functions are removed from the global namespace and
//     stored on a table in _scriptfuncs
//   * each thread has its own instance of the table "v" that it can use to store thread-local
//     variables
//
// idea pretty much ripped off from the script system in Aquaria, including variable names _scriptfuncs,
// _scriptvars, _threadvars and _threadtable.
//

namespace {

void
dump_lua_stack(lua_State *l)
{
	if (lua_gettop(l) == 0) {
		printf("(empty)\n");
	} else {
		for (int i = 1; i <= lua_gettop(l); i++) {
			fprintf(stderr, "%d: %s", i, lua_typename(l, lua_type(l, i)));

			switch (lua_type(l, i)) {
				case LUA_TNUMBER:
					fprintf(stderr, " (%f)", lua_tonumber(l, i));
					break;

				case LUA_TSTRING:
					fprintf(stderr, " (%s)", lua_tostring(l, i));
					break;
			}

			putchar('\n');
		}
	}

	putchar('\n');
}

void
dup_table(lua_State *l)
{
	assert(lua_gettop(l) > 0 && lua_istable(l, -1));

	lua_newtable(l);

	lua_pushnil(l);

	while (lua_next(l, -3)) {
			// orig_table | new_table | key | value
		lua_pushvalue(l, -2);
			// orig_table | new_table | key | value | key
		lua_insert(l, -2);
			// orig_table | new_table | key | key | value
		lua_settable(l, -4);
			// orig_table | new_table | key
	}
}

//
// exported functions
//

// foe

int
foe_update_position(lua_State *state)
{
	reinterpret_cast<phys_foe *>(lua_touserdata(state, -1))->update_position();
	return 0;
}

int
foe_rotate_to_player(lua_State *state)
{
	reinterpret_cast<phys_foe *>(lua_touserdata(state, -1))->rotate_to_player();
	return 0;
}

int
foe_set_speed(lua_State *state)
{
	reinterpret_cast<phys_foe *>(lua_touserdata(state, -2))->set_speed(lua_tonumber(state, -1));
	return 0;
}

// boss

int
boss_rotate_spike_to_player(lua_State *state)
{
	reinterpret_cast<boss *>(lua_touserdata(state, -1))->rotate_spike_to_player();
	return 0;
}

int
boss_rotate_spike(lua_State *state)
{
	reinterpret_cast<boss *>(lua_touserdata(state, -2))->rotate_spike(lua_tonumber(state, -1));
	return 0;
}

int
boss_set_spike_dispersion(lua_State *state)
{
	reinterpret_cast<boss *>(lua_touserdata(state, -2))->set_spike_dispersion(lua_tonumber(state, -1));
	return 0;
}

int
boss_fire_bullet(lua_State *state)
{
	reinterpret_cast<boss *>(lua_touserdata(state, -1))->fire_bullet();
	return 0;
}

const std::pair<const char *, lua_CFunction> exported_functions[] {
	{ "foe_update_position", foe_update_position },
	{ "foe_rotate_to_player", foe_rotate_to_player },
	{ "foe_set_speed", foe_set_speed },
	{ "boss_rotate_spike_to_player", boss_rotate_spike_to_player },
	{ "boss_rotate_spike", boss_rotate_spike },
	{ "boss_set_spike_dispersion", boss_set_spike_dispersion },
	{ "boss_fire_bullet", boss_fire_bullet },
};

const char *const script_interface_functions[] {
	"init",
	"update"
};

class script_interface
{
public:
	script_interface();
	~script_interface();

	std::unique_ptr<script_thread> create_script_thread(const std::string& path);

private:
	lua_State *lua_state_;
} *g_script_interface;

script_interface::script_interface()
: lua_state_ { luaL_newstate() }
{
	luaL_openlibs(lua_state_);

	// interface function tables for each script
	lua_newtable(lua_state_);
	lua_setglobal(lua_state_, "_scriptfuncs");

	// initial instance-local variable tables for each script
	lua_newtable(lua_state_);
	lua_setglobal(lua_state_, "_scriptvars");

	// instance-local variable tables for each thread
	lua_newtable(lua_state_);
	lua_setglobal(lua_state_, "_threadvars");

	// active threads (to avoid gc)
	lua_newtable(lua_state_);
	lua_setglobal(lua_state_, "_threadtable");

	// register functions
	for (auto& func : exported_functions)
		lua_register(lua_state_, func.first, func.second);
}

script_interface::~script_interface()
{
	lua_close(lua_state_);
}

std::unique_ptr<script_thread>
script_interface::create_script_thread(const std::string& path)
{
	// already loaded?

	lua_getglobal(lua_state_, "_scriptvars");
	lua_getfield(lua_state_, -1, path.c_str());
	bool loaded = lua_istable(lua_state_, -1);
	lua_pop(lua_state_, 2);

	if (!loaded) {
		// save v
		lua_getglobal(lua_state_, "v");

		// create new table for thread-local vars
		lua_newtable(lua_state_);
		lua_setglobal(lua_state_, "v");

		// load it

		if (luaL_loadfile(lua_state_, path.c_str())) {
			fprintf(stderr, "failed to parse %s: %s\n", path.c_str(), lua_tostring(lua_state_, -1));
			lua_pop(lua_state_, 1);
			return nullptr;
		}

		// first run

		lua_pcall(lua_state_, 0, 0, 0);

		// store initial thread-local vars on _scriptvars
		lua_getglobal(lua_state_, "_scriptvars");
		lua_getglobal(lua_state_, "v");
		lua_setfield(lua_state_, -2, path.c_str());
		lua_pop(lua_state_, 1);

		// restore v
		lua_setglobal(lua_state_, "v");

		// add interface functions to table, remove from global environment

		lua_getglobal(lua_state_, "_scriptfuncs");
		lua_newtable(lua_state_);

		for (auto func : script_interface_functions) {
			lua_getglobal(lua_state_, func);

			if (!lua_isnil(lua_state_, -1)) {
				lua_setfield(lua_state_, -2, func);

				lua_pushnil(lua_state_);
				lua_setglobal(lua_state_, func);
			} else {
				lua_pop(lua_state_, 1);
			}
		}

		// add function table to _scriptfuncs

		lua_setfield(lua_state_, -2, path.c_str());
		lua_pop(lua_state_, 1);
	}

	lua_State *thread = lua_newthread(lua_state_);

	// add thread to _threadtable to avoid gc

	lua_getglobal(lua_state_, "_threadtable");
	lua_pushlightuserdata(lua_state_, thread);
	lua_pushvalue(lua_state_, -3); // thread
	lua_rawset(lua_state_, -3);
	lua_pop(lua_state_, 2);

	// copy table from _scriptvars to new table, add it to _threadvars

	lua_getglobal(lua_state_, "_scriptvars");
	lua_getfield(lua_state_, -1, path.c_str());
	dup_table(lua_state_);

	lua_getglobal(lua_state_, "_threadvars");
	lua_pushlightuserdata(lua_state_, thread);
	lua_pushvalue(lua_state_, -3);
	lua_rawset(lua_state_, -3);

	lua_pop(lua_state_, 4);

	return std::unique_ptr<script_thread> { new script_thread { thread, path } };
}

} // (anonymous namespace)

script_thread::script_thread(lua_State *thread, const std::string& name)
: thread_ { thread }
, name_ { name }
{ }

script_thread::~script_thread()
{
	// XXX: probably should do something here
}

void
script_thread::setup_call(const std::string& func)
{
	// set global variable "v" to thread-local var table

	lua_getglobal(thread_, "_threadvars");
	lua_pushlightuserdata(thread_, thread_);
	lua_gettable(thread_, -2);
	lua_remove(thread_, -2);
	lua_setglobal(thread_, "v");

	// lookup function on this script's function table

	lua_getglobal(thread_, "_scriptfuncs");
	lua_getfield(thread_, -1, name_.c_str());
	lua_getfield(thread_, -1, func.c_str());
	lua_remove(thread_, -2);
	lua_remove(thread_, -2);
}

void
script_thread::cleanup_call()
{
	// XXX: stuff here
}

void
script_thread::push(float value)
{
	lua_pushnumber(thread_, value);
}

void
script_thread::push(const std::string& value)
{
	lua_pushstring(thread_, value.c_str());
}

void
script_thread::push(void *value)
{
	lua_pushlightuserdata(thread_, value);
}

void
init_script_interface()
{
	g_script_interface = new script_interface;
}

std::unique_ptr<script_thread>
create_script_thread(const std::string& script_path)
{
	return g_script_interface->create_script_thread(script_path);
}
