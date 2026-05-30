#ifndef COMPONENTS_ENEMY_H
#define COMPONENTS_ENEMY_H

#include "../ecs/Entity.h"
#include "../core/Constants.h"

struct EnemyComponent : public Component {
    Constants::EnemyType enemyType;
    float damageAmount = 1.0f;
    float baseSpeed = 0.0f;
    float moveRange = 0.0f;
    float moveTimer = 0.0f;
    float altitude = 0.0f; // For flying enemies
    float altitudeAmplitude = 0.0f;
    float altitudeFrequency = 0.0f;
    bool movingForward = true;

    explicit EnemyComponent(Constants::EnemyType type)
        : enemyType(type) {
        switch (type) {
            case Constants::EnemyType::SMALL_CACTUS:
                damageAmount = 1.0f;
                break;
            case Constants::EnemyType::LARGE_CACTUS:
                damageAmount = 2.0f;
                break;
            case Constants::EnemyType::PTERODACTYL:
                damageAmount = 1.0f;
                altitudeAmplitude = 80.0f;
                altitudeFrequency = 2.0f;
                break;
            case Constants::EnemyType::ROLLING_ROCK:
                damageAmount = 2.0f;
                baseSpeed = 200.0f;
                break;
            case Constants::EnemyType::GROUND_ENEMY:
                damageAmount = 3.0f;
                baseSpeed = 150.0f;
                moveRange = 200.0f;
                break;
        }
    }
};

#endif
