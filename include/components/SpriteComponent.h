#ifndef COMPONENTS_SPRITE_H
#define COMPONENTS_SPRITE_H

#include "../ecs/Entity.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <string>

struct SpriteComponent : public Component {
    sf::Sprite sprite;
    sf::Texture* texture = nullptr;
    std::string textureName;
    sf::Color color{255, 255, 255, 255};
    int zOrder = 0;
    bool visible = true;

    /// Desired origin point for the sprite.
    /// Re-applied after every texture switch (since setTexture() resets origin).
    /// Default (0,0) = top-left, compatible with all existing entities.
    sf::Vector2f origin{0.0f, 0.0f};

    explicit SpriteComponent(const std::string& texName = "")
        : textureName(texName) {}

    void setTexture(sf::Texture& tex) {
        texture = &tex;
        sprite.setTexture(tex);
        sprite.setTextureRect(sf::IntRect(0, 0, tex.getSize().x, tex.getSize().y));
        sprite.setOrigin(origin);
    }

    void setColor(const sf::Color& c) {
        color = c;
        sprite.setColor(c);
    }

    /// Apply origin to the underlying sf::Sprite. Call after modifying origin.
    void applyOrigin() {
        sprite.setOrigin(origin);
    }
};

#endif
