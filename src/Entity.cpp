#include "Entity.h"
#include "Constants.h"


Entity::Entity(EntityManager& manager): manager(manager) {
    this -> isActive = true;
}

Entity::Entity(EntityManager& manager, std::string name,LayerType layer): manager(manager), name(name) , layer(layer) {
    this -> isActive = true;
}

void Entity::Update(float deltaTime){
    for (auto& component:componenets) {
        component -> Update(deltaTime);
    }

}


void Entity::Render(){
    for (auto&component:componenets) {
        component -> Render();
    }
}


void Entity::Destroy(){
    this -> isActive = false;
}


bool Entity::IsActive() const {
    return this -> isActive;
}
