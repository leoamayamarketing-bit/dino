#ifndef LEVELS_LEVELMANAGER_H
#define LEVELS_LEVELMANAGER_H

#include "Level.h"
#include "DesertDayLevel.h"
#include "DesertNightLevel.h"
#include "CaveLevel.h"
#include "VolcanoLevel.h"
#include "InfiniteLevel.h"
#include "GrayscaleLevel.h"
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
    /// Notify levels of hard mode change to adjust spawn intervals
    void setHardMode(bool enabled) { hardMode_ = enabled; }
    bool isHardMode() const { return hardMode_; }
    /// Returns the effective theme (accounts for InfiniteLevel switching themes)
    Constants::LevelType getEffectiveTheme() const;

private:
    std::unique_ptr<Level> currentLevel_;
    Constants::LevelType currentType_ = Constants::LevelType::DESERT_DAY;
    bool hardMode_ = false;

    std::unique_ptr<Level> createLevel(Constants::LevelType type);
};

#endif
