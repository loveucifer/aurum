#ifndef LUABINDINGS_H
#define LUABINDINGS_H

#include "../lib/lua/sol/sol.hpp"
#include "Entity.h"
#include "EntityManager.h"
#include "Component.h"
#include "Constants.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/ColliderComponent.h"
#include "Game.h"
#include "AssetManager.h"

namespace LuaBindings {

inline void RegisterAll(sol::state& lua, EntityManager& manager) {


    lua.new_usertype<glm::vec2>("Vec2",
        sol::constructors<glm::vec2(), glm::vec2(float, float)>(),
        "x", &glm::vec2::x,
        "y", &glm::vec2::y
    );

    lua.new_enum("Layer",
        "TILEMAP",    TILEMAP_LAYER,
        "VEGETATION", VEGETATION_LAYER,
        "ENEMY",      ENEMY_LAYER,
        "OBSTACLE",   OBSTACLE_LAYER,
        "PLAYER",     PLAYER_LAYER,
        "PROJECTILE", PROJECTILE_LAYER,
        "GUI",        GUI_LAYER
    );


    lua.new_usertype<TransformComponent>("TransformComponent",
        "position", &TransformComponent::position,
        "velocity", &TransformComponent::velocity,
        "width",    &TransformComponent::width,
        "height",   &TransformComponent::height,
        "scale",    &TransformComponent::scale
    );


    lua.new_usertype<SpriteComponent>("SpriteComponent",
        "Play",       &SpriteComponent::Play,
        "SetTexture", &SpriteComponent::SetTexture,
        "spriteFlip", &SpriteComponent::spriteFlip
    );


    lua.new_usertype<ColliderComponent>("ColliderComponent",
        "colliderTag", &ColliderComponent::colliderTag,
        "collider",    &ColliderComponent::collider
    );


    lua.new_usertype<Entity>("Entity",
        "name",      &Entity::name,
        "layer",     &Entity::layer,
        "IsActive",  &Entity::IsActive,
        "Destroy",   &Entity::Destroy,


        "GetTransform", &Entity::GetComponent<TransformComponent>,
        "GetSprite",    &Entity::GetComponent<SpriteComponent>,
        "GetCollider",  &Entity::GetComponent<ColliderComponent>,

        "HasTransform", &Entity::HasComponent<TransformComponent>,
        "HasSprite",    &Entity::HasComponent<SpriteComponent>,
        "HasCollider",  &Entity::HasComponent<ColliderComponent>,


        "AddTransform", [](Entity& e, int px, int py, int vx, int vy, int w, int h, int s) -> TransformComponent& {
            return e.AddComponent<TransformComponent>(px, py, vx, vy, w, h, s);
        },
        "AddSprite", sol::overload(
            [](Entity& e, std::string textureId) -> SpriteComponent& {
                return e.AddComponent<SpriteComponent>(textureId);
            },
            [](Entity& e, std::string id, int frames, int speed, bool dirs, bool fixed) -> SpriteComponent& {
                return e.AddComponent<SpriteComponent>(id, frames, speed, dirs, fixed);
            }
        ),
        "AddCollider", [](Entity& e, std::string tag, int x, int y, int w, int h) -> ColliderComponent& {
            return e.AddComponent<ColliderComponent>(tag, x, y, w, h);
        }
    );


    lua.new_usertype<EntityManager>("EntityManager",
        "AddEntity",      &EntityManager::AddEntity,
        "GetEntities",    &EntityManager::GetEntities,
        "GetEntityCount", &EntityManager::GetEntityCount,
        "HasNoEntities",  &EntityManager::HasNoEntities
    );


    lua.new_usertype<AssetManager>("AssetManager",
        "AddTexture", &AssetManager::AddTexture,
        "AddFont",    &AssetManager::AddFont
    );


    lua.set("entities", &manager);
    lua.set("assets", Game::assetManager);

    lua.set_function("getCameraX", []() { return Game::camera.x; });
    lua.set_function("getCameraY", []() { return Game::camera.y; });


    lua.set_function("isKeyDown", [](int sdlKeycode) -> bool {
        const Uint8* keystate = SDL_GetKeyboardState(NULL);
        return keystate[SDL_GetScancodeFromKey(sdlKeycode)];
    });


    lua.set_function("log", [](const std::string& msg) {
        std::cout << "[Lua] " << msg << std::endl;
    });

    lua.set("WINDOW_WIDTH",  WINDOW_WIDTH);
    lua.set("WINDOW_HEIGHT", WINDOW_HEIGHT);
}

}

#endif
