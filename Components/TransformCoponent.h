#ifndef TRANSFORMCOMPONENT_H
#define TRANSFORMCOMPONENT_H

#include "../src/EntityManager.h"
#include "../lib/glm/glm.hpp"
#include "SDL_rect.h"
#include "SDL_render.h"
#include "SDL_surface.h"
#include "../src/Game.h"

class TransformComponenet: public Component {
    public:
        glm::vec2 position;
        glm::vec2 velocity;
        int width;
        int height;
        int scale;

        TransformComponenet(int posX , int posY , int velX , int velY , int w , int h , int s){
            position = glm::vec2(posX, posY);
            velocity = glm::vec2(velX, velY);
            width = w;
            height = h;
            scale = s;
        }

        void Initialize() override {

        }

        void Update(float deltaTime) override {
            // update position.velocity of a function of deltatime
        }

        void Render() override {
            SDL_Rect transformRectangle = {
                (int) position.x,
                (int) position.y,
                width,
                height
            };

            SDL_SetRenderDrawColor(Game::renderer, 255, 255,255, 255);
            SDL_RenderFillRect(Game::renderer, &transformRectangle);
        }
};




#endif
