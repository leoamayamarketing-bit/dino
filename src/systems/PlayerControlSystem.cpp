#include "../../include/systems/PlayerControlSystem.h"
#include "../../include/core/AssetManager.h"
#include "../../include/components/TransformComponent.h"
#include "../../include/components/PhysicsComponent.h"
#include "../../include/components/PlayerComponent.h"
#include "../../include/components/SpriteComponent.h"
#include "../../include/components/AnimationComponent.h"
#include "../../include/core/AudioManager.h"
#include "../../include/core/Constants.h"

PlayerControlSystem::PlayerControlSystem(InputManager& input, GameState& state,
                                         AudioManager* audio, AssetManager* assets)
    : input_(input), state_(state), audio_(audio), assetManager_(assets) {}

void PlayerControlSystem::update(float deltaTime, std::vector<Entity*>& entities) {
    for (auto* entity : entities) {
        auto* player = entity->getComponent<PlayerComponent>();
        auto* physics = entity->getComponent<PhysicsComponent>();
        auto* transform = entity->getComponent<TransformComponent>();
        auto* anim = entity->getComponent<AnimationComponent>();
        auto* sprite = entity->getComponent<SpriteComponent>();

        if (!player || !physics || !transform) continue;

        // ------------------------------------------------------------------
        // 1. Update timers
        // ------------------------------------------------------------------
        bool wasGrounded = wasGrounded_;
        wasGrounded_ = physics->isGrounded;

        // Coyote timer: active when leaving ground without having jumped
        if (wasGrounded && !physics->isGrounded) {
            coyoteTimer_ = Constants::COYOTE_TIME;
        } else if (!physics->isGrounded) {
            coyoteTimer_ -= deltaTime;
        } else {
            coyoteTimer_ = 0.0f; // grounded resets
        }

        // Jump buffer timer: counts down when a jump was buffered
        if (jumpBufferTimer_ > 0.0f) {
            jumpBufferTimer_ -= deltaTime;
        }

        // Squash timer: visual feedback on landing
        if (squashTimer_ > 0.0f) {
            squashTimer_ -= deltaTime;
        }

        // ------------------------------------------------------------------
        // 2. Detect jump just-pressed and just-released (action-level)
        // ------------------------------------------------------------------
        bool jumpPressed = input_.isJumpPressed();
        bool jumpJustPressed = jumpPressed && !wasJumpPressed_;
        bool jumpJustReleased = !jumpPressed && wasJumpPressed_;
        wasJumpPressed_ = jumpPressed;

        // ------------------------------------------------------------------
        // 3. Variable jump height: cut upward velocity when jump is released
        // ------------------------------------------------------------------
        if (jumpJustReleased && physics->velocity.y < 0.0f) {
            physics->velocity.y *= Constants::JUMP_CUT_MULTIPLIER;
        }

        // ------------------------------------------------------------------
        // 4. Determine effective grounded state (real ground + coyote time)
        // ------------------------------------------------------------------
        bool effectivelyGrounded = physics->isGrounded || coyoteTimer_ > 0.0f;

        // ------------------------------------------------------------------
        // 5. Jump execution
        // ------------------------------------------------------------------
        if (jumpJustPressed && physics->jumpsAvailable > 0) {
            // Consume one jump
            physics->jumpsAvailable--;
            physics->isGrounded = false;
            coyoteTimer_ = 0.0f; // cancel coyote once we jump

            if (effectivelyGrounded) {
                // Ground jump (or coyote-assisted jump) — full velocity
                physics->velocity.y = player->jumpVelocity;
            } else {
                // Double jump — weaker
                physics->velocity.y = player->doubleJumpVelocity;
            }

            if (audio_) {
                audio_->playSound("jump");
            }
        }

        // ------------------------------------------------------------------
        // 6. Buffer jump when pressed in the air with no jumps left
        // ------------------------------------------------------------------
        if (jumpJustPressed && physics->jumpsAvailable <= 0 && !effectivelyGrounded) {
            jumpBufferTimer_ = Constants::JUMP_BUFFER_TIME;
        }

        // ------------------------------------------------------------------
        // 7. Land on ground (after being in air)
        // ------------------------------------------------------------------
        if (physics->isGrounded && !wasGrounded) {
            // Check if we have a buffered jump
            if (jumpBufferTimer_ > 0.0f) {
                jumpBufferTimer_ = 0.0f;
                // Execute jump immediately on landing — uses one jump
                physics->jumpsAvailable = std::max(0, physics->jumpsAvailable - 1);
                physics->isGrounded = false;
                wasGrounded_ = false;  // prevent coyote from starting next frame
                physics->velocity.y = player->jumpVelocity;

                if (audio_) {
                    audio_->playSound("jump");
                }
            } else {
                // Normal landing
                squashTimer_ = Constants::LANDING_SQUASH_DURATION;
                if (audio_) {
                    audio_->playSound("land");
                }
            }
        }

        // ------------------------------------------------------------------
        // 8. Crouch (with progressive animation)
        // ------------------------------------------------------------------
        bool crouchPressed = input_.isCrouchPressed() && physics->isGrounded;
        player->isCrouching = crouchPressed;

        // ------------------------------------------------------------------
        // 9. Apply squash / crouch / normal scale
        // ------------------------------------------------------------------
        if (sprite) {
            if (squashTimer_ > 0.0f) {
                // Landing squash — brief stretch then fade
                float t = squashTimer_ / Constants::LANDING_SQUASH_DURATION; // 1 → 0
                float sx = 1.0f + (Constants::LANDING_SQUASH_X - 1.0f) * t;
                float sy = 1.0f - (1.0f - Constants::LANDING_SQUASH_Y) * t;
                sprite->sprite.setScale(sx, sy);
            } else if (player->isCrouching) {
                sprite->sprite.setScale(1.0f, player->crouchHeight);
            } else {
                sprite->sprite.setScale(1.0f, 1.0f);
            }
        }

        // ------------------------------------------------------------------
        // 10. Dash
        // ------------------------------------------------------------------
        if (input_.isDashPressed() && !physics->isDashing && physics->dashCooldown <= 0.0f) {
            physics->isDashing = true;
            physics->dashTimer = Constants::DASH_DURATION;
            physics->dashCooldown = Constants::DASH_COOLDOWN;

            if (audio_) {
                audio_->playSound("dash");
            }
        }

        // ------------------------------------------------------------------
        // 11. Animation state switching (with progressive crouch)
        // ------------------------------------------------------------------
        if (anim) {
            if (!physics->isGrounded && anim->hasAnimation("jump")) {
                if (anim->currentAnimation != "jump") {
                    anim->play("jump");
                    anim->looping = false;
                    crouchProgress_ = 0.0f;
                }
            } else if (player->isCrouching) {
                // ─── Crouch animation (progressive if PNG textures available) ──
                if (anim->hasAnimation("crouch")) {
                    if (anim->currentAnimation != "crouch") {
                        anim->play("crouch");
                        anim->looping = true;
                        crouchProgress_ = 0.0f;
                    }

                    // Progressive crouch: animate through frames 01→07
                    if (anim->useSeparateTextures && assetManager_) {
                        crouchProgress_ += deltaTime * CROUCH_SPEED;
                        if (crouchProgress_ > 1.0f) crouchProgress_ = 1.0f;

                        // Map progress 0→1 to frames dino_frame_01 through dino_frame_07
                        int frameIdx = FIRST_CROUCH_FRAME +
                            static_cast<int>(crouchProgress_ * (NUM_CROUCH_FRAMES - 1));
                        if (frameIdx > FIRST_CROUCH_FRAME + NUM_CROUCH_FRAMES - 1)
                            frameIdx = FIRST_CROUCH_FRAME + NUM_CROUCH_FRAMES - 1;

                        // Build the texture name for this frame
                        std::string texName = "dino_frame_";
                        if (frameIdx < 10) texName += "0";
                        texName += std::to_string(frameIdx);

                        if (assetManager_->hasTexture(texName) && sprite) {
                            sf::Texture& tex = assetManager_->getTexture(texName);
                            sprite->sprite.setTexture(tex);
                            sprite->sprite.setTextureRect(sf::IntRect(0, 0,
                                static_cast<int>(tex.getSize().x),
                                static_cast<int>(tex.getSize().y)));
                        }
                    }
                }
            } else if (!player->isCrouching && anim->currentAnimation == "crouch") {
                // ─── Standing up animation ────────────────────────────────
                // Show "stand" frame (dino_08.png) briefly before returning to run
                if (anim->hasAnimation("stand")) {
                    anim->play("stand");
                    anim->looping = false;
                    standTimer_ = STAND_DURATION;

                    // Set stand texture directly
                    if (anim->useSeparateTextures && assetManager_ && sprite) {
                        std::string texName = "dino_frame_08";
                        if (assetManager_->hasTexture(texName)) {
                            sf::Texture& tex = assetManager_->getTexture(texName);
                            sprite->sprite.setTexture(tex);
                            sprite->sprite.setTextureRect(sf::IntRect(0, 0,
                                static_cast<int>(tex.getSize().x),
                                static_cast<int>(tex.getSize().y)));
                        }
                    }
                } else if (anim->hasAnimation("run")) {
                    anim->play("run");
                    anim->looping = true;
                }
                crouchProgress_ = 0.0f;
            } else if (anim->currentAnimation == "stand") {
                // Wait for stand animation to finish, then switch to run
                standTimer_ -= deltaTime;
                if (standTimer_ <= 0.0f && anim->hasAnimation("run")) {
                    anim->play("run");
                    anim->looping = true;
                }
            } else {
                // Default: run animation
                if (anim->currentAnimation != "run" && anim->hasAnimation("run")) {
                    anim->play("run");
                    anim->looping = true;
                }
                crouchProgress_ = 0.0f;
            }
        }
    }
}
