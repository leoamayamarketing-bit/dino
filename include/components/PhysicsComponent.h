#ifndef COMPONENTS_PHYSICS_H
#define COMPONENTS_PHYSICS_H

#include "../ecs/Entity.h"
#include <SFML/System/Vector2.hpp>

struct PhysicsComponent : public Component {
    sf::Vector2f velocity{0.0f, 0.0f};
    sf::Vector2f acceleration{0.0f, 0.0f};
    float mass = 1.0f;
    bool usesGravity = true;
    bool isGrounded = false;
    int jumpsAvailable = 2; // Double jump
    bool isDashing = false;
    float dashTimer = 0.0f;
    float dashCooldown = 0.0f;

    explicit PhysicsComponent() = default;
};

#endif
