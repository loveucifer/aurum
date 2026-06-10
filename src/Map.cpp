#include "Constants.h"
#include "Entity.h"
#include "Map.h"
#include "EntityManager.h"
#include <cstdlib>
#include <fstream>
#include <string>
#include "../Components/TileComponent.h"

extern EntityManager manager;


Map::Map(std::string texutreId, int scale, int tileSize ){
    this -> textureId = texutreId;
    this -> scale = scale;
    this -> tileSize = tileSize;
}


void Map::LoadMap(std::string filePath , int mapSizeX, int mapSizeY){

    std::fstream mapFile;
    mapFile.open(filePath);
    if (!mapFile.is_open()) {
        std::cerr << "[Map] Error: Failed to open map file: " << filePath << std::endl;
        return;
    }

    for (int y = 0 ; y < mapSizeY ; y++){
        for (int x = 0; x < mapSizeX; x++) {

            char ch;
            mapFile.get(ch);
            int sourceRectangleY = (ch - '0') * tileSize;
            mapFile.get(ch);
            int sourceRectangleX = (ch - '0') * tileSize;
            AddTile(sourceRectangleX, sourceRectangleY, x * (scale * tileSize), y * (scale * tileSize));
            mapFile.ignore(); // ignore the commas

        }
    }
    mapFile.close();
}

void Map::AddTile(int sourceRectangleX, int sourceRectangleY , int x , int y){
    Entity& newTile(manager.AddEntity("Tile", TILEMAP_LAYER));
    newTile.AddComponent<TileComponent>(sourceRectangleX, sourceRectangleY, x , y ,tileSize , scale , textureId);

}
