#ifndef SYSTEMS_PLAYER_CONTROL_H
#define SYSTEMS_PLAYER_CONTROL_H

#include "../ecs/Entity.h"
#include "../core/InputManager.h"
#include "../core/GameState.h"

class AudioManager;
class AssetManager;

class PlayerControlSystem : public System {
public:
    PlayerControlSystem(InputManager& input, GameState& state,
                        AudioManager* audio = nullptr,
                        AssetManager* assets = nullptr);
    void setAudioManager(AudioManager* audio) { audio_ = audio; }
    void setAssetManager(AssetManager* assets) { assetManager_ = assets; }
    void update(float deltaTime, std::vector<Entity*>& entities) override;

private:
    InputManager& input_;
    GameState& state_;
    AudioManager* audio_ = nullptr;

    // Landing / ground detection
    bool wasGrounded_ = true;

    // Jump feel helpers
    float coyoteTimer_ = 0.0f;      // remaining coyote time (allows jump after leaving ground)
    float jumpBufferTimer_ = 0.0f; // remaining jump buffer time (press jump early, executes on land)
    float squashTimer_ = 0.0f;     // remaining landing squash time

    // Jump-pressed tracking for detecting just-pressed / just-released
    bool wasJumpPressed_ = false;

    // ─── Progressive crouch animation ───────────────────────────────────
    /// Current crouch progress (0.0 → 1.0) for progressive frame animation
    float crouchProgress_ = 0.0f;

    /// Timer for the "stand up" animation (showing dino_08.png)
    float standTimer_ = 0.0f;

    /// Duration of the stand-up animation in seconds
    static constexpr float STAND_DURATION = 0.15f;

    /// Speed at which crouch animation progress advances (per second)
    static constexpr float CROUCH_SPEED = 2.5f;

    /// Number of crouch frames available (dino_frame_01 through dino_frame_07)
    static constexpr int NUM_CROUCH_FRAMES = 7;

    /// Index of the first crouch frame in the animation sequence
    static constexpr int FIRST_CROUCH_FRAME = 1;

    AssetManager* assetManager_ = nullptr;
};

#endif
