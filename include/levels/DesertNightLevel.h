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
    sf::Color skyColor_{26, 26, 46};
    sf::Color groundColor_{45, 45, 68};

    // Moon with eclipse
    sf::Vector2f moonPos_{900.0f, 100.0f};
    float moonRadius_ = 45.0f;
    float eclipseOffsetX_ = 18.0f;   // shadow offset for crescent/eclipse
    float eclipseOffsetY_ = -10.0f;

    // Mars
    sf::Vector2f marsPos_{200.0f, 180.0f};
    float marsRadius_ = 14.0f;
    float marsPhase_ = 0.0f;  // for subtle pulsing

    // Background decorative stars with twinkle
    bool starsGenerated_ = false;
    std::vector<sf::Vector2f> starPositions_;
    std::vector<float> starBrightness_;
    std::vector<float> starPhase_;   // per-star phase offset for twinkle
    float starTwinkleTimer_ = 0.0f;
};

#endif
