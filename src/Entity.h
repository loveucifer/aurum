#ifndef ENTITY_H
#define ENTITY_H

#include "Component.h"
#include "Constants.h"
#include <string>
#include <typeinfo>
#include <vector>
#include<map>
#include <memory>

class Component;
class EntityManager;


class Entity{
    private:
        EntityManager& manager;
        bool isActive;
        //std::vector<Component*> components;
        std::vector<std::unique_ptr<Component>> components;
        std::map<const std::type_info*, Component*> componentTypeMap;

    public:
        std::string name;
        LayerType layer;
        Entity(EntityManager& manager);
        Entity(EntityManager& manager, std::string name, LayerType layer);
        void Update(float deltaTime);
        void Render();
        void Destroy();
        bool IsActive() const;
        void ListAllComponents() const;


        template <typename T, typename...TArgs>
        T& AddComponent(TArgs&&...args){
        auto Newcomponent = std::make_unique<T>(std::forward<TArgs>(args)...);
        T* rawPtr = Newcomponent.get();
        rawPtr->owner = this;
        componentTypeMap[&typeid(*rawPtr)] = rawPtr;
        components.emplace_back(std::move(Newcomponent));
        rawPtr->Initialize();
        return *rawPtr;
        }

        template<typename T>
        T* GetComponent(){
            return static_cast<T*>(componentTypeMap[&typeid(T)]);

        }

        template<typename T>
        bool HasComponent() const {
            return componentTypeMap.count(&typeid(T));
        }
};

#endif
