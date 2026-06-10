#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL_render.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "ScriptManager.h"
#include "FileWatcher.h"


class AssetManager;

class Game {
    private:
        bool isRunning;
        SDL_Window *window;
        SDL_Texture *viewportTexture;


    public:
        Game();
        ~Game();
        static ScriptManager* scriptManager;
        static FileWatcher*    fileWatcher;
        int ticksOfLastFrame;
        bool IsRunning() const;
        static SDL_Renderer *renderer;
        static AssetManager* assetManager;
        static SDL_Event event;
        static SDL_Rect camera;
        void LoadLevel (int levelNumber);
        void Initialize(int width , int height);
        void ProcessInput();
        void Update();
        void Render();
        void ProcessNextLevel(int levelNumber);
        void ProcessGameOver();
        void HandleCameraMovement();
        void CheckCollisions();
        void Destroy();

};

extern Game* gameInstance;

#endif
