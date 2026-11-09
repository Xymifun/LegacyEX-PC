#include "game/util/RandomizeSize.hpp"

#include <math.h>

#include "game/util/Random.hpp"

RandomizeSize::RandomizeSize(LayerSize* layerSize, float amount) : mLayerSize(layerSize), mAmount(amount)
{
}

unsigned int RandomizeSize::GetLayerSize(Random& random, double blend) const
{
    unsigned int size = mLayerSize->GetLayerSize(random, blend);
    unsigned int offset = (unsigned int)floorf((2.0f * mAmount * random.nextFloat() - 1.0f) * (float)size);
    return size + offset;
}
