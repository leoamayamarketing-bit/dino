#ifndef COMPONENTS_COIN_H
#define COMPONENTS_COIN_H

#include "../ecs/Entity.h"

struct CoinComponent : public Component {
    float value = 100.0f;
    bool collected = false;

    explicit CoinComponent(float val = 100.0f) : value(val) {}
};

#endif
