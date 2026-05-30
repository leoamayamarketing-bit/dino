#ifndef SYSTEMS_POWERUP_H
#define SYSTEMS_POWERUP_H

#include "../ecs/Entity.h"
#include "../core/GameState.h"

class PowerUpSystem : public System {
public:
    explicit PowerUpSystem(GameState& state);
    void update(float deltaTime, std::vector<Entity*>& entities) override;

private:
    GameState& state_;
};

#endif
