#ifndef TILECOMPONENET_H
#define TILECOMPONENET_H
#include "SDL2/SDL.h"
#include "../src/Component.h"
#include "../src/TextureManager.h"
#include "../src/AssetManager.h"
#include "SDL2/SDL_render.h"
#include "glm/glm.hpp"


class TileComponenet: public Component {
    public:
        SDL_Texture *texture;
        SDL_Rect sourceRectangle;
        SDL_Rect destinationRectangle;
        glm::vec2 position;

        TileComponenet(int sourceRectX , int sourceRectY , int x , int y , int tileSize , int tileScale, std::string assetTextureId){
            texture = Game::assetManager -> GetTexture(assetTextureId);
            sourceRectangle.x = sourceRectX;
            sourceRectangle.y = sourceRectY;
            sourceRectangle.w = tileSize;
            sourceRectangle.h = tileSize;

            destinationRectangle.x = x;
            destinationRectangle.y = y;
            destinationRectangle.w = tileSize * tileScale;
            destinationRectangle.h = tileSize * tileScale;

            position.x = x;
            position.y = y;

        }

        ~TileComponenet(){

            SDL_DestroyTexture(texture);

        }

        void Update (float deltaTime) override {

            destinationRectangle.x = position.x - Game::camera.x;
            destinationRectangle.y = position.y - Game::camera.y;

        }

        void Render () override {

            TextureManager::Draw(texture, sourceRectangle,destinationRectangle, SDL_FLIP_NONE);

        }

};


#endif
