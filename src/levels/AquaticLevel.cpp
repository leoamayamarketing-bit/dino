#include "../../include/levels/AquaticLevel.h"
#include "../../include/core/Constants.h"
#include "../../include/entities/ObstacleFactory.h"
#include "../../include/entities/EnemyFactory.h"
#include "../../include/components/TransformComponent.h"
#include "../../include/components/SpriteComponent.h"
#include "../../include/components/CollisionComponent.h"
#include "../../include/components/EnemyComponent.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>

void AquaticLevel::init(AssetManager& assets, GameState& state) {
    spawnInterval_ = 2.8f;
    spawnTimer_ = 0.0f;
    minSpawnInterval_ = 1.6f;

    // Add clouds to parallax (subtle overhead) — no ground parallax,
    // we render the seabed manually in grayscale-compatible blue tones
    if (assets.hasTexture("cloud")) {
        parallax_.addLayer(assets.getTexture("cloud"), 0.15f, 20.0f);
    }
    parallax_.setScrollDirection(1.0f);

    // Generate seaweed blades — varied heights and colors
    seaweeds_.clear();
    for (int i = 0; i < 14; i++) {
        Seaweed s;
        s.x = i * 95.0f + (std::rand() % 30);
        s.height = 80.0f + (std::rand() % 120);
        s.phase = (std::rand() % 100) * 0.01f * 6.2832f;
        // greenish tones for seaweed
        int g = 100 + (std::rand() % 80);
        s.color = sf::Color(30, g, 50);
        seaweeds_.push_back(s);
    }

    // Generate coral clusters
    corals_.clear();
    for (int i = 0; i < 8; i++) {
        Coral c;
        c.x = 60.0f + i * 160.0f + (std::rand() % 50);
        c.radius = 12.0f + (std::rand() % 15);
        int r = 180 + (std::rand() % 60);  // coral pink/red tones
        c.color = sf::Color(r, 60 + (std::rand() % 40), 80 + (std::rand() % 40));
        corals_.push_back(c);
    }

    spawnObstacles(state, assets);
}

void AquaticLevel::update(float deltaTime, AssetManager& assets, GameState& state) {
    parallax_.setScrollDirection(state.currentSpeed);
    std::vector<Entity*> emptyVec;
    parallax_.update(deltaTime, emptyVec);

    sandOffset_ = std::fmod(sandOffset_ + state.currentSpeed * deltaTime * 0.3f, 15.0f);

    // Bubbles rising from seabed
    bubbleTimer_ += deltaTime;
    if (bubbleTimer_ > 0.15f) {
        bubbleTimer_ = 0.0f;
        float bx = static_cast<float>(std::rand() % Constants::WINDOW_WIDTH);
        bubbles_.emit(
            sf::Vector2f(bx, Constants::GROUND_Y - 5),
            1, sf::Color(180, 220, 255, 100), 30.0f, 2.5f, 4.0f);
    }

    // Light rays shimmer
    lightRayTimer_ += deltaTime;
    if (lightRayTimer_ > 0.08f) {
        lightRayTimer_ = 0.0f;
        float lx = static_cast<float>(std::rand() % Constants::WINDOW_WIDTH);
        lightRays_.emit(
            sf::Vector2f(lx, 0.0f),
            1, sf::Color(200, 230, 255, 20), 15.0f, 1.2f, 6.0f);
    }

    // Obstacle spawning
    spawnTimer_ += deltaTime;
    if (spawnTimer_ >= spawnInterval_) {
        spawnTimer_ = 0.0f;
        spawnObstacles(state, assets);
        spawnInterval_ = std::max(minSpawnInterval_, spawnInterval_ - (state.hardMode ? 0.08f : 0.04f));
    }

    cleanupOffscreen(state);
    bubbles_.updateParticles(deltaTime);
    lightRays_.updateParticles(deltaTime);
}

