#ifndef CORE_GAMESTATE_H
#define CORE_GAMESTATE_H

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include "Constants.h"
#include "../ecs/Entity.h"

struct GameState {
    // Current game mode
    Constants::GameStateType currentState = Constants::GameStateType::MENU;
    Constants::LevelType currentLevel = Constants::LevelType::DESERT_DAY;
    Constants::DinoType selectedDino = Constants::DinoType::TREX;

    // Player
    int lives = Constants::INITIAL_LIVES;
    float score = 0.0f;
    float distance = 0.0f;
    float highScore = 0.0f;
    int coins = 0;

    // Speed
    float currentSpeed = Constants::BASE_SPEED;
    float speedTimer = 0.0f;

    // Power-ups active
    bool shieldActive = false;
    bool magnetActive = false;
    bool doublePointsActive = false;
    bool speedBoostActive = false;
    float shieldTimer = 0.0f;
    float magnetTimer = 0.0f;
    float doublePointsTimer = 0.0f;
    float speedBoostTimer = 0.0f;

    // Entities
    std::vector<std::unique_ptr<Entity>> entities;
    Entity* player = nullptr;

    // Unlocked dinosaurs
    bool trexUnlocked = true;
    bool raptorUnlocked = false;
    bool triceratopsUnlocked = false;

    // Level progress
    float levelProgress = 0.0f;
    float levelLength = 5000.0f;

    // Timer
    float gameTime = 0.0f;
    float levelTransitionTimer = 0.0f;

    void reset() {
        lives = Constants::INITIAL_LIVES;
        score = 0.0f;
        distance = 0.0f;
        coins = 0;
        currentSpeed = Constants::BASE_SPEED;
        speedTimer = 0.0f;
        shieldActive = false;
        magnetActive = false;
        doublePointsActive = false;
        speedBoostActive = false;
        shieldTimer = 0.0f;
        magnetTimer = 0.0f;
        doublePointsTimer = 0.0f;
        speedBoostTimer = 0.0f;
        gameTime = 0.0f;
        levelProgress = 0.0f;
        entities.clear();
        player = nullptr;
    }
};

#endif
