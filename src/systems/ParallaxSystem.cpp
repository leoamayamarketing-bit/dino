#include "../../include/systems/ParallaxSystem.h"
#include "../../include/core/Constants.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <cmath>

void ParallaxSystem::update(float deltaTime, std::vector<Entity*>&) {
    for (auto& layer : layers_) {
        layer.offsetX += scrollX_ * layer.speedMultiplier * deltaTime;
        // Prevent float precision loss from unbounded growth.
        // render() uses std::fmod so the visual result is identical.
        if (layer.offsetX > layer.width * 1000.0f) {
            layer.offsetX -= layer.width * 1000.0f;
        }
        if (layer.offsetX < -layer.width * 1000.0f) {
            layer.offsetX += layer.width * 1000.0f;
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
        float y = std::round(layer.offsetY);

        // Apply grayscale tint if enabled
        if (grayscaleTint_) {
            layer.sprite.setColor(sf::Color(200, 200, 200));
        } else {
            layer.sprite.setColor(sf::Color::White);
        }

        // Use fmod to get the effective position within one tile width.
        // This prevents abrupt wrapping jumps and ensures smooth scrolling.
        float x = -std::fmod(layer.offsetX, layer.width);

        // Draw enough copies to tile across the entire screen width.
        // Important: piso.png (903px) is narrower than the 1280px screen.
        for (float px = x; px < static_cast<float>(Constants::WINDOW_WIDTH); px += layer.width) {
            layer.sprite.setPosition(px, y);
            window.draw(layer.sprite);
        }
    }
}

void ParallaxSystem::clear() {
    layers_.clear();
}
