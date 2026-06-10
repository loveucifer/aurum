#include <cstddef>
#include <iostream>
#include <string>
#include "./Constants.h"
#include "./Game.h"
#include "Entity.h"
#include "EntityManager.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "AssetManager.h"
#include "SDL2/SDL_events.h"
#include "../Components/KeyboardControlComponent.h"
#include "Map.h"
#include "../Components/ColliderComponent.h"
#include "../Components/LabelComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/ScriptComponent.h"
#include "ScriptManager.h"
#include "LuaBindings.h"
#include "../lib/lua/sol/sol.hpp"
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "EditorStyle.h"
#include "EditorPanels.h"


EntityManager manager;
SDL_Renderer* Game::renderer;
AssetManager* Game::assetManager = new AssetManager(&manager);
ScriptManager* Game::scriptManager = new ScriptManager();
SDL_Event Game::event;
SDL_Rect Game::camera = {0,0,WINDOW_WIDTH, WINDOW_HEIGHT};
FileWatcher* Game::fileWatcher = new FileWatcher();
Map* map;
Entity* player = nullptr;
Game* gameInstance = nullptr;


Game::Game(){
    this -> isRunning = false;
    gameInstance = this;
}



Game::~Game(){

}




bool Game::IsRunning() const {
    return this->isRunning;
}


void Game::Initialize(int width , int height ){

    if ( SDL_Init(SDL_INIT_EVERYTHING) != 0 ) {
        std::cerr << "Error Initializing SDL" << std::endl;
        return;
    }

    if (TTF_Init() != 0 ) {

        std::cerr << "Error initializing SDL_ttf" << std::endl;
        return;

    }

    window = SDL_CreateWindow(
        "Aurum",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);

    if (!window){
        std::cerr << "Error creating SDL window" << std::endl;
        return;
    }


    // Try creating hardware accelerated renderer with target texture support first
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    if (!renderer) {
        std::cerr << "Accelerated target texture renderer creation failed: " << SDL_GetError() << ". Retrying default..." << std::endl;
        renderer = SDL_CreateRenderer(window, -1, 0);
    }
    if (!renderer) {
        std::cerr << "Error creating SDL renderer: " << SDL_GetError() << std::endl;
        return;
    }

    // ImGui init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;  // enable docking

    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    // Apply retro style
    ApplyRetroStyle();

    // Load classic Windows Arial font
    ImFontConfig fontConfig;
    fontConfig.OversampleH = 1;
    fontConfig.OversampleV = 1;
    fontConfig.PixelSnapH = true;
    ImFont* retroFont = io.Fonts->AddFontFromFileTTF("assets/fonts/arial.ttf", 13.0f, &fontConfig);
    if (retroFont) {
        io.FontDefault = retroFont;
    } else {
        io.Fonts->AddFontDefault();
    }

    // Create viewport texture
    viewportTexture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        WINDOW_WIDTH, WINDOW_HEIGHT
    );
    if (!viewportTexture) {
        std::cerr << "Warning: Failed to create offscreen viewport texture target: " << SDL_GetError() << std::endl;
    }

    LoadLevel(0);


    isRunning = true;
    return;

}

