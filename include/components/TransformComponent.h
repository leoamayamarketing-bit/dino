#ifndef COMPONENTS_TRANSFORM_H
#define COMPONENTS_TRANSFORM_H

#include "../ecs/Entity.h"
#include <SFML/System/Vector2.hpp>

struct TransformComponent : public Component {
    sf::Vector2f position;
    sf::Vector2f scale{1.0f, 1.0f};
    float rotation = 0.0f;

    explicit TransformComponent(float x = 0, float y = 0)
        : position(x, y) {}
};

#endif
