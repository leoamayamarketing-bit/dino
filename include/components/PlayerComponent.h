#ifndef COMPONENTS_PLAYER_H
#define COMPONENTS_PLAYER_H

#include "../ecs/Entity.h"
#include "../core/Constants.h"

struct PlayerComponent : public Component {
    Constants::DinoType dinoType;
    float jumpVelocity;
    float doubleJumpVelocity;
    float dashMultiplier;
    float maxHealth;
    float crouchHeight;

    bool isCrouching = false;
    bool hasShield = false;
    bool hasMagnet = false;

    explicit PlayerComponent(Constants::DinoType type = Constants::DinoType::TREX)
        : dinoType(type) {
        switch (type) {
            case Constants::DinoType::TREX:
                jumpVelocity = Constants::JUMP_VELOCITY;
                doubleJumpVelocity = Constants::DOUBLE_JUMP_VELOCITY;
                dashMultiplier = Constants::DASH_SPEED_MULTIPLIER;
                maxHealth = 3.0f;
                crouchHeight = Constants::CROUCH_HEIGHT;
                break;
            case Constants::DinoType::VELOCIRAPTOR:
                jumpVelocity = -500.0f;
                doubleJumpVelocity = -400.0f;
                dashMultiplier = 3.0f;
                maxHealth = 2.0f;
                crouchHeight = 0.6f;
                break;
            case Constants::DinoType::TRICERATOPS:
                jumpVelocity = -550.0f;
                doubleJumpVelocity = -450.0f;
                dashMultiplier = 1.5f;
                maxHealth = 5.0f;
                crouchHeight = 0.4f;
                break;
        }
    }
};

#endif
