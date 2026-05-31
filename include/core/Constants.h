#ifndef CORE_CONSTANTS_H
#define CORE_CONSTANTS_H

#include <cstdint>

namespace Constants {
    // Window
    constexpr int WINDOW_WIDTH = 1280;
    constexpr int WINDOW_HEIGHT = 720;
    constexpr const char* GAME_TITLE = "DinoRunner: Primal Rush";
    constexpr int FPS_LIMIT = 60;

    // Physics
    constexpr float GRAVITY = 1800.0f;
    constexpr float GROUND_Y = 480.0f;
    constexpr float BASE_SPEED = 400.0f;
    constexpr float MAX_SPEED = 900.0f;
    constexpr float SPEED_INCREMENT = 2.0f;
    constexpr float SPEED_INTERVAL = 5.0f;

    // Player
    constexpr float JUMP_VELOCITY = -620.0f;
    constexpr float DOUBLE_JUMP_VELOCITY = -520.0f;
    constexpr float DASH_SPEED_MULTIPLIER = 2.0f;
    constexpr float DASH_DURATION = 0.3f;
    constexpr float DASH_COOLDOWN = 1.0f;
    constexpr float CROUCH_HEIGHT = 0.5f;

    // Jump feel improvements
    constexpr float COYOTE_TIME = 0.08f;           // ms after leaving ground you can still jump
    constexpr float JUMP_BUFFER_TIME = 0.1f;       // ms before landing you can buffer a jump
    constexpr float JUMP_CUT_MULTIPLIER = 0.4f;    // velocity multiplier when releasing jump early
    constexpr float GRAVITY_UP_FACTOR = 0.65f;     // gravity multiplier while ascending (less floaty, snappier)
    constexpr float GRAVITY_DOWN_FACTOR = 1.2f;    // gravity multiplier while falling (snappier)
    constexpr float LANDING_SQUASH_DURATION = 0.12f;
    constexpr float LANDING_SQUASH_X = 1.2f;       // max horizontal stretch on landing
    constexpr float LANDING_SQUASH_Y = 0.7f;       // max vertical squash on landing

    // Player types
    enum class DinoType {
        TREX,
        VELOCIRAPTOR,
        TRICERATOPS
    };

    // Enums
    enum class GameStateType {
        MENU,
        PLAYING,
        PAUSED,
        GAME_OVER,
        LEVEL_SELECT
    };

    enum class EnemyType {
        SMALL_CACTUS,
        LARGE_CACTUS,
        PTERODACTYL,
        ROLLING_ROCK,
        GROUND_ENEMY
    };

    enum class PowerUpType {
        SHIELD,
        MAGNET,
        DOUBLE_POINTS,
        SPEED_BOOST,
        EXTRA_LIFE
    };

    enum class LevelType {
        DESERT_DAY,
        DESERT_NIGHT,
        CAVE,
        VOLCANO,
        INFINITE,
        GRAYSCALE
    };

    // ECS
    constexpr int MAX_ENTITIES = 10000;

    // Gameplay
    constexpr float COIN_SCORE = 100.0f;
    constexpr float DISTANCE_SCORE_MULT = 10.0f;
    constexpr int INITIAL_LIVES = 3;
    constexpr int MAX_LIVES = 5;
    constexpr float POWERUP_DURATION = 8.0f;
    constexpr float MAGNET_RADIUS = 200.0f;

    // Colors
    namespace Colors {
        constexpr uint32_t DESERT_SKY = 0x87CEEBFF;
        constexpr uint32_t DESERT_GROUND = 0xD2B48CFF;
        constexpr uint32_t NIGHT_SKY = 0x1a1a2eff;
        constexpr uint32_t NIGHT_GROUND = 0x2d2d44ff;
        constexpr uint32_t CAVE_DARK = 0x0d0d0dff;
        constexpr uint32_t VOLCANO_SKY = 0x4a0000ff;
        constexpr uint32_t LAVA_COLOR = 0xff4500ff;
    }
}

#endif // CORE_CONSTANTS_H