void Game::LoadLevel(int levelNumber) {

    std::string scriptPath = "assets/scripts/level" + std::to_string(levelNumber) + ".lua";


    LuaBindings::RegisterAll(scriptManager->GetLuaState(), manager);


    if (!scriptManager->LoadScript(scriptPath)) {
        std::cerr << "[Game] Failed to load level script: " << scriptPath << ". Aborting." << std::endl;
        isRunning = false;
        return;
    }

    sol::state& lua = scriptManager->GetLuaState();


    sol::table level = lua["Level1"];
    if (!level.valid()) {
        std::cerr << "[Game] 'Level1' table not found after loading " << scriptPath << std::endl;
        isRunning = false;
        return;
    }
    std::cout << "[Game] Level1 table loaded successfully from " << scriptPath << std::endl;

    // -------------------------------------------------------
    // Load assets defined in Level1.assets
    // -------------------------------------------------------
    assetManager->ClearData();
    sol::table assetsTable = level["assets"];
    if (!assetsTable.valid()) {
        std::cerr << "[Game] Warning: Level1.assets table missing" << std::endl;
    } else {
        assetsTable.for_each([&](sol::object /*key*/, sol::object val) {
            if (val.get_type() != sol::type::table) return;
            sol::table asset  = val.as<sol::table>();
            std::string type  = asset.get_or<std::string>("type",  "");
            std::string id    = asset.get_or<std::string>("id",    "");
            std::string file  = asset.get_or<std::string>("file",  "");
            if (id.empty() || file.empty()) return;
            if (type == "texture") {
                assetManager->AddTexture(id, file.c_str());
                std::cout << "[Lua] texture loaded: " << id << std::endl;
            } else if (type == "font") {
                int fontSize = asset.get_or("fontSize", 14);
                assetManager->AddFont(id, file.c_str(), fontSize);
                std::cout << "[Lua] font loaded: " << id << std::endl;
            }
        });
    }

    // -------------------------------------------------------
    // Configure and load the map from Level1.map
    // -------------------------------------------------------
    sol::table mapConfig = level["map"];
    if (!mapConfig.valid()) {
        std::cerr << "[Game] Warning: Level1.map table missing" << std::endl;
    } else {
        if (map) {
            delete map;
            map = nullptr;
        }
        std::string texId   = mapConfig.get_or<std::string>("textureAssetId", "");
        std::string mapFile = mapConfig.get_or<std::string>("file",           "");
        int scale    = mapConfig.get_or("scale",    1);
        int tileSize = mapConfig.get_or("tileSize", 32);
        int sizeX    = mapConfig.get_or("mapSizeX", 25);
        int sizeY    = mapConfig.get_or("mapSizeY", 20);
        std::cout << "[Game] Loading map: " << mapFile
                  << " texture=" << texId
                  << " scale=" << scale
                  << " tileSize=" << tileSize << std::endl;
        map = new Map(texId, scale, tileSize);
        map->LoadMap(mapFile, sizeX, sizeY);
    }

    // -------------------------------------------------------
    // Create entities dynamically from Level1.entities
    // -------------------------------------------------------
    manager.ClearData();
    sol::table entitiesTable = level["entities"];
    if (!entitiesTable.valid()) {
        std::cerr << "[Game] Warning: Level1.entities table missing" << std::endl;
    } else {
        entitiesTable.for_each([&](sol::object key, sol::object val) {
            if (val.get_type() != sol::type::table) return;
            sol::table ent = val.as<sol::table>();
            
            std::string name = ent.get_or<std::string>("name", "unnamed");
            int layerInt = ent.get_or("layer", 0);
            LayerType layer = static_cast<LayerType>(layerInt);
            
            Entity& entity = manager.AddEntity(name, layer);
            
            sol::optional<sol::table> componentsOpt = ent["components"];
            if (componentsOpt && componentsOpt->valid()) {
                sol::table components = *componentsOpt;

                // 1. TransformComponent
                sol::optional<sol::table> transformOpt = components["transform"];
                if (transformOpt && transformOpt->valid()) {
                    sol::table transform = *transformOpt;
                    sol::optional<sol::table> posTable = transform["position"];
                    sol::optional<sol::table> velTable = transform["velocity"];
                    int px = (posTable && posTable->valid()) ? posTable->get_or("x", 0) : 0;
                    int py = (posTable && posTable->valid()) ? posTable->get_or("y", 0) : 0;
                    int vx = (velTable && velTable->valid()) ? velTable->get_or("x", 0) : 0;
                    int vy = (velTable && velTable->valid()) ? velTable->get_or("y", 0) : 0;
                    int w = transform.get_or("width", 32);
                    int h = transform.get_or("height", 32);
                    int s = transform.get_or("scale", 1);
                    entity.AddComponent<TransformComponent>(px, py, vx, vy, w, h, s);
                }
                
                // 2. SpriteComponent
                sol::optional<sol::table> spriteOpt = components["sprite"];
                if (spriteOpt && spriteOpt->valid()) {
                    sol::table sprite = *spriteOpt;
                    std::string texAssetId = sprite.get_or<std::string>("textureAssetId", "");
                    bool animated = sprite.get_or("animated", false);
                    if (animated) {
                        int frameCount = sprite.get_or("frameCount", 1);
                        int speed = sprite.get_or("animationSpeed", 100);
                        bool hasDirections = sprite.get_or("hasDirections", false);
                        bool fixed = sprite.get_or("fixed", false);
                        entity.AddComponent<SpriteComponent>(texAssetId, frameCount, speed, hasDirections, fixed);
                    } else {
                        entity.AddComponent<SpriteComponent>(texAssetId);
                    }
                }
                
                // 3. ColliderComponent
                sol::optional<sol::table> colliderOpt = components["collider"];
                if (colliderOpt && colliderOpt->valid()) {
                    sol::table collider = *colliderOpt;
                    std::string tag = collider.get_or<std::string>("tag", "UNKNOWN");
                    int cx = 0, cy = 0, cw = 32, ch = 32;
                    if (entity.HasComponent<TransformComponent>()) {
                        auto* trans = entity.GetComponent<TransformComponent>();
                        cx = static_cast<int>(trans->position.x);
                        cy = static_cast<int>(trans->position.y);
                        cw = trans->width * trans->scale;
                        ch = trans->height * trans->scale;
                    }
                    entity.AddComponent<ColliderComponent>(tag, cx, cy, cw, ch);
                }
                
                // 4. KeyboardControlComponent
                sol::optional<sol::table> inputOpt = components["input"];
                if (inputOpt && inputOpt->valid()) {
                    sol::table input = *inputOpt;
                    sol::optional<sol::table> kbOpt = input["keyboard"];
                    if (kbOpt && kbOpt->valid()) {
                        sol::table kb = *kbOpt;
                        std::string up = kb.get_or<std::string>("up", "w");
                        std::string right = kb.get_or<std::string>("right", "d");
                        std::string down = kb.get_or<std::string>("down", "s");
                        std::string left = kb.get_or<std::string>("left", "a");
                        std::string shoot = kb.get_or<std::string>("shoot", "space");
                        entity.AddComponent<KeyboardControlComponent>(up, right, down, left, shoot);
                    }
                }
                
                // 5. ProjectileEmitterComponent (spawns separate projectile entity)
                sol::optional<sol::table> projEmitOpt = components["projectileEmitter"];
                if (projEmitOpt && projEmitOpt->valid()) {
                    sol::table projEmit = *projEmitOpt;
                    int speed = projEmit.get_or("speed", 100);
                    int angle = projEmit.get_or("angle", 0);
                    int range = projEmit.get_or("range", 200);
                    bool shouldLoop = projEmit.get_or("shouldLoop", true);
                    std::string projTexId = projEmit.get_or<std::string>("textureAssetId", "projectile-texture");
                    int pw = projEmit.get_or("width", 4);
                    int ph = projEmit.get_or("height", 4);
                    
                    int startX = 0;
                    int startY = 0;
                    if (entity.HasComponent<TransformComponent>()) {
                        auto* trans = entity.GetComponent<TransformComponent>();
                        startX = static_cast<int>(trans->position.x) + (trans->width * trans->scale) / 2 - pw / 2;
                        startY = static_cast<int>(trans->position.y) + (trans->height * trans->scale) / 2 - ph / 2;
                    }
                    
                    std::string projName = name + "_projectile";
                    Entity& projectile = manager.AddEntity(projName, PROJECTILE_LAYER);
                    projectile.AddComponent<TransformComponent>(startX, startY, 0, 0, pw, ph, 1);
                    projectile.AddComponent<SpriteComponent>(projTexId);
                    projectile.AddComponent<ColliderComponent>("PROJECTILE", startX, startY, pw, ph);
                    projectile.AddComponent<ProjectileEmitterComponent>(speed, angle, range, shouldLoop);
                }
                
                // 6. ScriptComponent
                sol::optional<sol::table> scriptOpt = components["script"];
                if (scriptOpt && scriptOpt->valid()) {
                    sol::table script = *scriptOpt;
                    std::string path = script.get_or<std::string>("path", "");
                    if (!path.empty()) {
                        entity.AddComponent<ScriptComponent>(path);
                    }
                }
                
                // 7. LabelComponent
                sol::optional<sol::table> labelOpt = components["label"];
                if (labelOpt && labelOpt->valid()) {
                    sol::table label = *labelOpt;
                    std::string text = label.get_or<std::string>("text", "");
                    std::string fontFamily = label.get_or<std::string>("fontFamily", "charriot-font");
                    sol::optional<sol::table> colorTable = label["color"];
                    SDL_Color color = { 255, 255, 255, 255 };
                    if (colorTable && colorTable->valid()) {
                        color.r = colorTable->get_or("r", 255);
                        color.g = colorTable->get_or("g", 255);
                        color.b = colorTable->get_or("b", 255);
                        color.a = colorTable->get_or("a", 255);
                    }
                    int lx = 0, ly = 0;
                    if (entity.HasComponent<TransformComponent>()) {
                        auto* trans = entity.GetComponent<TransformComponent>();
                        lx = static_cast<int>(trans->position.x);
                        ly = static_cast<int>(trans->position.y);
                    }
                    entity.AddComponent<LabelComponent>(lx, ly, text, fontFamily, color);
                }
            }
            
            if (name == "player") {
                player = &entity;
            }
        });
    }
}

