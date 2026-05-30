#ifndef SYSTEMS_COIN_H
#define SYSTEMS_COIN_H

#include "../ecs/Entity.h"
#include "../core/GameState.h"

class CoinSystem : public System {
public:
    explicit CoinSystem(GameState& state);
    void update(float deltaTime, std::vector<Entity*>& entities) override;

private:
    GameState& state_;
};

#endif
