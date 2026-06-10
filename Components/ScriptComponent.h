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

    // Each script gets its own isolated environment so scripts cannot
    // accidentally read/write each other's globals.
    sol::environment          env;
    sol::protected_function   updateFunc;
    sol::protected_function   initFunc;
    bool initialized = false;

    // ---------------------------------------------------------------
    // Core load + execute logic.  Called on first Initialize() and on
    // every hot-reload triggered by the FileWatcher.
    // ---------------------------------------------------------------
    void LoadAndExecute() {
        sol::state& lua = Game::scriptManager->GetLuaState();

        // ----- Preserve state before destroying the old environment -----

        // Strategy 2: grab the 'state' table so the script can pick it
        // up automatically on reload via  state = state or {}
        sol::table persistentState;
        if (initialized && env.valid()) {
            persistentState = env["state"];
        }

        // Strategy 3: explicit on_save callback for fine-grained control
        sol::table savedState;
        if (initialized && env.valid()) {
            sol::protected_function saveFunc = env["on_save"];
            if (saveFunc.valid()) {
                auto r = saveFunc();
                if (r.valid()) savedState = r;
            }
        }

        // ----- Create a fresh isolated environment -----
        env = sol::environment(lua, sol::create, lua.globals());
        env["self"]      = owner;
        env["is_reload"] = initialized;   // false on first load, true on reload

        // Inject preserved 'state' table before the script executes so
        //   state = state or {}
        // picks up the live values instead of re-initializing.
        if (persistentState.valid()) {
            env["state"] = persistentState;
        }

        // ----- Execute the script file -----
        auto result = lua.safe_script_file(scriptPath, env, sol::script_pass_on_error);
        if (!result.valid()) {
            sol::error err = result;
            std::cerr << "[ScriptComponent] Load error in " << scriptPath
                      << ": " << err.what() << std::endl;
            // Clear updateFunc so the entity safely stops updating.
            // The developer can fix the error; the next file-save triggers
            // another reload and the entity resumes automatically.
            updateFunc = sol::protected_function{};
            return;
        }

        // ----- Cache function references -----
        updateFunc = env["on_update"];
        initFunc   = env["on_init"];

        // ----- on_init -----
        if (initFunc.valid()) {
            auto r = initFunc();
            if (!r.valid()) {
                sol::error err = r;
                std::cerr << "[ScriptComponent] on_init error in " << scriptPath
                          << ": " << err.what() << std::endl;
            }
        }

        // ----- on_restore (Strategy 3) -----
        if (savedState.valid()) {
            sol::protected_function restoreFunc = env["on_restore"];
            if (restoreFunc.valid()) restoreFunc(savedState);
        }

        std::cout << "[ScriptComponent] " << (initialized ? "Reloaded" : "Loaded")
                  << ": " << scriptPath << std::endl;
    }

    // Register this script path with the global FileWatcher so edits
    // automatically trigger Reload().
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
            // Intentionally NOT disabling: a follow-up hot-reload fixes it
            // without restarting the game.
        }
    }

    void Render() override {}
};

#endif
