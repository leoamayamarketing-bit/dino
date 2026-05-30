#include "../../include/systems/CoinSystem.h"
#include "../../include/components/TransformComponent.h"
#include "../../include/components/CoinComponent.h"
#include <cmath>
#include "../../include/components/SpriteComponent.h"
#include "../../include/components/CollisionComponent.h"

CoinSystem::CoinSystem(GameState& state) : state_(state) {}

void CoinSystem::update(float deltaTime, std::vector<Entity*>& entities) {
    for (auto* entity : entities) {
        auto* coin = entity->getComponent<CoinComponent>();
        auto* transform = entity->getComponent<TransformComponent>();
        auto* sprite = entity->getComponent<SpriteComponent>();

        if (!coin || !transform) continue;

        if (coin->collected) continue;

        // Animate coin (bobbing)
        static float globalTimer = 0.0f;
        globalTimer += deltaTime;
        transform->position.y += std::sin(globalTimer * 3.0f + transform->position.x) * 0.5f;

        // Rotate coin via scale
        if (sprite) {
            float scaleX = std::abs(std::cos(globalTimer * 2.0f + transform->position.x * 0.1f));
            if (scaleX < 0.1f) scaleX = 0.1f;
            sprite->sprite.setScale(scaleX, 1.0f);
        }

        // Scroll with world (handled by EnemyAISystem or level)
    }
}
