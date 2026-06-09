#include "AssetManager.h"
#include "Entity.h"
#include "SDL2/SDL_render.h"
#include "TextureManager.h"


AssetManager::AssetManager(EntityManager* manager): manager(manager){

}

void AssetManager::ClearData(){
    textures.clear();
}

void AssetManager::AddTexture(std::string textureId, const char* fileName){
    textures.emplace(textureId,TextureManager::LoadTexture(fileName));
}

SDL_Texture* AssetManager::GetTexture(std::string textureId){
    return textures[textureId];
}
