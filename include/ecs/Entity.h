#ifndef ECS_ENTITY_H
#define ECS_ENTITY_H

#include <cstdint>
#include <typeindex>
#include <unordered_map>
#include <memory>
#include <vector>

class Component {
public:
    virtual ~Component() = default;
};

class Entity {
public:
    explicit Entity(uint32_t id) : id_(id) {}

    template<typename T, typename... Args>
    T& addComponent(Args&&... args) {
        static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
        auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *ptr;
        components_[std::type_index(typeid(T))] = std::move(ptr);
        return ref;
    }

    template<typename T>
    T* getComponent() {
        static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
        auto it = components_.find(std::type_index(typeid(T)));
        if (it != components_.end()) {
            return static_cast<T*>(it->second.get());
        }
        return nullptr;
    }

    template<typename T>
    const T* getComponent() const {
        static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
        auto it = components_.find(std::type_index(typeid(T)));
        if (it != components_.end()) {
            return static_cast<const T*>(it->second.get());
        }
        return nullptr;
    }

    template<typename T>
    bool hasComponent() const {
        return components_.find(std::type_index(typeid(T))) != components_.end();
    }

    template<typename T>
    void removeComponent() {
        components_.erase(std::type_index(typeid(T)));
    }

    uint32_t getId() const { return id_; }
    bool isActive() const { return active_; }
    void setActive(bool active) { active_ = active; }

private:
    uint32_t id_;
    bool active_ = true;
    std::unordered_map<std::type_index, std::unique_ptr<Component>> components_;
};

class System {
public:
    virtual void update(float deltaTime, std::vector<Entity*>& entities) = 0;
    virtual ~System() = default;
};

#endif // ECS_ENTITY_H
