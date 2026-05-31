#ifndef LEVELS_CAVE_H
#define LEVELS_CAVE_H

#include "Level.h"

class CaveLevel : public Level {
public:
    void init(AssetManager& assets, GameState& state) override;
    void update(float deltaTime, AssetManager& assets, GameState& state) override;
    void render(sf::RenderWindow& window) override;
    void spawnObstacles(GameState& state, AssetManager& assets) override;
    Constants::LevelType getType() const override { return Constants::LevelType::CAVE; }

private:
    ParallaxSystem parallax_;
    ParticleSystem dripParticles_;
    sf::Color caveColor_{13, 13, 13};
    sf::Color groundColor_{30, 30, 40};
    std::vector<sf::Vector2f> stalactites_;
    float stalactiteTimer_ = 0.0f;
    float rockTimer_ = 0.0f;
    float rockInterval_ = 2.5f;
    ParticleSystem rockParticles_;
};

#endif
