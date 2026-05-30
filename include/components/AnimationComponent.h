#ifndef COMPONENTS_ANIMATION_H
#define COMPONENTS_ANIMATION_H

#include "../ecs/Entity.h"
#include <SFML/Graphics/Rect.hpp>
#include <vector>
#include <string>
#include <unordered_map>

struct AnimationFrame {
    sf::IntRect rect;
    float duration = 0.1f;
};

struct AnimationComponent : public Component {
    std::unordered_map<std::string, std::vector<AnimationFrame>> animations;
    std::vector<AnimationFrame>* currentFrames = nullptr;
    int currentFrame = 0;
    float timer = 0.0f;
    bool playing = true;
    bool looping = true;
    float speed = 1.0f;
    std::string currentAnimation;
    bool flipped = false;

    explicit AnimationComponent() = default;

    /// Register a named animation with its frame list
    void addAnimation(const std::string& name, const std::vector<AnimationFrame>& frames) {
        animations[name] = frames;
        // Auto-play if this is the first animation added
        if (!currentFrames) {
            play(name);
        }
    }

    /// Switch to a named animation, resetting to the first frame
    void play(const std::string& name) {
        if (currentAnimation == name && currentFrames) return;
        auto it = animations.find(name);
        if (it != animations.end()) {
            currentAnimation = name;
            currentFrames = &it->second;
            currentFrame = 0;
            timer = 0.0f;
            playing = true;
        }
    }

    /// Returns true if the named animation exists
    bool hasAnimation(const std::string& name) const {
        return animations.find(name) != animations.end();
    }

    void update(float dt) {
        if (!playing || !currentFrames || currentFrames->empty()) return;
        timer += dt * speed;
        if (timer >= (*currentFrames)[currentFrame].duration) {
            timer = 0.0f;
            currentFrame++;
            if (currentFrame >= static_cast<int>(currentFrames->size())) {
                if (looping) currentFrame = 0;
                else {
                    currentFrame = static_cast<int>(currentFrames->size()) - 1;
                    playing = false;
                }
            }
        }
    }

    sf::IntRect getCurrentFrame() const {
        if (!currentFrames || currentFrames->empty()) return {};
        if (currentFrame >= static_cast<int>(currentFrames->size())) return {};
        return (*currentFrames)[currentFrame].rect;
    }

    void reset() {
        currentFrame = 0;
        timer = 0.0f;
        playing = true;
    }
};

#endif
