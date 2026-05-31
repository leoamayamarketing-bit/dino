#ifndef LEVELS_GRAYSCALE_H
#define LEVELS_GRAYSCALE_H

#include "Level.h"

struct GrayscaleBuilding {
    float x;
    float width;
    float height;
};

class GrayscaleLevel : public Level {
public:
    void init(AssetManager& assets, GameState& state) override;
    void update(float deltaTime, AssetManager& assets, GameState& state) override;
    void render(sf::RenderWindow& window) override;
    void spawnObstacles(GameState& state, AssetManager& assets) override;
    Constants::LevelType getType() const override { return Constants::LevelType::GRAYSCALE; }

private:
    ParallaxSystem parallax_;
    ParticleSystem smokeParticles_;
    sf::Color skyColor_{200, 200, 200};
    float fogTimer_ = 0.0f;
    std::vector<GrayscaleBuilding> citySilhouette_;
};

#endif
