#include "../../include/systems/PowerUpSystem.h"
#include "../../include/components/TransformComponent.h"
#include "../../include/components/PowerUpComponent.h"
#include <cmath>
#include "../../include/components/SpriteComponent.h"

PowerUpSystem::PowerUpSystem(GameState& state) : state_(state) {}

void PowerUpSystem::update(float deltaTime, std::vector<Entity*>& entities) {
    for (auto* entity : entities) {
        auto* pu = entity->getComponent<PowerUpComponent>();
        auto* transform = entity->getComponent<TransformComponent>();
        auto* sprite = entity->getComponent<SpriteComponent>();

        if (!pu || !transform) continue;

        if (pu->collected) continue;

        // Pulsing animation
        static float timer = 0.0f;
        timer += deltaTime;
        float pulse = 1.0f + std::sin(timer * 4.0f + transform->position.x) * 0.15f;
        if (sprite) {
            sprite->sprite.setScale(pulse, pulse);
        }

        // Float up and down
        transform->position.y += std::sin(timer * 2.0f + transform->position.x * 0.1f) * 0.3f;

        // Scroll with world (move left so the player can reach them)
        transform->position.x -= state_.currentSpeed * deltaTime;
    }
}
