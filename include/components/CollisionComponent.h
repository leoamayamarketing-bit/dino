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

    explicit CollisionComponent(const sf::FloatRect& rect = {}, const std::string& t = "")
        : bounds(rect), tag(t) {}

    void setBounds(const sf::FloatRect& rect) { bounds = rect; }
    void updatePosition(float x, float y) {
        bounds.left = x;
        bounds.top = y;
    }
};

#endif
