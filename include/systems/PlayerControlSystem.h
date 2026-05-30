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
    bool wasGrounded_ = true;  // for detecting jump start
};

#endif