void Game::ProcessInput(){

    SDL_PollEvent(&event);
    ImGui_ImplSDL2_ProcessEvent(&event);

    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard || io.WantCaptureMouse) {
        if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
            isRunning = false;
        }
        return;
    }

    switch (event.type){
        case SDL_QUIT: {
            isRunning = false;
            break;
        }
        case SDL_KEYDOWN: {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                isRunning = false;
            }
            if (event.key.keysym.sym == SDLK_F5) {
                fileWatcher->SetEnabled(!fileWatcher->IsEnabled());
                std::cout << "[Hot Reload] "
                          << (fileWatcher->IsEnabled() ? "ON" : "OFF") << std::endl;
            }
        }
        default:{
            break;
        }
    }
}


void Game::Update(){

    // wait until 16ms has ellapsed since last frame

    while (!SDL_TICKS_PASSED(SDL_GetTicks(), ticksOfLastFrame + FRAME_TARGET_TIME));

    // while loops arent really good for doing this because its gonna consume lot of cpu power , this is gonna
    // burn a lot of compute we really dont want that so we should find another way

    // delta time is the differnece in ticks from last frame converted to seconds
    float deltaTime = (SDL_GetTicks() - ticksOfLastFrame) / 1000.0f;

    //clamp delta time

    deltaTime = (deltaTime > 0.05f) ? 0.05f : deltaTime;


    // sets the new ticks for the current frame which is to be used in the next pass
    ticksOfLastFrame = SDL_GetTicks();

    fileWatcher->Update(deltaTime);

    if (GetIsPlaying()) {
        manager.Update(deltaTime);
        HandleCameraMovement();
        CheckCollisions();
    }


}


