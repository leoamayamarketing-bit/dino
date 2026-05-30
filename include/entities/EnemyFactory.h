#ifndef ENTITIES_ENEMY_FACTORY_H
#define ENTITIES_ENEMY_FACTORY_H

#include "../ecs/Entity.h"
#include "../core/Constants.h"
#include "../core/AssetManager.h"

class EnemyFactory {
public:
    static std::unique_ptr<Entity> createEnemy(Constants::EnemyType type,
                                                AssetManager& assets,
                                                float startX);
};

#endif
