#include <cstddef>
#include <iostream>
#include "./Constants.h"
#include "./Game.h"

#include "SDL_render.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>


Game::Game(){
    this -> isRunning = false;
}

Game::~Game(){

}

bool Game::IsRunning() const {
    return this->IsRunning();
}

float ProjectilePositionX = 0.0f;
float ProjectilePositionY = 0.0f;
float ProjectileVelocityX = 50.0f;
float ProjectileVelocityY = 50.0f;

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
