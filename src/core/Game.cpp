#include "../../include/core/Game.h"
#include "../../include/entities/PlayerFactory.h"
#include "../../include/entities/EnemyFactory.h"
#include "../../include/entities/ObstacleFactory.h"
#include "../../include/components/TransformComponent.h"
#include "../../include/components/SpriteComponent.h"
#include "../../include/components/PhysicsComponent.h"
#include "../../include/components/CollisionComponent.h"
#include "../../include/components/HealthComponent.h"
#include "../../include/components/PlayerComponent.h"
#include "../../include/components/EnemyComponent.h"
#include "../../include/components/CoinComponent.h"
#include "../../include/components/PowerUpComponent.h"
#include <SFML/Graphics/RenderTexture.hpp>
#include <algorithm>

Game::Game()
    : window_(sf::VideoMode(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT),
              Constants::GAME_TITLE,
              sf::Style::Default)
{
    window_.setFramerateLimit(Constants::FPS_LIMIT);
    init();
}

Game::~Game() = default;

void Game::init() {
    // Generate all procedural assets (textures and font)
    assetManager_.generateAllAssets();

    // Initialize procedural audio
    audioManager_.init();

    // Initialize systems
    initSystems();

    // Initialize UI
    if (assetManager_.hasFont("main")) {
        sf::Font& font = assetManager_.getFont("main");
        menu_.init(font, assetManager_);
        hud_.init(font);
        gameOverScreen_.init(font);
    }

    // Initialize collision rules
    initCollisionRules();
}

void Game::initSystems() {
    auto renderSys = std::make_unique<RenderSystem>(window_);
    renderSystem_ = renderSys.get();
    systems_.push_back(std::move(renderSys));

    auto physicsSys = std::make_unique<PhysicsSystem>();
    physicsSystem_ = physicsSys.get();
    systems_.push_back(std::move(physicsSys));

    auto collisionSys = std::make_unique<CollisionSystem>();
    collisionSystem_ = collisionSys.get();
    systems_.push_back(std::move(collisionSys));

    auto animSys = std::make_unique<AnimationSystem>();
    animationSystem_ = animSys.get();
    systems_.push_back(std::move(animSys));

    auto playerCtrlSys = std::make_unique<PlayerControlSystem>(inputManager_, gameState_, &audioManager_);
    playerControlSystem_ = playerCtrlSys.get();
    systems_.push_back(std::move(playerCtrlSys));

    auto enemyAISys = std::make_unique<EnemyAISystem>(gameState_.currentSpeed);
    enemyAISystem_ = enemyAISys.get();
    systems_.push_back(std::move(enemyAISys));

    auto coinSys = std::make_unique<CoinSystem>(gameState_);
    coinSystem_ = coinSys.get();
    systems_.push_back(std::move(coinSys));

    auto powerUpSys = std::make_unique<PowerUpSystem>(gameState_);
    powerUpSystem_ = powerUpSys.get();
    systems_.push_back(std::move(powerUpSys));
}

