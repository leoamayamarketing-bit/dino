#ifndef LEVELS_DESERT_NIGHT_H
#define LEVELS_DESERT_NIGHT_H

#include "Level.h"

class DesertNightLevel : public Level {
public:
    void init(AssetManager& assets, GameState& state) override;
    void update(float deltaTime, AssetManager& assets, GameState& state) override;
    void render(sf::RenderWindow& window) override;
    void spawnObstacles(GameState& state, AssetManager& assets) override;
    Constants::LevelType getType() const override { return Constants::LevelType::DESERT_NIGHT; }

private:
    ParallaxSystem parallax_;
    ParticleSystem fireflies_;
    sf::Color skyColor_{26, 26, 46};
    sf::Color groundColor_{45, 45, 68};
    float fireflyTimer_ = 0.0f;
};

#endif
