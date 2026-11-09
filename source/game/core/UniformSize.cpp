#include "game/core/UniformSize.hpp"
#include <cstddef>

UniformSize::UniformSize(unsigned int size)
{
    UniformSize* obj = this;
    if (obj != NULL || (obj = static_cast<UniformSize*>(operator new(8)), obj != NULL))
    {
        obj->mSize = size;
    }
}

unsigned int UniformSize::GetLayerSize(Random& random, double blend) const
{
    return mSize;
}