void Game::Render(){

    // 1. Render game world to viewport texture target
    if (viewportTexture) {
        SDL_SetRenderTarget(renderer, viewportTexture);
    }
    SDL_SetRenderDrawColor(renderer, 21, 21, 21, 255);
    SDL_RenderClear(renderer);

    if (!manager.HasNoEntities()) {
        manager.Render();
    }

    if (viewportTexture) {
        SDL_SetRenderTarget(renderer, NULL); // Restore default render target
    }

    // 2. Render default background for main window
    SDL_SetRenderDrawColor(renderer, 192, 192, 192, 255); // Win95 light gray
    SDL_RenderClear(renderer);

    // 3. Render ImGui UI
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    DrawEditorUI(viewportTexture);

    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);

    SDL_RenderPresent(renderer);

}

void Game::HandleCameraMovement(){

    if (!player) return;
    TransformComponent* mainPlayerTransform = player->GetComponent<TransformComponent>();
    camera.x =  mainPlayerTransform -> position.x - static_cast<int>(WINDOW_WIDTH/2);
    camera.y = mainPlayerTransform -> position.y - static_cast<int>(WINDOW_HEIGHT/2);



    camera.x = camera.x < 0 ? 0: camera.x;
    camera.y = camera.y < 0 ? 0: camera.y;
    camera.x = camera.x > camera.w ? camera.w : camera.x;
    camera.y = camera.y > camera.h ? camera.h : camera.y;


}



void Game::CheckCollisions() {
    CollisionType collisionType = manager.CheckCollisions();
    if (collisionType == PLAYER_ENEMY_COLLISION) {
        // ProcessGameOver();
    }
    if (collisionType == PLAYER_PROJECTILE_COLLISION) {
        // ProcessGameOver();
    }
    if (collisionType == PLAYER_LEVEL_COMPLETE_COLLISION) {
        // ProcessNextLevel(1);
    }
}

void Game::ProcessNextLevel(int levelNumber) {
    std::cout << "Next Level Triggered" << std::endl;
    // isRunning = false;
}

void Game::ProcessGameOver() {
    std::cout << "Game Over Triggered" << std::endl;
    // isRunning = false;
}


void Game::Destroy(){
    SDL_DestroyTexture(viewportTexture);

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