void AquaticLevel::render(sf::RenderWindow& window) {
    /// --- Water gradient background ---
    // Draw vertical stripes to simulate depth gradient
    for (int i = 0; i < Constants::WINDOW_HEIGHT; i += 4) {
        float t = static_cast<float>(i) / Constants::WINDOW_HEIGHT;
        sf::Color c(
            static_cast<sf::Uint8>(shallowColor_.r + (deepColor_.r - shallowColor_.r) * t),
            static_cast<sf::Uint8>(shallowColor_.g + (deepColor_.g - shallowColor_.g) * t),
            static_cast<sf::Uint8>(shallowColor_.b + (deepColor_.b - shallowColor_.b) * t)
        );
        sf::RectangleShape stripe(sf::Vector2f(Constants::WINDOW_WIDTH, 4));
        stripe.setFillColor(c);
        stripe.setPosition(0, static_cast<float>(i));
        window.draw(stripe);
    }

    /// --- Light rays (sun shafts through water) ---
    lightRays_.render(window);

    /// --- Seaweed blades (waving animation) ---
    for (const auto& s : seaweeds_) {
        // Wavy blade using a series of rectangles
        int segments = 8;
        float segH = s.height / segments;
        float waveTime = sandOffset_ * 0.08f;
        for (int j = 0; j < segments; j++) {
            float segT = static_cast<float>(j) / segments;
            float sway = std::sin(waveTime + s.phase + segT * 2.0f) * 6.0f * (1.0f - segT * 0.3f);
            float segAlpha = 1.0f - segT * 0.2f;
            sf::Color segColor = s.color;
            segColor.a = static_cast<sf::Uint8>(200 + 55 * segAlpha);

            sf::RectangleShape blade(sf::Vector2f(6.0f, segH + 2));
            blade.setFillColor(segColor);
            float bx = s.x + sway;
            float by = Constants::GROUND_Y - s.height + j * segH;
            blade.setPosition(bx, by);
            window.draw(blade);
        }

        // Seaweed tip (small oval)
        sf::CircleShape tip(5.0f);
        tip.setFillColor(sf::Color(s.color.r + 20, s.color.g + 10, s.color.b, 220));
        float tipSway = std::sin(sandOffset_ * 0.08f + s.phase + 1.0f) * 7.0f;
        tip.setPosition(s.x + tipSway - 5.0f, Constants::GROUND_Y - s.height - 5.0f);
        window.draw(tip);
    }

    /// --- Coral clusters on the seabed ---
    for (const auto& c : corals_) {
        // Main coral body
        sf::CircleShape coral(c.radius);
        coral.setFillColor(c.color);
        coral.setPosition(c.x - c.radius, Constants::GROUND_Y - c.radius * 0.6f);
        window.draw(coral);

        // Coral branches (smaller circles on top)
        sf::CircleShape branch1(c.radius * 0.5f);
        branch1.setFillColor(sf::Color(
            std::min(255, c.color.r + 30),
            std::max(0, c.color.g - 10),
            std::max(0, c.color.b - 10)));
        branch1.setPosition(c.x - c.radius * 0.3f, Constants::GROUND_Y - c.radius * 1.2f);
        window.draw(branch1);

        sf::CircleShape branch2(c.radius * 0.4f);
        branch2.setFillColor(sf::Color(
            std::max(0, c.color.r - 20),
            c.color.g,
            c.color.b));
        branch2.setPosition(c.x + c.radius * 0.2f, Constants::GROUND_Y - c.radius * 1.0f);
        window.draw(branch2);
    }

    /// --- Seabed (sand/dark ground) ---
    sf::RectangleShape groundFill(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT - Constants::GROUND_Y));
    groundFill.setFillColor(groundColor_);
    groundFill.setPosition(0, Constants::GROUND_Y);
    window.draw(groundFill);

    // Sandy surface line
    sf::RectangleShape sandLine(sf::Vector2f(Constants::WINDOW_WIDTH, 3));
    sandLine.setFillColor(sf::Color(80, 90, 70));
    sandLine.setPosition(0, Constants::GROUND_Y);
    window.draw(sandLine);

    // Sandy texture dots
    sf::CircleShape sand(1.2f);
    for (int i = 0; i < 25; i++) {
        float sx = std::fmod(i * 52.0f + sandOffset_ - 10.0f, static_cast<float>(Constants::WINDOW_WIDTH));
        if (sx < 0.0f) sx += Constants::WINDOW_WIDTH;
        float sy = Constants::GROUND_Y + 5.0f + (i * 13) % 50;
        int sg = 65 + (i * 5) % 20;
        sand.setFillColor(sf::Color(sg, sg + 10, sg - 5));
        sand.setPosition(sx, sy);
        window.draw(sand);
    }

    // Render parallax (clouds)
    parallax_.render(window);

    /// --- Bubbles (on top of everything) ---
    bubbles_.render(window);
}

