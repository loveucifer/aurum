#include <cstddef>
#include <iostream>
#include "./Constants.h"
#include "./Game.h"
#include "../lib/glm/glm.hpp"
#include "EntityManager.h"

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>


EntityManager manager;
SDL_Renderer* Game::renderer;


Game::Game(){
    this -> isRunning = false;
}



Game::~Game(){

}




bool Game::IsRunning() const {
    return this->isRunning;
}

glm::vec2  ProjectilePosition = glm::vec2(0.0f,0.0f);
glm::vec2  ProjectileVelocity = glm::vec2(20.0f,20.0f);

void Game::Initialize(int width , int height ){

    if ( SDL_Init(SDL_INIT_EVERYTHING) != 0 ) {
        std::cerr << "Error Initializing SDL" << std::endl;
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


    isRunning = true;
    return;

}


void Game::ProcessInput(){
    SDL_Event event;
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

    // uses deltatime to update game objects

    ProjectilePosition = glm::vec2(
        ProjectilePosition.x + ProjectileVelocity.x * deltaTime,
        ProjectilePosition.y + ProjectileVelocity.y * deltaTime
    );

}


void Game::Render(){

    SDL_SetRenderDrawColor(renderer, 21, 21, 21 ,255);
    SDL_RenderClear(renderer);

    SDL_Rect projectile {
        (int)ProjectilePosition.x,
        (int)ProjectilePosition.y,
        10,
        10
    };

    SDL_SetRenderDrawColor(renderer , 255 , 255 , 255, 255);
    SDL_RenderFillRect(renderer, &projectile);
    SDL_RenderPresent(renderer);

}


void Game::Destroy(){
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
