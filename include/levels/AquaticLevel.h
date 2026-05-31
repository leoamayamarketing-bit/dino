#ifndef LEVELS_AQUATIC_H
#define LEVELS_AQUATIC_H

#include "Level.h"

struct Seaweed {
    float x;
    float height;
    float phase;       // wave animation phase offset
    sf::Color color;
};

struct Coral {
    float x;
    float radius;
    sf::Color color;
};

class AquaticLevel : public Level {
public:
    void init(AssetManager& assets, GameState& state) override;
    void update(float deltaTime, AssetManager& assets, GameState& state) override;
    void render(sf::RenderWindow& window) override;
    void spawnObstacles(GameState& state, AssetManager& assets) override;
    Constants::LevelType getType() const override { return Constants::LevelType::AQUATIC; }

private:
    ParallaxSystem parallax_;
    ParticleSystem bubbles_;
    ParticleSystem lightRays_;

    // Seaweed blades (generated once in init())
    std::vector<Seaweed> seaweeds_;
    std::vector<Coral> corals_;

    // Sand ground detail offset
    float sandOffset_ = 0.0f;

    // Bubble timer
    float bubbleTimer_ = 0.0f;
    float lightRayTimer_ = 0.0f;

    // Sky / water gradient colors
    sf::Color shallowColor_{20, 100, 160};   // lighter blue at top
    sf::Color deepColor_{5, 30, 60};         // darker blue at bottom
    sf::Color groundColor_{60, 70, 50};      // sandy/dark seabed
};

#endif
