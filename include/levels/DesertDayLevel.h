#ifndef LEVELS_DESERT_DAY_H
#define LEVELS_DESERT_DAY_H

#include "Level.h"

class DesertDayLevel : public Level {
public:
    void init(AssetManager& assets, GameState& state) override;
    void update(float deltaTime, AssetManager& assets, GameState& state) override;
    void render(sf::RenderWindow& window) override;
    void spawnObstacles(GameState& state, AssetManager& assets) override;
    Constants::LevelType getType() const override { return Constants::LevelType::DESERT_DAY; }

private:
    ParallaxSystem parallax_;
    ParticleSystem particles_;
    sf::Color skyColor_{135, 206, 235};
    sf::Color groundColor_{210, 180, 140};
};

#endif