void Game::initCollisionRules() {
    collisionSystem_->onCollision("player", "enemy",
        [this](Entity* player, Entity* enemy) {
            auto* health = player->getComponent<HealthComponent>();
            auto* playerComp = player->getComponent<PlayerComponent>();
            if (health && !health->invulnerable) {
                if (playerComp && playerComp->hasShield) {
                    playerComp->hasShield = false;
                    gameState_.shieldActive = false;
                    gameState_.shieldTimer = 0.0f;
                    audioManager_.playSound("shield");
                    return;
                }
                health->takeDamage(1);
                gameState_.lives = health->health;
                audioManager_.playSound("damage");
                if (health->isDead()) {
                    checkGameOver();
                }
            }
        });

    collisionSystem_->onCollision("player", "coin",
        [this](Entity* player, Entity* coin) {
            auto* coinComp = coin->getComponent<CoinComponent>();
            if (coinComp && !coinComp->collected) {
                coinComp->collected = true;
                float points = coinComp->value;
                if (gameState_.doublePointsActive) points *= 2;
                gameState_.score += points;
                gameState_.coins++;
                audioManager_.playSound("coin");
            }
        });

    collisionSystem_->onCollision("player", "powerup",
        [this](Entity* player, Entity* powerup) {
            auto* pu = powerup->getComponent<PowerUpComponent>();
            if (pu && !pu->collected) {
                pu->collected = true;
                audioManager_.playSound("powerup");
                switch (pu->powerUpType) {
                    case Constants::PowerUpType::SHIELD:
                        gameState_.shieldActive = true;
                        gameState_.shieldTimer = pu->duration;
                        if (auto* pc = player->getComponent<PlayerComponent>())
                            pc->hasShield = true;
                        break;
                    case Constants::PowerUpType::MAGNET:
                        gameState_.magnetActive = true;
                        gameState_.magnetTimer = pu->duration;
                        if (auto* pc = player->getComponent<PlayerComponent>())
                            pc->hasMagnet = true;
                        break;
                    case Constants::PowerUpType::DOUBLE_POINTS:
                        gameState_.doublePointsActive = true;
                        gameState_.doublePointsTimer = pu->duration;
                        break;
                    case Constants::PowerUpType::SPEED_BOOST:
                        gameState_.speedBoostActive = true;
                        gameState_.speedBoostTimer = pu->duration;
                        break;
                    case Constants::PowerUpType::EXTRA_LIFE:
                        if (gameState_.lives < Constants::MAX_LIVES)
                            gameState_.lives++;
                        if (auto* h = player->getComponent<HealthComponent>())
                            h->heal(1);
                        break;
                }
            }
        });
}

void Game::run() {
    while (window_.isOpen()) {
        float deltaTime = clock_.restart().asSeconds();
        if (deltaTime > 0.05f) deltaTime = 0.05f; // Cap delta time

        handleEvents();
        update(deltaTime);
        render();
    }
}

void Game::handleEvents() {
    sf::Event event;
    while (window_.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window_.close();
        }
        if (event.type == sf::Event::KeyPressed) {
            if (gameState_.currentState == Constants::GameStateType::MENU) {
                menu_.handleInput(gameState_);
                if (menu_.shouldStart()) {
                    gameState_.selectedDino = menu_.getSelectedDino();
                    gameState_.currentLevel = menu_.getSelectedLevel();
                    startGame();
                }
            } else if (gameState_.currentState == Constants::GameStateType::GAME_OVER) {
                gameOverScreen_.handleInput(gameState_);
                if (gameOverScreen_.shouldRestart()) {
                    startGame();
                } else if (gameOverScreen_.shouldMenu()) {
                    gameState_.currentState = Constants::GameStateType::MENU;
                    menu_.reset();
                }
            } else if (gameState_.currentState == Constants::GameStateType::PLAYING) {
                if (event.key.code == sf::Keyboard::Escape) {
                    gameState_.currentState = Constants::GameStateType::PAUSED;
                }
            } else if (gameState_.currentState == Constants::GameStateType::PAUSED) {
                if (event.key.code == sf::Keyboard::Escape) {
                    gameState_.currentState = Constants::GameStateType::PLAYING;
                }
            }
            inputManager_.onKeyPressed(event.key.code);
        }
        if (event.type == sf::Event::KeyReleased) {
            inputManager_.onKeyReleased(event.key.code);
        }
    }
}

