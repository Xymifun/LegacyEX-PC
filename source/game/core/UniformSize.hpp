#ifndef GAME_CORE_UNIFORMSIZE_HPP
#define GAME_CORE_UNIFORMSIZE_HPP

#include "game/core/LayerSize.hpp"

class UniformSize : public LayerSize
{
public:
    UniformSize(unsigned int size);

    virtual unsigned int GetLayerSize(Random& random, double blend) const;

private:
    int mSize;
};

#endif
