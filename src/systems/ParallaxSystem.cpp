#include "../../include/systems/ParallaxSystem.h"
#include <SFML/Graphics/RenderWindow.hpp>

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

void ParallaxSystem::addLayer(const sf::Texture& texture, float speedMultiplier) {
    ParallaxLayer layer;
    layer.sprite.setTexture(texture);
    layer.speedMultiplier = speedMultiplier;
    layer.width = static_cast<float>(texture.getSize().x);
    layer.offsetX = 0.0f;
    layers_.push_back(layer);
}

void ParallaxSystem::render(sf::RenderWindow& window) {
    for (auto& layer : layers_) {
        // Draw two copies for seamless scrolling
        layer.sprite.setPosition(-layer.offsetX, 0);
        window.draw(layer.sprite);

        layer.sprite.setPosition(-layer.offsetX + layer.width, 0);
        window.draw(layer.sprite);
    }
}

void ParallaxSystem::clear() {
    layers_.clear();
}
