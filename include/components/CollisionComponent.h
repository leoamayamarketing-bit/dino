#ifndef COMPONENTS_COLLISION_H
#define COMPONENTS_COLLISION_H

#include "../ecs/Entity.h"
#include <SFML/Graphics/Rect.hpp>
#include <string>

struct CollisionComponent : public Component {
    sf::FloatRect bounds;
    bool isTrigger = false; // trigger = no physical response
    bool isStatic = false;
    std::string tag; // "player", "enemy", "coin", "powerup", "ground", "obstacle"

    // Damage on collision
    int damageAmount = 0;

    // Callback ID for collision events
    int collisionCallbackId = -1;

    // Local offset from transform position (origin) to collision box top-left.
    // Useful when sprite origin is not at its top-left (e.g. centered / bottom-anchored).
    sf::Vector2f localOffset{0.0f, 0.0f};

    explicit CollisionComponent(const sf::FloatRect& rect = {}, const std::string& t = "")
        : bounds(rect), tag(t) {}

    void setBounds(const sf::FloatRect& rect) { bounds = rect; }
    void updatePosition(float x, float y) {
        bounds.left = x + localOffset.x;
        bounds.top = y + localOffset.y;
    }
};

#endif
