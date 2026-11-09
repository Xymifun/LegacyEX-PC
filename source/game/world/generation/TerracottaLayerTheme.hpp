#ifndef GAME_WORLD_GENERATION_TERRACOTTALAYERTHEME_HPP
#define GAME_WORLD_GENERATION_TERRACOTTALAYERTHEME_HPP

#include <boost/functional/hash.hpp>

#include "game/world/generation/LayerTheme.hpp"

class Random;
class BlockTransaction;
class Direction;

typedef unsigned char (*TerracottaLayerThemeFunction)(int, int);

class TerracottaLayerTheme : public LayerTheme
{
public:
    struct Pattern
    {
        int size;
        const Direction* values[16];
    };

    TerracottaLayerTheme(const Pattern& pattern, TerracottaLayerThemeFunction function);

    static unsigned int CONCENTRIC_RINGS(int x, int z);
    static unsigned int OFFSET_RINGS(int x, int z);
    static unsigned int HASHED(int x, int z);
    static unsigned char TARTAN(int x, int z);
    static unsigned int STRIPES(int x, int z);
    static unsigned int UNIFORM(int x, int z);

    virtual void Apply(Random& random, BlockTransaction* transaction, int& width, int& height) const;

private:
    Pattern mPattern;
    TerracottaLayerThemeFunction mFunction;
};

#endif
