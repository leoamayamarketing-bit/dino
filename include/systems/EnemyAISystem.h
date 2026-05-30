#ifndef SYSTEMS_ENEMY_AI_H
#define SYSTEMS_ENEMY_AI_H

#include "../ecs/Entity.h"

class EnemyAISystem : public System {
public:
    explicit EnemyAISystem(float& gameSpeed);
    void update(float deltaTime, std::vector<Entity*>& entities) override;

private:
    float& gameSpeed_;
};

#endif
