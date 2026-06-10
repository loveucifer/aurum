#include "AssetManager.h"
#include "Entity.h"
#include "FontManager.h"
#include "SDL2/SDL_render.h"
#include "SDL2/SDL_ttf.h"
#include "TextureManager.h"


AssetManager::AssetManager(EntityManager* manager): manager(manager){

}

AssetManager::~AssetManager() {
    ClearData();
}

void AssetManager::ClearData(){
    for (auto& pair : textures) {
        if (pair.second) {
            SDL_DestroyTexture(pair.second);
        }
    }
    textures.clear();

    for (auto& pair : fonts) {
        if (pair.second) {
            TTF_CloseFont(pair.second);
        }
    }
    fonts.clear();
}

void AssetManager::AddTexture(std::string textureId, const char* fileName){
    auto it = textures.find(textureId);
    if (it != textures.end()) {
        if (it->second) {
            SDL_DestroyTexture(it->second);
        }
        textures.erase(it);
    }
    textures.emplace(textureId, TextureManager::LoadTexture(fileName));
}

void AssetManager::AddFont(std::string fontId, const char* filePath, int fontSize){
    auto it = fonts.find(fontId);
    if (it != fonts.end()) {
        if (it->second) {
            TTF_CloseFont(it->second);
        }
        fonts.erase(it);
    }
    fonts.emplace(fontId, FontManager::LoadFont(filePath, fontSize));
}

TTF_Font* AssetManager::GetFont(std::string fontId){
    return fonts[fontId];
}

SDL_Texture* AssetManager::GetTexture(std::string textureId){
    return textures[textureId];
}
