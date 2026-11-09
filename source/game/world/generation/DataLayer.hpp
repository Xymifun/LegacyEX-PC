#ifndef GAME_WORLD_GENERATION_DATALAYER_HPP
#define GAME_WORLD_GENERATION_DATALAYER_HPP

#include "game/world/biome/arrayWithLength.hpp"

class DataLayer
{
public:
    DataLayer();
    DataLayer(arrayWithLength<unsigned char> data);
    ~DataLayer();

    int getPosition(int index);
    bool isFirst(int index);
    int get(int index);
    int getIndex(int x, int y, int z);
    int get(int x, int y, int z);
    void set(int index, int value);
    void set(int x, int y, int z, int value);
    arrayWithLength<unsigned char> getData();

private:
    arrayWithLength<unsigned char> m_data;
};

#endif
