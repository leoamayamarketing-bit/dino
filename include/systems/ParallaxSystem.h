#ifndef SYSTEMS_PARALLAX_H
#define SYSTEMS_PARALLAX_H

#include "../ecs/Entity.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <vector>

struct ParallaxLayer {
    sf::Sprite sprite;
    float speedMultiplier;
    float width;
    float offsetX = 0.0f;
};

class ParallaxSystem : public System {
public:
    void update(float deltaTime, std::vector<Entity*>& entities) override;

    void addLayer(const sf::Texture& texture, float speedMultiplier);
    void setScrollDirection(float x) { scrollX_ = x; }
    void render(sf::RenderWindow& window);
    void clear();

private:
    std::vector<ParallaxLayer> layers_;
    float scrollX_ = 0.0f;
};

#endif
