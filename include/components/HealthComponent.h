#ifndef COMPONENTS_HEALTH_H
#define COMPONENTS_HEALTH_H

#include "../ecs/Entity.h"

struct HealthComponent : public Component {
    int health = 3;
    int maxHealth = 3;
    bool invulnerable = false;
    float invulnerabilityTimer = 0.0f;
    float invulnerabilityDuration = 1.0f;
    bool destroyOnDeath = false;

    explicit HealthComponent(int hp = 3, int maxHp = 3)
        : health(hp), maxHealth(maxHp) {}

    void takeDamage(int dmg) {
        if (!invulnerable) {
            health -= dmg;
            invulnerable = true;
            invulnerabilityTimer = invulnerabilityDuration;
        }
    }

    void heal(int amount) {
        health = std::min(health + amount, maxHealth);
    }

    bool isDead() const { return health <= 0; }

    void update(float dt) {
        if (invulnerable) {
            invulnerabilityTimer -= dt;
            if (invulnerabilityTimer <= 0.0f) {
                invulnerable = false;
            }
        }
    }
};

#endif
