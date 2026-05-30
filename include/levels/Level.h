#ifndef LEVELS_LEVEL_H
#define LEVELS_LEVEL_H

#include "../ecs/Entity.h"
#include "../core/GameState.h"
#include "../core/AssetManager.h"
#include "../core/Constants.h"
#include "../systems/ParticleSystem.h"
#include "../systems/ParallaxSystem.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <vector>

class Level {
public:
    virtual ~Level() = default;

    virtual void init(AssetManager& assets, GameState& state) = 0;
    virtual void update(float deltaTime, AssetManager& assets, GameState& state) = 0;
    virtual void render(sf::RenderWindow& window) = 0;
    virtual void spawnObstacles(GameState& state, AssetManager& assets) = 0;
    virtual Constants::LevelType getType() const = 0;
    /// Returns the effective theme (same as getType() for regular levels,
    /// but for InfiniteLevel it returns the current dynamic theme)
    virtual Constants::LevelType getEffectiveTheme() const { return getType(); }

protected:
    float spawnTimer_ = 0.0f;
    float spawnInterval_ = 2.0f;
    float backgroundY_ = 600.0f;

    void spawnGround(Entity* player, GameState& state, AssetManager& assets);
    void cleanupOffscreen(GameState& state);
};

#endif
