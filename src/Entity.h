#ifndef ENTITY_H
#define ENTITY_H

#include "Component.h"
#include "Constants.h"
#include <string>
#include <typeinfo>
#include <vector>
#include<map>

class Component;
class EntityManager;


class Entity{
    private:
        EntityManager& manager;
        bool isActive;
        std::vector<Component*> components;
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
        T& Addcomponent(TArgs&&...args){
            T* Newcomponent(new T(std::forward<TArgs>(args)...));
            Newcomponent -> owner = this;
            // this is the owner of that specific component whatever it may be
            components.emplace_back(Newcomponent);
            componentTypeMap[&typeid(*Newcomponent)] = Newcomponent;
            Newcomponent -> Initialize();
            return *Newcomponent;
        }

        template<typename T>
        T* Getcomponent(){
            return static_cast<T*>(componentTypeMap[&typeid(T)]);

        }

        template<typename T>
        bool HasComponent() const {
            return componentTypeMap.count(&typeid(T));
        }
};

#endif
