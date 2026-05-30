#ifndef ENTITIES_OBSTACLE_FACTORY_H
#define ENTITIES_OBSTACLE_FACTORY_H

#include "../ecs/Entity.h"
#include "../core/AssetManager.h"
#include <vector>

struct GameState;

class ObstacleFactory {
public:
    static std::unique_ptr<Entity> createSmallCactus(AssetManager& assets, float x);
    static std::unique_ptr<Entity> createLargeCactus(AssetManager& assets, float x);
    static std::unique_ptr<Entity> createRock(AssetManager& assets, float x);
    static std::unique_ptr<Entity> createStalactite(AssetManager& assets, float x);
    static std::unique_ptr<Entity> createCoin(AssetManager& assets, float x, float y);
    static std::unique_ptr<Entity> createPowerUp(AssetManager& assets,
                                                   Constants::PowerUpType type,
                                                   float x, float y);

    static std::vector<std::unique_ptr<Entity>> generateObstaclePattern(
        Constants::LevelType level, AssetManager& assets,
        float startX, int patternLength);

    // Spawning helpers for Game.cpp
    static void spawnRandomObstacle(AssetManager& assets, GameState& state, Constants::LevelType level);
    static void spawnCoinCluster(AssetManager& assets, GameState& state, float x, float y, int count);
    static void spawnRandomPowerUp(AssetManager& assets, GameState& state);
};

#endif
