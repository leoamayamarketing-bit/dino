#include "../../include/systems/ParallaxSystem.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <cmath>

void ParallaxSystem::update(float deltaTime, std::vector<Entity*>&) {
    for (auto& layer : layers_) {
        layer.offsetX += scrollX_ * layer.speedMultiplier * deltaTime;
        if (layer.offsetX > layer.width) {
            layer.offsetX -= layer.width;
        }
        if (layer.offsetX < 0) {
            layer.offsetX += layer.width;
        }
    }
}

void ParallaxSystem::addLayer(const sf::Texture& texture, float speedMultiplier, float offsetY) {
    ParallaxLayer layer;
    layer.sprite.setTexture(texture);
    layer.speedMultiplier = speedMultiplier;
    layer.width = static_cast<float>(texture.getSize().x);
    layer.offsetX = 0.0f;
    layer.offsetY = offsetY;
    layers_.push_back(layer);
}

void ParallaxSystem::render(sf::RenderWindow& window) {
    for (auto& layer : layers_) {
        // Round positions to integer coordinates to prevent sub-pixel
        // rendering artifacts (1-pixel seams between tiled copies).
        float x = std::round(-layer.offsetX);
        float y = std::round(layer.offsetY);

        // Apply grayscale tint if enabled
        if (grayscaleTint_) {
            layer.sprite.setColor(sf::Color(200, 200, 200));
        } else {
            layer.sprite.setColor(sf::Color::White);
        }

        // Draw two copies for seamless scrolling
        layer.sprite.setPosition(x, y);
        window.draw(layer.sprite);

        layer.sprite.setPosition(x + layer.width, y);
        window.draw(layer.sprite);
    }
}

void ParallaxSystem::clear() {
    layers_.clear();
}