void Game::update(float deltaTime) {
    inputManager_.update();
    audioManager_.update();  // cleanup finished sounds

    switch (gameState_.currentState) {
        case Constants::GameStateType::MENU:
            menu_.update(deltaTime);
            break;

        case Constants::GameStateType::PLAYING:
        {
            // Update speed
            updateSpeed(deltaTime);
            updatePowerUps(deltaTime);

            // Update distance
            gameState_.distance += gameState_.currentSpeed * deltaTime * 0.1f;

            // Update level
            levelManager_.update(deltaTime, assetManager_, gameState_);

            // Get active entities
            std::vector<Entity*> activeEntities;
            for (auto& entity : gameState_.entities) {
                if (entity && entity->isActive()) {
                    activeEntities.push_back(entity.get());
                }
            }

            // Update all systems
            for (auto& system : systems_) {
                system->update(deltaTime, activeEntities);
            }

            // Spawn new obstacles periodically
            static float obstacleTimer = 0.0f;
            obstacleTimer += deltaTime;
            if (obstacleTimer > std::max(0.8f, 2.0f - gameState_.distance / 500.0f)) {
                obstacleTimer = 0.0f;
                ObstacleFactory::spawnRandomObstacle(assetManager_, gameState_, gameState_.currentLevel);
            }

            // Spawn coins occasionally
            static float coinTimer = 0.0f;
            coinTimer += deltaTime;
            if (coinTimer > 3.0f) {
                coinTimer = 0.0f;
                float coinY = Constants::GROUND_Y - 50 - static_cast<float>(std::rand() % 100);
                ObstacleFactory::spawnCoinCluster(assetManager_, gameState_,
                    Constants::WINDOW_WIDTH + 50, coinY, 5);
            }

            // Spawn powerups rarely
            static float powerUpTimer = 0.0f;
            powerUpTimer += deltaTime;
            if (powerUpTimer > 8.0f + static_cast<float>(std::rand() % 10)) {
                powerUpTimer = 0.0f;
                ObstacleFactory::spawnRandomPowerUp(assetManager_, gameState_);
            }

            // Cleanup dead entities (process after iteration)
            gameState_.entities.erase(
                std::remove_if(gameState_.entities.begin(), gameState_.entities.end(),
                    [](const std::unique_ptr<Entity>& e) {
                        return !e || !e->isActive();
                    }),
                gameState_.entities.end());

            // Update HUD
            hud_.update(gameState_);

            // Check game over condition
            if (gameState_.lives <= 0) {
                checkGameOver();
            }
            break;
        }

        case Constants::GameStateType::PAUSED:
            // Nothing updates while paused
            break;

        case Constants::GameStateType::GAME_OVER:
            gameOverScreen_.update(deltaTime, gameState_);
            break;

        default:
            break;
    }
}

void Game::render() {
    window_.clear();

    switch (gameState_.currentState) {
        case Constants::GameStateType::MENU:
            menu_.render(window_);
            break;

        case Constants::GameStateType::PLAYING:
        case Constants::GameStateType::PAUSED:
        {
            // Render level background
            levelManager_.render(window_);

            // Render all entities via render system
            {
                std::vector<Entity*> activeEntities;
                for (auto& entity : gameState_.entities) {
                    if (entity && entity->isActive()) {
                        activeEntities.push_back(entity.get());
                    }
                }
                renderSystem_->update(0.0f, activeEntities);
            }

            // Render HUD
            hud_.render(window_);

            // Render pause overlay
            if (gameState_.currentState == Constants::GameStateType::PAUSED) {
                sf::RectangleShape overlay(sf::Vector2f(
                    Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT));
                overlay.setFillColor(sf::Color(0, 0, 0, 128));
                window_.draw(overlay);

                if (assetManager_.hasFont("main")) {
                    sf::Text pauseText;
                    pauseText.setFont(assetManager_.getFont("main"));
                    pauseText.setCharacterSize(48);
                    pauseText.setFillColor(sf::Color::White);
                    pauseText.setString("PAUSED");
                    pauseText.setPosition(
                        Constants::WINDOW_WIDTH / 2 - pauseText.getGlobalBounds().width / 2,
                        Constants::WINDOW_HEIGHT / 2 - 50);
                    window_.draw(pauseText);

                    sf::Text resumeText;
                    resumeText.setFont(assetManager_.getFont("main"));
                    resumeText.setCharacterSize(24);
                    resumeText.setFillColor(sf::Color(200, 200, 200));
                    resumeText.setString("Press ESC to resume");
                    resumeText.setPosition(
                        Constants::WINDOW_WIDTH / 2 - resumeText.getGlobalBounds().width / 2,
                        Constants::WINDOW_HEIGHT / 2 + 20);
                    window_.draw(resumeText);
                }
            }
            break;
        }

        case Constants::GameStateType::GAME_OVER:
        {
            // Render final game state behind game over screen
            std::vector<Entity*> activeEntities;
            for (auto& entity : gameState_.entities) {
                if (entity && entity->isActive()) {
                    activeEntities.push_back(entity.get());
                }
            }
            renderSystem_->update(0.0f, activeEntities);
            gameOverScreen_.render(window_);
            break;
        }

        default:
            break;
    }

    window_.display();
}

