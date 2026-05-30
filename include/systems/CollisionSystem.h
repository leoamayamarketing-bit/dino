#ifndef SYSTEMS_COLLISION_H
#define SYSTEMS_COLLISION_H

#include "../ecs/Entity.h"
#include <SFML/Graphics/Rect.hpp>
#include <functional>
#include <string>
#include <vector>

struct CollisionPair {
    Entity* a;
    Entity* b;
};

class CollisionSystem : public System {
public:
    using CollisionCallback = std::function<void(Entity*, Entity*)>;

    void update(float deltaTime, std::vector<Entity*>& entities) override;

    void onCollision(const std::string& tagA, const std::string& tagB, CollisionCallback callback);

private:
    struct CollisionRule {
        std::string tagA;
        std::string tagB;
        CollisionCallback callback;
    };

    std::vector<CollisionRule> collisionRules_;
    std::vector<CollisionPair> activeCollisions_;

    bool checkAABB(const sf::FloatRect& a, const sf::FloatRect& b) const;
};

#endif
