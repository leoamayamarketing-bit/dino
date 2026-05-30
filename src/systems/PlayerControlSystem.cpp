#include "../../include/systems/PlayerControlSystem.h"
#include "../../include/components/TransformComponent.h"
#include "../../include/components/PhysicsComponent.h"
#include "../../include/components/PlayerComponent.h"
#include "../../include/components/SpriteComponent.h"
#include "../../include/components/AnimationComponent.h"
#include "../../include/core/AudioManager.h"

PlayerControlSystem::PlayerControlSystem(InputManager& input, GameState& state, AudioManager* audio)
    : input_(input), state_(state), audio_(audio) {}

void PlayerControlSystem::update(float deltaTime, std::vector<Entity*>& entities) {
    for (auto* entity : entities) {
        auto* player = entity->getComponent<PlayerComponent>();
        auto* physics = entity->getComponent<PhysicsComponent>();
        auto* transform = entity->getComponent<TransformComponent>();
        auto* anim = entity->getComponent<AnimationComponent>();

        if (!player || !physics || !transform) continue;

        bool wasGrounded = wasGrounded_;
        wasGrounded_ = physics->isGrounded;

        // Jump
        if (input_.isJumpPressed() && physics->jumpsAvailable > 0) {
            physics->isGrounded = false;
            physics->jumpsAvailable--;

            if (physics->jumpsAvailable == 1) {
                physics->velocity.y = player->jumpVelocity;
            } else {
                physics->velocity.y = player->doubleJumpVelocity;
            }

            // Play jump sound
            if (audio_) {
                audio_->playSound("jump");
            }
        }

        // Land on ground (after being in air)
        if (physics->isGrounded && !wasGrounded) {
            if (audio_) {
                audio_->playSound("land");
            }
        }

        // Crouch
        player->isCrouching = input_.isCrouchPressed() && physics->isGrounded;
        auto* sprite = entity->getComponent<SpriteComponent>();
        if (sprite) {
            if (player->isCrouching) {
                sprite->sprite.setScale(1.0f, player->crouchHeight);
            } else {
                sprite->sprite.setScale(1.0f, 1.0f);
            }
        }

        // Dash
        if (input_.isDashPressed() && !physics->isDashing && physics->dashCooldown <= 0.0f) {
            physics->isDashing = true;
            physics->dashTimer = Constants::DASH_DURATION;
            physics->dashCooldown = Constants::DASH_COOLDOWN;

            if (audio_) {
                audio_->playSound("dash");
            }
        }

        // --- Animation state switching ---
        if (anim) {
            if (!physics->isGrounded && anim->hasAnimation("jump")) {
                // In the air → jump animation (no loop)
                if (anim->currentAnimation != "jump") {
                    anim->play("jump");
                    anim->looping = false;
                }
            } else if (player->isCrouching && anim->hasAnimation("crouch")) {
                // Crouching → crouch animation
                if (anim->currentAnimation != "crouch") {
                    anim->play("crouch");
                    anim->looping = true;
                }
            } else {
                // Running on ground → run animation (looping)
                if (anim->currentAnimation != "run") {
                    anim->play("run");
                    anim->looping = true;
                }
            }
        }

        // Don't scroll player - instead the world moves around them
        // Entity position is maintained by the world scrolling
    }
}
