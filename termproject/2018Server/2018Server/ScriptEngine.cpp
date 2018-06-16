#include "ScriptEngine.h"

ScriptEngine::ScriptEngine(const char * filePath, int id)
{
	L = luaL_newstate();
	luaL_openlibs(L);
	int error = luaL_loadfile(L, filePath);
	if (error > 0)
		display_error(error);
	error = lua_pcall(L, 0, 0, 0);
	if (error > 0)
		display_error(error);

	lua_getglobal(L, "set_myid");
	lua_pushnumber(L, id);
	error = lua_pcall(L, 1, 0, 0);
	if (error > 0)
		display_error(error);
}

ScriptEngine::~ScriptEngine()
{
	delete L;
}

void ScriptEngine::display_error(int err)
{
	printf("error : %s\n", lua_tostring(L, -1));
	lua_pop(L, 1);
}

void ScriptEngine::eventPlayerMove(int player)
{
	if (L == nullptr)
		return;
	lua_getglobal(L, "event_player_move");
	lua_pushnumber(L, player);
	int error = lua_pcall(L, 1, 1, 0);
	if (error > 0)
		display_error(error);
	lua_pop(L, 1);
}
