#include "../../include/systems/PhysicsSystem.h"
#include "../../include/components/TransformComponent.h"
#include "../../include/components/PhysicsComponent.h"

void PhysicsSystem::update(float deltaTime, std::vector<Entity*>& entities) {
    for (auto* entity : entities) {
        auto* transform = entity->getComponent<TransformComponent>();
        auto* physics = entity->getComponent<PhysicsComponent>();

        if (!transform || !physics) continue;

        // Apply gravity
        if (physics->usesGravity && !physics->isGrounded) {
            physics->velocity.y += gravity_ * deltaTime;
        }

        // Apply acceleration
        physics->velocity.x += physics->acceleration.x * deltaTime;
        physics->velocity.y += physics->acceleration.y * deltaTime;

        // Update position
        transform->position.x += physics->velocity.x * deltaTime;
        transform->position.y += physics->velocity.y * deltaTime;

        // Ground check
        if (transform->position.y >= groundY_) {
            transform->position.y = groundY_;
            physics->velocity.y = 0.0f;
            if (!physics->isGrounded) {
                physics->isGrounded = true;
                physics->jumpsAvailable = 2;
            }
        }

        // Update dash
        if (physics->isDashing) {
            physics->dashTimer -= deltaTime;
            if (physics->dashTimer <= 0.0f) {
                physics->isDashing = false;
            }
        }
        if (physics->dashCooldown > 0.0f) {
            physics->dashCooldown -= deltaTime;
        }
    }
}
