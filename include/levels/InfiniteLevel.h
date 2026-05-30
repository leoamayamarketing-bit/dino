#ifndef LEVELS_INFINITE_H
#define LEVELS_INFINITE_H

#include "Level.h"
#include <random>

class InfiniteLevel : public Level {
public:
    void init(AssetManager& assets, GameState& state) override;
    void update(float deltaTime, AssetManager& assets, GameState& state) override;
    void render(sf::RenderWindow& window) override;
    void spawnObstacles(GameState& state, AssetManager& assets) override;
    Constants::LevelType getType() const override { return Constants::LevelType::INFINITE; }
    Constants::LevelType getEffectiveTheme() const override { return currentTheme_; }

private:
    ParallaxSystem parallax_;
    std::mt19937 rng_{42};
    float difficulty_ = 1.0f;
    float themeChangeTimer_ = 0.0f;
    Constants::LevelType currentTheme_ = Constants::LevelType::DESERT_DAY;
    sf::Color bgColor_{135, 206, 235};

    void changeTheme();
};

#endif
