#include <iostream>
#include "TextureManager.h"
#include "SDL2/SDL_render.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_surface.h"

SDL_Texture* TextureManager::LoadTexture(const char *fileName){
    SDL_Surface* Surface = IMG_Load(fileName);
    if (!Surface) {
        std::cerr << "TextureManager::LoadTexture failed to load: " << fileName << " (" << IMG_GetError() << ")" << std::endl;
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(Game::renderer, Surface);
    SDL_FreeSurface(Surface);
    return texture;
}

void TextureManager::Draw(SDL_Texture* texture, SDL_Rect sourceRectangle, SDL_Rect destinationRectangle, SDL_RendererFlip flip){
    if (!texture) return;
    SDL_RenderCopyEx(Game::renderer, texture, &sourceRectangle, &destinationRectangle, 0.0, NULL ,flip );
}