void Game::startGame() {
    // Reset game state
    gameState_.score = 0.0f;
    gameState_.distance = 0.0f;
    gameState_.currentSpeed = Constants::BASE_SPEED;
    gameState_.speedTimer = 0.0f;
    gameState_.coins = 0;
    gameState_.lives = Constants::INITIAL_LIVES;
    gameState_.shieldActive = false;
    gameState_.magnetActive = false;
    gameState_.doublePointsActive = false;
    gameState_.speedBoostActive = false;
    gameState_.shieldTimer = 0.0f;
    gameState_.magnetTimer = 0.0f;
    gameState_.doublePointsTimer = 0.0f;
    gameState_.speedBoostTimer = 0.0f;
    gameState_.currentState = Constants::GameStateType::PLAYING;

    // Clear old entities
    gameState_.entities.clear();

    // Create player
    auto player = PlayerFactory::createPlayer(
        gameState_.selectedDino, assetManager_,
        100.0f, Constants::GROUND_Y);

    // Store pointer to player for systems
    gameState_.player = player.get();
    gameState_.entities.push_back(std::move(player));

    // Initialize level
    levelManager_.init(gameState_.currentLevel, assetManager_, gameState_);

    // Start background music
    audioManager_.startMusic();
}

void Game::checkGameOver() {
    gameState_.currentState = Constants::GameStateType::GAME_OVER;
    if (gameState_.score > gameState_.highScore) {
        gameState_.highScore = gameState_.score;
    }
    gameOverScreen_.setScore(gameState_.score, gameState_.highScore);
    audioManager_.stopMusic();
    audioManager_.playSound("gameover");
}

void Game::updateSpeed(float deltaTime) {
    gameState_.speedTimer += deltaTime;
    if (gameState_.speedTimer >= Constants::SPEED_INTERVAL) {
        gameState_.speedTimer = 0.0f;
        gameState_.currentSpeed = std::min(
            gameState_.currentSpeed + Constants::SPEED_INCREMENT,
            Constants::MAX_SPEED);
    }
}

void Game::updatePowerUps(float deltaTime) {
    // Shield timer
    if (gameState_.shieldActive) {
        gameState_.shieldTimer -= deltaTime;
        if (gameState_.shieldTimer <= 0.0f) {
            gameState_.shieldActive = false;
            if (gameState_.player) {
                auto* pc = gameState_.player->getComponent<PlayerComponent>();
                if (pc) pc->hasShield = false;
            }
        }
    }

    // Magnet timer
    if (gameState_.magnetActive) {
        gameState_.magnetTimer -= deltaTime;
        if (gameState_.magnetTimer <= 0.0f) {
            gameState_.magnetActive = false;
            if (gameState_.player) {
                auto* pc = gameState_.player->getComponent<PlayerComponent>();
                if (pc) pc->hasMagnet = false;
            }
        }
    }

    // Double points timer
    if (gameState_.doublePointsActive) {
        gameState_.doublePointsTimer -= deltaTime;
        if (gameState_.doublePointsTimer <= 0.0f) {
            gameState_.doublePointsActive = false;
        }
    }

    // Speed boost timer
    if (gameState_.speedBoostActive) {
        gameState_.speedBoostTimer -= deltaTime;
        if (gameState_.speedBoostTimer <= 0.0f) {
            gameState_.speedBoostActive = false;
        }
    }
}
