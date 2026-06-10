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


EntityManager manager;
SDL_Renderer* Game::renderer;
AssetManager* Game::assetManager = new AssetManager(&manager);
ScriptManager* Game::scriptManager = new ScriptManager();
SDL_Event Game::event;
SDL_Rect Game::camera = {0,0,WINDOW_WIDTH, WINDOW_HEIGHT};
FileWatcher* Game::fileWatcher = new FileWatcher();
Map* map;
Entity* player = nullptr;


Game::Game(){
    this -> isRunning = false;
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
        0);

    if (!window){
        std::cerr << "Error creating SDL window" << std::endl;
        return;
    }


    // -1 means get default driver
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer ) {
        std::cerr << "Error creating SDL renderer" << std::endl;
        return;
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
    // Create entities (asset IDs match what Level1.assets defines)
    // -------------------------------------------------------
    player = &manager.AddEntity("chopper", PLAYER_LAYER);
    player->AddComponent<TransformComponent>(240, 106, 0, 0, 32, 32, 1);
    player->AddComponent<SpriteComponent>("chopper-texture", 2, 90, true, false);
    player->AddComponent<KeyboardControlComponent>("up", "right", "down", "left", "space");
    player->AddComponent<ColliderComponent>("PLAYER", 240, 106, 32, 32);

    Entity& tankEntity = manager.AddEntity("tank", ENEMY_LAYER);
    tankEntity.AddComponent<TransformComponent>(150, 495, 5, 0, 32, 32, 1);
    tankEntity.AddComponent<SpriteComponent>("tank-big-right-texture");
    tankEntity.AddComponent<ColliderComponent>("ENEMY", 150, 495, 32, 32);
    tankEntity.AddComponent<ScriptComponent>("assets/scripts/enemy_patrol.lua");

    Entity& projectile = manager.AddEntity("projectile", PROJECTILE_LAYER);
    projectile.AddComponent<TransformComponent>(150 + 16, 495 + 16, 0, 0, 4, 4, 1);
    projectile.AddComponent<SpriteComponent>("projectile-texture");
    projectile.AddComponent<ColliderComponent>("PROJECTILE", 150 + 16, 495 + 16, 4, 4);
    projectile.AddComponent<ProjectileEmitterComponent>(50, 270, 200, true);

    Entity& heliport = manager.AddEntity("Heliport", OBSTACLE_LAYER);
    heliport.AddComponent<TransformComponent>(470, 420, 0, 0, 32, 32, 1);
    heliport.AddComponent<SpriteComponent>("heliport-texture");
    heliport.AddComponent<ColliderComponent>("LEVEL_COMPLETE", 470, 420, 32, 32);

    Entity& radarEntity = manager.AddEntity("radar", GUI_LAYER);
    radarEntity.AddComponent<TransformComponent>(720, 0, 0, 0, 64, 64, 1);
    radarEntity.AddComponent<SpriteComponent>("radar-texture", 8, 150, false, true);

    Entity& labelLevelName = manager.AddEntity("LabelLevelName", GUI_LAYER);
    labelLevelName.AddComponent<LabelComponent>(10, 10, "First Level...", "charriot-font", WHITE_COLOR);
}

void Game::ProcessInput(){

    SDL_PollEvent(&event);

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

    manager.Update(deltaTime);

    HandleCameraMovement();

    CheckCollisions();


}


void Game::Render(){

    SDL_SetRenderDrawColor(renderer, 21, 21, 21 ,255);
    SDL_RenderClear(renderer);

    if (manager.HasNoEntities()){
        return;
    }

    manager.Render();



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
        ProcessGameOver();
    }
    if (collisionType == PLAYER_PROJECTILE_COLLISION) {

        ProcessGameOver();

    }
    if (collisionType == PLAYER_LEVEL_COMPLETE_COLLISION) {
        ProcessNextLevel(1);
    }
}

void Game::ProcessNextLevel(int levelNumber) {
    std::cout << "Next Level" << std::endl;
    isRunning = false;
}

void Game::ProcessGameOver() {
    std::cout << "Game Over" << std::endl;
    isRunning = false;
}


void Game::Destroy(){
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
