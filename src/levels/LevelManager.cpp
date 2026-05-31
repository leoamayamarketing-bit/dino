#include "../../include/levels/LevelManager.h"

LevelManager::LevelManager() = default;

void LevelManager::init(Constants::LevelType type, AssetManager& assets, GameState& state) {
    currentType_ = type;
    currentLevel_ = createLevel(type);
    if (currentLevel_) {
        currentLevel_->init(assets, state);
    }
}

void LevelManager::update(float deltaTime, AssetManager& assets, GameState& state) {
    if (currentLevel_) {
        currentLevel_->update(deltaTime, assets, state);
    }
}

void LevelManager::render(sf::RenderWindow& window) {
    if (currentLevel_) {
        currentLevel_->render(window);
    }
}

void LevelManager::switchLevel(Constants::LevelType type, AssetManager& assets, GameState& state) {
    currentType_ = type;
    currentLevel_ = createLevel(type);
    if (currentLevel_) {
        currentLevel_->init(assets, state);
    }
}

Constants::LevelType LevelManager::getCurrentLevelType() const {
    return currentType_;
}

Constants::LevelType LevelManager::getEffectiveTheme() const {
    if (currentLevel_) {
        return currentLevel_->getEffectiveTheme();
    }
    return currentType_;
}

std::unique_ptr<Level> LevelManager::createLevel(Constants::LevelType type) {
    switch (type) {
        case Constants::LevelType::DESERT_DAY:
            return std::make_unique<DesertDayLevel>();
        case Constants::LevelType::DESERT_NIGHT:
            return std::make_unique<DesertNightLevel>();
        case Constants::LevelType::CAVE:
            return std::make_unique<CaveLevel>();
        case Constants::LevelType::VOLCANO:
            return std::make_unique<VolcanoLevel>();
        case Constants::LevelType::INFINITE:
            return std::make_unique<InfiniteLevel>();
        case Constants::LevelType::GRAYSCALE:
            return std::make_unique<GrayscaleLevel>();
    }
    return nullptr;
}
