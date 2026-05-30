#ifndef ENTITIES_PLAYER_FACTORY_H
#define ENTITIES_PLAYER_FACTORY_H

#include "../ecs/Entity.h"
#include "../core/Constants.h"
#include "../core/AssetManager.h"

class PlayerFactory {
public:
    static std::unique_ptr<Entity> createPlayer(Constants::DinoType type,
                                                  AssetManager& assets,
                                                  float startX = 200.0f,
                                                  float startY = 600.0f);
};

#endif
