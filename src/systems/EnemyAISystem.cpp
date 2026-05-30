#include "../../include/systems/EnemyAISystem.h"
#include "../../include/components/TransformComponent.h"
#include "../../include/components/SpriteComponent.h"
#include "../../include/components/EnemyComponent.h"
#include "../../include/components/CollisionComponent.h"
#include <cmath>

EnemyAISystem::EnemyAISystem(float& gameSpeed) : gameSpeed_(gameSpeed) {}

void EnemyAISystem::update(float deltaTime, std::vector<Entity*>& entities) {
    for (auto* entity : entities) {
        auto* enemy = entity->getComponent<EnemyComponent>();
        auto* transform = entity->getComponent<TransformComponent>();

        if (!enemy || !transform) continue;

        // Move enemies towards the left (player runs right, world scrolls left)
        transform->position.x -= gameSpeed_ * deltaTime;

        switch (enemy->enemyType) {
            case Constants::EnemyType::SMALL_CACTUS:
            case Constants::EnemyType::LARGE_CACTUS:
                // Static obstacles, just scroll with world
                break;

            case Constants::EnemyType::PTERODACTYL:
                // Flying: bobbing motion with wing flap animation
                enemy->moveTimer += deltaTime;
                transform->position.y = Constants::GROUND_Y - 180.0f +
                    std::sin(enemy->moveTimer * enemy->altitudeFrequency) * (enemy->altitudeAmplitude * 0.5f);
                
                // Wing flap: oscillate Y scale to simulate wings moving
                // Uses a faster sine wave (8x speed of bobbing) mapped to 0.65-1.0 range
                {
                    auto* sprite = entity->getComponent<SpriteComponent>();
                    if (sprite) {
                        float wingPhase = std::sin(enemy->moveTimer * 15.0f);
                        // Map -1..1 to 0.65..1.0 for vertical squash/stretch
                        float yScale = 0.825f + 0.175f * wingPhase;
                        sprite->sprite.setScale(1.0f, yScale);
                    }
                }
                break;

            case Constants::EnemyType::ROLLING_ROCK:
                // Rolling: additional horizontal movement towards player
                transform->position.x -= enemy->baseSpeed * deltaTime;
                // Rotation effect
                transform->rotation += deltaTime * 200.0f;
                break;

            case Constants::EnemyType::GROUND_ENEMY:
                // Patrol behavior
                enemy->moveTimer += deltaTime;
                if (enemy->movingForward) {
                    transform->position.x += enemy->baseSpeed * deltaTime;
                    if (enemy->moveTimer >= 2.0f) {
                        enemy->movingForward = false;
                        enemy->moveTimer = 0.0f;
                    }
                } else {
                    transform->position.x -= enemy->baseSpeed * deltaTime;
                    if (enemy->moveTimer >= 2.0f) {
                        enemy->movingForward = true;
                        enemy->moveTimer = 0.0f;
                    }
                }
                break;
        }
    }
}
