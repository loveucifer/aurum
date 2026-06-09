#ifndef SPRITECOMPONENT_H
#define SPRITECOMPONENT_H


#include "SDL2/SDL_rect.h"
#include "SDL2/SDL_render.h"
#include "../src/TextureManager.h"
#include "SDL2/SDL_timer.h"
#include "TransformComponent.h"
#include "../src/AssetManager.h"
#include "../src/Animation.h"


class SpriteComponenet : public Component {

    private:
        TransformComponenet* Transform;
        SDL_Texture* texture;
        SDL_Rect sourceRectangle;
        SDL_Rect destinationRectangle;
        bool isAnimated;
        int numFrames;
        int animationSpeed;
        bool isFixed;  // same position in scren for eg for texts etc ui and other stuff can use this
        std::map<std::string, Animation> animations;
        std::string currentAnimationName;
        unsigned int animationIndex = 0;


    public:
        SDL_RendererFlip spriteFlip = SDL_FLIP_NONE;

        SpriteComponenet(std::string assetTextureId){
            isAnimated = false;
            isFixed = false;
            SetTexture(assetTextureId);

        }

        SpriteComponenet(std::string id, int numFrames, int animationSpeed, bool hasDirections, bool isFixed ){
            this -> isAnimated = true;
            this -> numFrames = numFrames;
            this -> animationSpeed = animationSpeed;
            this -> isFixed = isFixed;

            if (hasDirections) {

                Animation downAnimation = Animation(0, numFrames,animationSpeed);
                Animation rightAnimation = Animation(1, numFrames,animationSpeed);
                Animation leftAnimation = Animation(2, numFrames,animationSpeed);
                Animation upAnimation = Animation(3, numFrames,animationSpeed);

                animations.emplace("DownAnimation", downAnimation);
                animations.emplace("RightAnimation", rightAnimation);
                animations.emplace("LeftAnimation", leftAnimation);
                animations.emplace("UpAnimation", upAnimation);
                this -> animationIndex = 0;
                this -> currentAnimationName = "DownAnimation";

            } else {
                Animation singleAnimation = Animation(0,numFrames,animationSpeed);
                animations.emplace("Singleanimation",singleAnimation);
                this -> animationIndex = 0;
                this -> currentAnimationName = "Singleanimation";
            }
            Play(this->currentAnimationName);
            SetTexture(id);
        }


        void Play(std::string animationName){
            numFrames = animations[animationName].numFrames;
            animationIndex = animations[animationName].index;
            animationSpeed = animations[animationName].animationSpeed;
            currentAnimationName = animationName;
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

            if (isAnimated) {
                sourceRectangle.x = (sourceRectangle.w * static_cast<int>((SDL_GetTicks()/ animationSpeed) % numFrames ));
            }

            sourceRectangle.y = animationIndex * Transform -> height;

            destinationRectangle.x = static_cast<int>(Transform->position.x) - (isFixed?0 :Game::camera.x);
            destinationRectangle.y = static_cast<int>(Transform->position.y) - (isFixed ?0 :Game::camera.y);
            destinationRectangle.w = Transform->width * Transform->scale;
            destinationRectangle.h = Transform->height* Transform->scale;

        }

        void Render() override {

            TextureManager::Draw(texture,sourceRectangle, destinationRectangle, spriteFlip);

        }
};




#endif
