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
    textures.clear();
    fonts.clear();
}

void AssetManager::AddTexture(std::string textureId, const char* fileName){
    textures.emplace(textureId,TextureManager::LoadTexture(fileName));
}

void AssetManager::AddFont(std::string fontId, const char* filePath, int fontSize){

    fonts.emplace(fontId,FontManager::LoadFont(filePath, fontSize));
}

TTF_Font* AssetManager::GetFont(std::string fontId){
    return fonts[fontId];
}

SDL_Texture* AssetManager::GetTexture(std::string textureId){
    return textures[textureId];
}
