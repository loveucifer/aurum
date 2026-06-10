#ifndef SCRIPTMANAGER_H
#define SCRIPTMANAGER_H

#include <string>
#include <iostream>
#include <stdio.h>

#include "../lib/lua/sol/sol.hpp"

class Entity;
class EntityManager;

class ScriptManager {
private:
    sol::state lua;

public:
    ScriptManager() {
        lua.open_libraries(
            sol::lib::base,
            sol::lib::math,
            sol::lib::string,
            sol::lib::table,
            sol::lib::io,       // remove this for sandboxing (see below)
            sol::lib::os        // remove this for sandboxing (see below)
        );
    }

    sol::state& GetLuaState() { return lua; }


    bool LoadScript(const std::string& filePath) {
        try {
            sol::load_result result = lua.load_file(filePath);
            if (!result.valid()) {
                sol::error err = result;
                std::cerr << "[Lua Load Error] " << filePath << ": " << err.what() << std::endl;
                return false;
            }
            sol::protected_function script = result;
            sol::protected_function_result execResult = script();
            if (!execResult.valid()) {
                sol::error err = execResult;
                std::cerr << "[Lua Runtime Error] " << filePath << ": " << err.what() << std::endl;
                return false;
            }
            return true;
        } catch (const sol::error& e) {
            std::cerr << "[Lua Exception] " << filePath << ": " << e.what() << std::endl;
            return false;
        }
    }


    bool ExecuteString(const std::string& code) {
        try {
            sol::protected_function_result result = lua.safe_script(code, sol::script_pass_on_error);
            if (!result.valid()) {
                sol::error err = result;
                std::cerr << "[Lua Error] " << err.what() << std::endl;
                return false;
            }
            return true;
        } catch (const sol::error& e) {
            std::cerr << "[Lua Exception] " << e.what() << std::endl;
            return false;
        }
    }
};


#endif
