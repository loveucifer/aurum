#ifndef COLLISION_H
#define COLLISION_H


#include "SDL2/SDL_rect.h"


class Collision {
    public:
        static bool CheckRectangleCollision (const SDL_Rect& rectangleA , const SDL_Rect& rectangleB );

        // AABB axis alligned boundery box

};


#endif
