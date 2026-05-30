#ifndef SYSTEMS_PLAYER_CONTROL_H
#define SYSTEMS_PLAYER_CONTROL_H

#include "../ecs/Entity.h"
#include "../core/InputManager.h"
#include "../core/GameState.h"

class AudioManager;

class PlayerControlSystem : public System {
public:
    PlayerControlSystem(InputManager& input, GameState& state, AudioManager* audio = nullptr);
    void setAudioManager(AudioManager* audio) { audio_ = audio; }
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
};

#endif
