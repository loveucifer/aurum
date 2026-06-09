#ifndef ENTITY_H
#define ENTITY_H

#include "Component.h"
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
        std::vector<Component*> componenets;
        std::map<const std::type_info*, Component*> componenetTypeMap;

    public:
        std::string name;
        Entity(EntityManager& manager);
        Entity(EntityManager& manager, std::string name);
        void Update(float deltaTime);
        void Render();
        void Destroy();
        bool IsActive() const;


        template <typename T, typename...TArgs>
        T& AddComponenet(TArgs&&...args){
            T* NewComponenet(new T(std::forward<TArgs>(args)...));
            NewComponenet -> owner = this;
            // this is the owner of that specific componenet whatever it may be
            componenets.emplace_back(NewComponenet);
            componenetTypeMap[&typeid(*NewComponenet)] = NewComponenet;
            NewComponenet -> Initialize();
            return *NewComponenet;
        }
        template<typename T>
        T* GetComponenet(){
            return static_cast<T*>(componenetTypeMap[&typeid(T)]);

        }
};

#endif
