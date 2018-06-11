#pragma once

extern "C" {
#include "lua\lua.h"
#include "lua\lauxlib.h"
#include "lua\lualib.h"
}

#pragma comment(lib, "lua53.lib")

class ScriptEngine {
private:
	lua_State * L;
public:
	ScriptEngine(const char* filePath, int id);
	~ScriptEngine();

	lua_State* getInstance() { return L; }

	void display_error(int err);
	
	void eventPlayerMove(int player);
};