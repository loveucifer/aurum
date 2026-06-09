#include "EntityManager.h"
#include "Constants.h"
#include "Entity.h"
#include <vector>
#include "Collision.h"
#include "Component.h"
#include <stdio.h>
#include <iostream>
#include "../Components/ColliderComponent.h"

void EntityManager::ClearData() {
    for (auto& entity:entities) {
        entity ->Destroy();
    }
}

bool EntityManager::HasNoEntities(){
    return entities.size() == 0;
}

void EntityManager::Update(float deltaTime){
    for (auto& entity:entities) {
        entity -> Update(deltaTime);
    }
}

std::vector<Entity*> EntityManager::GetEntitiesByLayer(LayerType layer) const {
    std::vector<Entity*> selectedEntities;
    for (auto& entity:entities) {
        if (entity -> layer == layer ) {

            selectedEntities.emplace_back(entity);

        }
    }
    return selectedEntities;
}

void EntityManager::Render(){
    for (int layerNumber = 0 ; layerNumber < NUM_LAYERS ; layerNumber++) {
        for (auto& entity: GetEntitiesByLayer(static_cast<LayerType>(layerNumber))) {
            entity ->Render();
        }
    }
}

void EntityManager::ListAllEntities() const {
    unsigned int i = 0;
    for (auto& entity: entities) {
        std::cout << "Entity[" << i << "]: " << entity->name << std::endl;
        entity->ListAllComponents();
        i++;
    }
}

std::string EntityManager::CheckEntityCollisions(Entity& myEntity) const {

    ColliderComponent* myCollider = myEntity.Getcomponent<ColliderComponent>();
    for (auto& entity:entities) {

        if(entity->name.compare(myEntity.name) != 0 && entity->name.compare("Tile")!=0){

        if (entity ->HasComponent<ColliderComponent>()) {

            ColliderComponent* OtherCollider = entity->Getcomponent<ColliderComponent>();

            if (Collision::CheckRectangleCollision(myCollider->collider, OtherCollider->collider)){

                return OtherCollider->colliderTag;


            }
        }
    }
 }
 return std::string();
}

Entity& EntityManager::AddEntity(std::string entityName , LayerType layer){
    Entity *entity = new Entity(*this,entityName, layer);
    entities.emplace_back(entity);
    return *entity;
}

std::vector<Entity*> EntityManager::GetEntities() const {
    return entities;
}


unsigned int EntityManager::GetEntityCount  () const{
    return entities.size();
}
