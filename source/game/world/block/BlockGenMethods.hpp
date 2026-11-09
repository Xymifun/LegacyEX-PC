#ifndef GAME_WORLD_BLOCK_BLOCKGENMETHODS_HPP
#define GAME_WORLD_BLOCK_BLOCKGENMETHODS_HPP

#include "game/core/Direction.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class Level;

class BlockGenMethods
{
public:
    static void generateBox(Level* level, arrayWithLength<unsigned char> data, int x0, int y0, int z0, int x1, int y1,
                            int z1, unsigned char primary, unsigned char secondary);
    static void generateFrame(Level* level, arrayWithLength<unsigned char> data, int x0, int y0, int z0, int x1, int y1,
                              const Direction* direction, unsigned char primary, unsigned char secondary);
    static void generateDirectionLine(Level* level, arrayWithLength<unsigned char> data, int x0, int y0, int z0, int x1,
                                      int y1, int z1, unsigned char value, const Direction* firstDirection,
                                      const Direction* secondDirection);
};

#endif
