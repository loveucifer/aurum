#ifndef SCRIPTCOMPONENT_H
#define SCRIPTCOMPONENT_H

#include "../src/Component.h"
#include "../src/Game.h"
#include "../lib/lua/sol/sol.hpp"
#include <string>
#include <iostream>

class ScriptComponent : public Component {
private:
    std::string scriptPath;


    sol::environment          env;
    sol::protected_function   updateFunc;
    sol::protected_function   initFunc;
    bool initialized = false;

    // ---------------------------------------------------------------
    // Core load + execute logic.
    // ---------------------------------------------------------------
    void LoadAndExecute() {
        sol::state& lua = Game::scriptManager->GetLuaState();


        sol::table persistentState;
        if (initialized && env.valid()) {
            persistentState = env["state"];
        }


        sol::table savedState;
        if (initialized && env.valid()) {
            sol::protected_function saveFunc = env["on_save"];
            if (saveFunc.valid()) {
                auto r = saveFunc();
                if (r.valid()) savedState = r;
            }
        }


        env = sol::environment(lua, sol::create, lua.globals());
        env["self"]      = owner;
        env["is_reload"] = initialized;   // false on first load, true on reload


        if (persistentState.valid()) {
            env["state"] = persistentState;
        }

        // ----- Execute the script file -----
        auto result = lua.safe_script_file(scriptPath, env, sol::script_pass_on_error);
        if (!result.valid()) {
            sol::error err = result;
            std::cerr << "[ScriptComponent] Load error in " << scriptPath
                      << ": " << err.what() << std::endl;


            updateFunc = sol::protected_function{};
            return;
        }


        updateFunc = env["on_update"];
        initFunc   = env["on_init"];


        if (initFunc.valid()) {
            auto r = initFunc();
            if (!r.valid()) {
                sol::error err = r;
                std::cerr << "[ScriptComponent] on_init error in " << scriptPath
                          << ": " << err.what() << std::endl;
            }
        }


        if (savedState.valid()) {
            sol::protected_function restoreFunc = env["on_restore"];
            if (restoreFunc.valid()) restoreFunc(savedState);
        }

        std::cout << "[ScriptComponent] " << (initialized ? "Reloaded" : "Loaded")
                  << ": " << scriptPath << std::endl;
    }


    void RegisterWithFileWatcher() {
        Game::fileWatcher->Watch(scriptPath, [this](const std::string&) {
            this->Reload();
        });
    }

public:
    explicit ScriptComponent(const std::string& path) : scriptPath(path) {}

    const std::string& GetScriptPath() const { return scriptPath; }

    void Initialize() override {
        LoadAndExecute();
        initialized = true;
        RegisterWithFileWatcher();
    }

    // Called by the FileWatcher callback whenever the .lua file is saved.
    void Reload() {
        Uint32 start = SDL_GetTicks();
        LoadAndExecute();
        Uint32 elapsed = SDL_GetTicks() - start;
        std::cout << "[Hot Reload] " << scriptPath
                  << " reloaded in " << elapsed << " ms" << std::endl;
    }

    void Update(float deltaTime) override {
        if (!updateFunc.valid()) return;

        auto result = updateFunc(deltaTime);
        if (!result.valid()) {
            sol::error err = result;
            std::cerr << "[ScriptComponent] on_update error in " << scriptPath
                      << ": " << err.what() << std::endl;
            // Intentionally NOT disabling: a follow-up hot-reload probably fixes it
            // without restarting the game.
        }
    }

    void Render() override {}
};

#endif
