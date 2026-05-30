#ifndef LEVELS_LEVELMANAGER_H
#define LEVELS_LEVELMANAGER_H

#include "Level.h"
#include "DesertDayLevel.h"
#include "DesertNightLevel.h"
#include "CaveLevel.h"
#include "VolcanoLevel.h"
#include "InfiniteLevel.h"
#include "../core/AssetManager.h"
#include "../core/GameState.h"
#include <memory>

class LevelManager {
public:
    LevelManager();

    void init(Constants::LevelType type, AssetManager& assets, GameState& state);
    void update(float deltaTime, AssetManager& assets, GameState& state);
    void render(sf::RenderWindow& window);

    void switchLevel(Constants::LevelType type, AssetManager& assets, GameState& state);
    Constants::LevelType getCurrentLevelType() const;

private:
    std::unique_ptr<Level> currentLevel_;
    Constants::LevelType currentType_ = Constants::LevelType::DESERT_DAY;

    std::unique_ptr<Level> createLevel(Constants::LevelType type);
};

#endif
