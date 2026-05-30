#include "../../include/systems/CollisionSystem.h"
#include "../../include/components/CollisionComponent.h"
#include "../../include/components/TransformComponent.h"
#include <algorithm>

void CollisionSystem::update(float, std::vector<Entity*>& entities) {
    std::vector<CollisionPair> currentCollisions;

    // Update collision bounds based on transforms
    for (auto* entity : entities) {
        auto* coll = entity->getComponent<CollisionComponent>();
        auto* trans = entity->getComponent<TransformComponent>();
        if (coll && trans) {
            coll->updatePosition(trans->position.x, trans->position.y);
        }
    }

    // Check collisions
    for (size_t i = 0; i < entities.size(); i++) {
        auto* a = entities[i];
        auto* ca = a->getComponent<CollisionComponent>();
        if (!ca) continue;

        for (size_t j = i + 1; j < entities.size(); j++) {
            auto* b = entities[j];
            auto* cb = b->getComponent<CollisionComponent>();
            if (!cb) continue;

            if (!checkAABB(ca->bounds, cb->bounds)) continue;

            currentCollisions.push_back({a, b});

            // Fire callbacks
            for (const auto& rule : collisionRules_) {
                if ((rule.tagA == ca->tag && rule.tagB == cb->tag) ||
                    (rule.tagA == cb->tag && rule.tagB == ca->tag)) {
                    rule.callback(a, b);
                }
            }
        }
    }

    activeCollisions_ = std::move(currentCollisions);
}

void CollisionSystem::onCollision(const std::string& tagA, const std::string& tagB,
                                    CollisionCallback callback) {
    collisionRules_.emplace_back(CollisionRule{tagA, tagB, callback});
}

bool CollisionSystem::checkAABB(const sf::FloatRect& a, const sf::FloatRect& b) const {
    return a.intersects(b);
}
