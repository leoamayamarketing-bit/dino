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

    // Configure particle physics for underwater feel:
    // Bubbles: negative gravity (buoyancy) so they rise upward
    bubbles_.setGravity(-35.0f);
    // Light rays: near-zero gravity so they drift slowly and fade gracefully
    lightRays_.setGravity(5.0f);
    // Sea dust: very gentle drift, sparkles suspended in water
    seaDust_.setGravity(-2.0f);

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

    // Generate kraken tentacles at fixed positions (right side of screen)
    krakenTentacles_.clear();
    krakenCollisionIds_.clear();
    for (int i = 0; i < 4; i++) {
        KrakenTentacle t;
        t.x = Constants::WINDOW_WIDTH + 100.0f + i * 200.0f + (std::rand() % 80);
        t.height = 100.0f + (std::rand() % 100);
        t.phase = (std::rand() % 100) * 0.01f * 6.2832f;
        t.baseWidth = 20.0f + (std::rand() % 15);
        t.timer = 0.0f;
        t.rising = true;
        t.currentHeight = 0.0f;
        t.holdTimer = 0.0f;
        krakenTentacles_.push_back(t);

        // Create invisible collision entity for this tentacle
        auto tentacleEntity = std::make_unique<Entity>(2000 + i);
        tentacleEntity->addComponent<TransformComponent>(-1000.0f, -1000.0f);
        auto& coll = tentacleEntity->addComponent<CollisionComponent>(
            sf::FloatRect(0, 0, 1, 1), "enemy");
        coll.localOffset = sf::Vector2f(-0.5f, 0);
        coll.isTrigger = false;
        coll.isStatic = true;
        coll.damageAmount = 1;
        krakenCollisionIds_.push_back(state.entities.size());
        state.entities.push_back(std::move(tentacleEntity));
    }
    krakenTimer_ = 0.0f;
    krakenInterval_ = 10.0f;
    krakenActive_ = false;
    jellyfishPhase_ = 0.0f;

    // Generate caustic light patches on the seabed
    caustics_.clear();
    for (int i = 0; i < 18; i++) {
        Caustic c;
        c.x = static_cast<float>(std::rand() % Constants::WINDOW_WIDTH);
        c.size = 20.0f + (std::rand() % 35);
        c.phase = (std::rand() % 100) * 0.01f * 6.2832f;
        c.speed = 0.4f + (std::rand() % 5) * 0.1f;
        c.alpha = 15.0f + (std::rand() % 25);
        caustics_.push_back(c);
    }
    causticTimer_ = 0.0f;

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
        // Track recent light ray X positions for sea dust clustering
        lightRayPositions_[lightRayIndex_++ % 4] = lx;
        lightRays_.emit(
            sf::Vector2f(lx, 0.0f),
            1, sf::Color(200, 230, 255, 20), 15.0f, 1.2f, 6.0f);
    }

    // Sea dust sparkles (suspended particles glittering in light rays)
    dustTimer_ += deltaTime;
    if (dustTimer_ > 0.12f) {
        dustTimer_ = 0.0f;
        // Cluster dust near recent light ray X positions so sparkles appear within shafts
        float dx = lightRayPositions_[std::rand() % 4];
        if (dx == 0.0f) dx = static_cast<float>(std::rand() % Constants::WINDOW_WIDTH);
        dx += (std::rand() % 60 - 30);  // slight spread around the shaft
        float dy = 50.0f + static_cast<float>(std::rand() % static_cast<int>(Constants::GROUND_Y - 160));  // mid-water
        // Random warm-gold to cool-white sparkle
        int hue = std::rand() % 3;
        sf::Color dustColor;
        if (hue == 0)      dustColor = sf::Color(255, 255, 230, 120);  // warm white
        else if (hue == 1) dustColor = sf::Color(220, 240, 255, 100);  // cool white
        else               dustColor = sf::Color(255, 250, 200, 80);   // gold
        float dustSize = 1.5f + (std::rand() % 3) * 0.8f;
        seaDust_.emit(
            sf::Vector2f(dx, dy),
            1, dustColor, 8.0f, 1.8f + (std::rand() % 5) * 0.2f, dustSize);
    }

    // Surface wave animation
    waveTimer_ += deltaTime * 1.5f;

    // Obstacle spawning
    spawnTimer_ += deltaTime;
    if (spawnTimer_ >= spawnInterval_) {
        spawnTimer_ = 0.0f;
        spawnObstacles(state, assets);
        spawnInterval_ = std::max(minSpawnInterval_, spawnInterval_ - (state.hardMode ? 0.08f : 0.04f));
    }

    // Jellyfish bobbing — identified by texture name, not EnemyType
    // Animate caustic light patterns on seabed
    causticTimer_ += deltaTime * 1.2f;
    float scrollSpeed = state.currentSpeed * deltaTime * 0.15f;
    for (auto& c : caustics_) {
        // Slow horizontal drift opposite to game scroll, then wrap
        c.x -= scrollSpeed;
        if (c.x < -c.size) c.x += Constants::WINDOW_WIDTH + c.size * 2;
        // Undulating shimmer using sin
        float wave = std::sin(causticTimer_ * c.speed + c.phase);
        // Slight horizontal jitter
        c.x += wave * 0.3f * deltaTime * 30.0f;
    }

    jellyfishPhase_ += deltaTime * 2.0f;
    for (size_t i = 0; i < state.entities.size(); i++) {
        auto& e = state.entities[i];
        if (!e || !e->isActive()) continue;
        auto* sprite = e->getComponent<SpriteComponent>();
        auto* trans = e->getComponent<TransformComponent>();
        if (sprite && trans && sprite->textureName == "jellyfish") {
            // Sine wave vertical bob
            float bob = std::sin(jellyfishPhase_ * 0.7f + i * 0.8f) * 6.0f;
            trans->position.y += bob * deltaTime * 2.0f;
        }
    }

    cleanupOffscreen(state);
    bubbles_.updateParticles(deltaTime);
    lightRays_.updateParticles(deltaTime);
    seaDust_.updateParticles(deltaTime);

    // Kraken tentacle animation
    krakenTimer_ += deltaTime;
    if (krakenTimer_ >= krakenInterval_) {
        krakenTimer_ = 0.0f;
        krakenActive_ = true;
        // Reset all tentacles
        for (auto& t : krakenTentacles_) {
            t.rising = true;
            t.currentHeight = 0.0f;
            t.holdTimer = 0.0f;
        }
    }

    if (krakenActive_) {
        bool allDone = true;
        for (size_t i = 0; i < krakenTentacles_.size(); i++) {
            auto& t = krakenTentacles_[i];
            if (t.rising) {
                t.currentHeight += deltaTime * 120.0f;  // rise speed
                if (t.currentHeight >= t.height) {
                    t.currentHeight = t.height;
                    t.rising = false;
                    t.holdTimer = 0.0f;
                }
            } else if (t.holdTimer < 1.5f) {
                t.holdTimer += deltaTime;  // hold at full height
            } else {
                t.currentHeight -= deltaTime * 80.0f;  // descend
                if (t.currentHeight <= 0.0f) {
                    t.currentHeight = 0.0f;
                }
            }

            // Update collision entity position for this tentacle
            if (i < krakenCollisionIds_.size()) {
                size_t idx = krakenCollisionIds_[i];
                if (idx < state.entities.size()) {
                    auto& tentacleEntity = state.entities[idx];
                    auto* trans = tentacleEntity->getComponent<TransformComponent>();
                    auto* coll = tentacleEntity->getComponent<CollisionComponent>();
                    if (trans && coll) {
                        if (t.currentHeight > 30.0f) {
                            float sway = std::sin(krakenTimer_ * 1.5f + t.phase) * 8.0f;
                            float colWidth = t.baseWidth * 0.6f;
                            float colHeight = std::min(t.currentHeight * 0.3f, 60.0f);
                            trans->position.x = t.x + sway;
                            trans->position.y = Constants::GROUND_Y - t.currentHeight + 15.0f;
                            coll->bounds = sf::FloatRect(0, 0, colWidth, colHeight);
                            coll->localOffset = sf::Vector2f(-colWidth * 0.5f, 0);
                            tentacleEntity->setActive(true);
                        } else {
                            trans->position.x = -1000.0f;
                            trans->position.y = -1000.0f;
                            tentacleEntity->setActive(true);  // keep active to avoid cleanup
                        }
                    }
                }
            }

            if (t.currentHeight > 0.0f) {
                allDone = false;
            }
        }
        if (allDone) {
            krakenActive_ = false;
        }
    }
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

    /// --- Surface waves (undulating horizontal lines at water surface) ---
    {
        int waveCount = 5;
        float baseY = 25.0f;
        for (int w = 0; w < waveCount; w++) {
            float wavePhase = waveTimer_ + w * 1.8f;
            float yOffset = std::sin(wavePhase) * 5.0f;
            float yPos = baseY + w * 7.0f + yOffset;
            float alphaBase = 25 + w * 15;
            // Each wave line is drawn as a series of short segments with varying alpha
            for (int seg = 0; seg < 20; seg++) {
                float segX = seg * (Constants::WINDOW_WIDTH / 20.0f);
                float segPhase = waveTimer_ * 0.6f + seg * 0.5f + w * 0.9f;
                float segY = yPos + std::sin(segPhase) * 4.0f;
                float segAlpha = alphaBase + 20.0f * (0.5f + 0.5f * std::sin(segPhase * 1.3f));
                sf::Uint8 a = static_cast<sf::Uint8>(std::clamp(segAlpha, 0.0f, 100.0f));
                sf::RectangleShape waveSeg(sf::Vector2f(Constants::WINDOW_WIDTH / 20.0f + 4, 1.5f));
                waveSeg.setFillColor(sf::Color(220, 240, 255, a));
                waveSeg.setPosition(segX, segY);
                window.draw(waveSeg);
            }
        }
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

    /// --- Caustic light patterns on the seabed ---
    {
        float baseY = Constants::GROUND_Y;
        for (const auto& c : caustics_) {
            float shimmer = 0.5f + 0.5f * std::sin(causticTimer_ * c.speed + c.phase);
            float currentAlpha = c.alpha * (0.6f + 0.4f * shimmer);
            sf::Uint8 a = static_cast<sf::Uint8>(std::clamp(currentAlpha, 0.0f, 80.0f));

            // Elongated ellipse-like shape for the caustic patch
            sf::RectangleShape patch(sf::Vector2f(c.size * 2.2f, c.size * 0.6f));
            patch.setOrigin(c.size * 1.1f, c.size * 0.3f);  // center origin for natural rotation
            // Soft light yellow-white color
            patch.setFillColor(sf::Color(180, 210, 230, a));
            patch.setPosition(c.x, baseY - c.size * 0.3f);

            // Slight rotation for natural look
            float angle = std::sin(causticTimer_ * c.speed * 0.7f + c.phase * 1.3f) * 20.0f;
            patch.setRotation(angle);

            window.draw(patch);

            // Secondary smaller highlight
            sf::Uint8 a2 = static_cast<sf::Uint8>(std::clamp(currentAlpha * 0.5f, 0.0f, 40.0f));
            sf::RectangleShape highlight(sf::Vector2f(c.size * 0.7f, c.size * 0.3f));
            highlight.setOrigin(c.size * 0.35f, c.size * 0.15f);  // center origin
            highlight.setFillColor(sf::Color(210, 235, 255, a2));
            highlight.setPosition(c.x, baseY - c.size * 0.1f);
            highlight.setRotation(angle * 0.5f);
            window.draw(highlight);
        }
    }

    /// --- Sea dust sparkles (glittering particles suspended in light) ---
    seaDust_.render(window);

    /// --- Bubbles (on top of everything) ---
    bubbles_.render(window);

    /// --- Kraken tentacles (drawn on top) ---
    if (krakenActive_) {
        for (auto& t : krakenTentacles_) {
            if (t.currentHeight <= 0.0f) continue;
            float sway = std::sin(krakenTimer_ * 1.5f + t.phase) * 8.0f;
            // Tentacle body (tapered rectangle)
            float tipWidth = t.baseWidth * 0.3f;
            int segments = 6;
            float segH = t.currentHeight / segments;
            for (int j = 0; j < segments; j++) {
                float segT = static_cast<float>(j) / segments;
                float w = t.baseWidth - (t.baseWidth - tipWidth) * segT;
                float segSway = sway * (1.0f - segT * 0.5f);
                sf::RectangleShape segment(sf::Vector2f(w, segH + 2));
                // Dark green/mottled color with gradient
                sf::Color segColor(
                    static_cast<sf::Uint8>(25 + 30 * segT),
                    static_cast<sf::Uint8>(50 + 20 * segT),
                    static_cast<sf::Uint8>(40 + 15 * segT));
                segment.setFillColor(segColor);
                float sx = t.x + segSway - w * 0.5f;
                float sy = Constants::GROUND_Y - t.currentHeight + j * segH;
                segment.setPosition(sx, sy);
                window.draw(segment);
            }
            // Tentacle tip (rounded)
            sf::CircleShape tip(tipWidth * 0.8f);
            tip.setFillColor(sf::Color(40, 60, 50));
            float tipSway = sway * 0.8f;
            tip.setPosition(t.x + tipSway - tipWidth * 0.8f,
                           Constants::GROUND_Y - t.currentHeight - tipWidth * 0.5f);
            window.draw(tip);

            // Suction cup highlights on tentacle
            for (int s = 0; s < 3; s++) {
                float st = 0.25f + s * 0.2f;
                float sy = Constants::GROUND_Y - t.currentHeight * (1.0f - st);
                float sx = t.x + sway * (1.0f - st * 0.5f);
                sf::CircleShape sucker(2.5f);
                sucker.setFillColor(sf::Color(60, 80, 65));
                sucker.setPosition(sx - 2.5f, sy - 2.5f);
                window.draw(sucker);
            }
        }
    }
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
    } else if (roll < 45) {
        // Jellyfish (floating enemy, mid-water)
        static uint32_t jellyId = 600;
        auto jelly = std::make_unique<Entity>(jellyId++);
        float jy = Constants::GROUND_Y - 120 - (std::rand() % 80);
        jelly->addComponent<TransformComponent>(startX, jy);
        auto& jSprite = jelly->addComponent<SpriteComponent>("jellyfish");
        if (assets.hasTexture("jellyfish")) {
            jSprite.setTexture(assets.getTexture("jellyfish"));
        } else if (assets.hasTexture("fish")) {
            jSprite.setTexture(assets.getTexture("fish"));
        }
        jSprite.zOrder = 5;
        jSprite.sprite.setOrigin(20.0f, 25.0f);
        float jscale = 0.8f + (std::rand() % 5) * 0.1f;
        jSprite.sprite.setScale(jscale, jscale);
        // Collision: dome shape
        auto& jColl = jelly->addComponent<CollisionComponent>(
            sf::FloatRect(0, 0, 30 * jscale, 25 * jscale), "enemy");
        jColl.localOffset = sf::Vector2f(-15.0f * jscale, -12.0f * jscale);
        jColl.isTrigger = false;
        jColl.isStatic = true;
        jColl.damageAmount = 1;
        // Use SMALL_CACTUS type to avoid EnemyAISystem altitude bobbing (handled manually in update)
        jelly->addComponent<EnemyComponent>(Constants::EnemyType::SMALL_CACTUS);
        state.entities.push_back(std::move(jelly));
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
