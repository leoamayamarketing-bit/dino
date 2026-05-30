#ifndef LEVELS_VOLCANO_H
#define LEVELS_VOLCANO_H

#include "Level.h"

class VolcanoLevel : public Level {
public:
    void init(AssetManager& assets, GameState& state) override;
    void update(float deltaTime, AssetManager& assets, GameState& state) override;
    void render(sf::RenderWindow& window) override;
    void spawnObstacles(GameState& state, AssetManager& assets) override;
    Constants::LevelType getType() const override { return Constants::LevelType::VOLCANO; }

private:
    ParallaxSystem parallax_;
    ParticleSystem ashParticles_;
    ParticleSystem lavaParticles_;
    sf::Color skyColor_{74, 0, 0};
    sf::Color groundColor_{60, 20, 10};
    sf::Color lavaColor_{255, 69, 0};
    float lavaTimer_ = 0.0f;
    std::vector<sf::FloatRect> lavaPools_;
};

#endif