void AquaticLevel::spawnObstacles(GameState& state, AssetManager& assets) {
    float startX = Constants::WINDOW_WIDTH + 100.0f + static_cast<float>(std::rand() % 200);
    int roll = std::rand() % 100;

    // Helper to create a fish enemy entity
    auto createFish = [&](float x, float y, float scale, bool movingRight) -> std::unique_ptr<Entity> {
        static uint32_t fishId = 500;
        auto fish = std::make_unique<Entity>(fishId++);

        fish->addComponent<TransformComponent>(x, y);

        // Use the fish texture (procedurally generated in AssetManager)
        auto& sprite = fish->addComponent<SpriteComponent>("fish");
        if (assets.hasTexture("fish")) {
            sprite.setTexture(assets.getTexture("fish"));
        } else {
            // Fallback: use rock texture
            sprite.setTexture(assets.getTexture("rock"));
        }
        sprite.zOrder = 5;
        // Center the sprite origin so collision box aligns with visual
        sprite.sprite.setOrigin(25.0f, 12.0f);
        sprite.sprite.setScale(scale, scale);

        // Flip fish horizontally if moving right (or left by default)
        if (!movingRight) {
            sprite.sprite.setScale(-scale, scale);
        }

        // Collision box matching fish size
        auto& coll = fish->addComponent<CollisionComponent>(
            sf::FloatRect(0, 0, 50 * scale, 20 * scale), "enemy");
        coll.localOffset = sf::Vector2f(-25.0f * scale, -10.0f * scale);
        coll.isTrigger = false;
        coll.isStatic = true;
        coll.damageAmount = 1;

        fish->addComponent<EnemyComponent>(Constants::EnemyType::GROUND_ENEMY);

        return fish;
    };

    if (roll < 30) {
        // Ground fish (swimming near the seabed)
        auto fish = createFish(startX, Constants::GROUND_Y - 30, 1.2f, false);
        state.entities.push_back(std::move(fish));
    } else if (roll < 55) {
        // Mid-water fish
        auto fish = createFish(startX, Constants::GROUND_Y - 100 - (std::rand() % 60), 1.0f, false);
        state.entities.push_back(std::move(fish));
    } else if (roll < 70) {
        // School of two small fish
        auto fish1 = createFish(startX, Constants::GROUND_Y - 80, 0.8f, false);
        state.entities.push_back(std::move(fish1));
        auto fish2 = createFish(startX + 70, Constants::GROUND_Y - 90, 0.7f, false);
        state.entities.push_back(std::move(fish2));
    } else if (roll < 85) {
        // Spiky pufferfish / rock obstacle on seabed
        auto enemy = EnemyFactory::createEnemy(Constants::EnemyType::SMALL_CACTUS, assets, startX);
        state.entities.push_back(std::move(enemy));
    } else if (roll < 95) {
        // Coin cluster
        for (int i = 0; i < 3; i++) {
            auto coin = ObstacleFactory::createCoin(assets, startX + i * 50,
                Constants::GROUND_Y - 120 - (std::rand() % 80));
            state.entities.push_back(std::move(coin));
        }
    } else {
        // Powerup chance
        auto pu = ObstacleFactory::createPowerUp(assets,
            static_cast<Constants::PowerUpType>(std::rand() % 5),
            startX + 250, Constants::GROUND_Y - 100);
        state.entities.push_back(std::move(pu));
    }

    // Extra powerup chance
    if (std::rand() % 100 < 6) {
        auto pu = ObstacleFactory::createPowerUp(assets,
            static_cast<Constants::PowerUpType>(std::rand() % 5),
            startX + 250, Constants::GROUND_Y - 100);
        state.entities.push_back(std::move(pu));
    }
}
