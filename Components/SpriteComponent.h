#ifndef SPRITECOMPONENT_H
#define SPRITECOMPONENT_H


#include "SDL2/SDL_rect.h"
#include "SDL2/SDL_render.h"
#include "../src/TextureManager.h"
#include "TransformComponent.h"
#include "../src/AssetManager.h"


class SpriteComponenet : public Component {

    private:
        TransformComponenet* Transform;
        SDL_Texture* texture;
        SDL_Rect sourceRectangle;
        SDL_Rect destinationRectangle;


    public:
        SDL_RendererFlip spriteFlip = SDL_FLIP_NONE;

        SpriteComponenet(const char* filePath){

            SetTexture(filePath);

        }

        void SetTexture(std::string assetTextureId){
            texture = Game::assetManager -> GetTexture(assetTextureId);
        }

        void Initialize() override {
            Transform = owner->GetComponenet<TransformComponenet>();
            sourceRectangle.x = 0;
            sourceRectangle.y = 0;
            sourceRectangle.w = Transform->width;
            sourceRectangle.h = Transform->height;
        }

        void Update(float deltaTime) override {

            destinationRectangle.x = (int)Transform->position.x;
            destinationRectangle.y = (int)Transform->position.y;
            destinationRectangle.w = Transform->width * Transform->scale;
            destinationRectangle.h = Transform->height* Transform->scale;

        }

        void Render() override {

            TextureManager::Draw(texture,sourceRectangle, destinationRectangle, spriteFlip);

        }
};




#endif
