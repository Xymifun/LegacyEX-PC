#ifndef GAME_WORLD_BLOCK_IDMAP_HPP
#define GAME_WORLD_BLOCK_IDMAP_HPP

template <class T>
class IdMap
{
public:
    virtual int getId(T value) = 0;
    virtual T byId(int id) = 0;
};

#endif
