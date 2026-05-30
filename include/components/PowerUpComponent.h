#ifndef COMPONENTS_POWERUP_H
#define COMPONENTS_POWERUP_H

#include "../ecs/Entity.h"
#include "../core/Constants.h"
#include <SFML/Graphics/Color.hpp>

struct PowerUpComponent : public Component {
    Constants::PowerUpType powerUpType;
    float duration = Constants::POWERUP_DURATION;
    bool active = false;
    bool collected = false;

    explicit PowerUpComponent(Constants::PowerUpType type)
        : powerUpType(type) {}

    const char* getTextureName() const {
        switch (powerUpType) {
            case Constants::PowerUpType::SHIELD: return "powerup_shield";
            case Constants::PowerUpType::MAGNET: return "powerup_magnet";
            case Constants::PowerUpType::DOUBLE_POINTS: return "powerup_double";
            case Constants::PowerUpType::SPEED_BOOST: return "powerup_speed";
            case Constants::PowerUpType::EXTRA_LIFE: return "powerup_life";
        }
        return "powerup_shield";
    }

    sf::Color getColor() const {
        switch (powerUpType) {
            case Constants::PowerUpType::SHIELD: return sf::Color::Cyan;
            case Constants::PowerUpType::MAGNET: return sf::Color::Magenta;
            case Constants::PowerUpType::DOUBLE_POINTS: return sf::Color::Yellow;
            case Constants::PowerUpType::SPEED_BOOST: return sf::Color::Green;
            case Constants::PowerUpType::EXTRA_LIFE: return sf::Color::Red;
        }
        return sf::Color::White;
    }
};

#endif
