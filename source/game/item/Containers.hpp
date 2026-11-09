#ifndef GAME_ITEM_CONTAINERS_HPP
#define GAME_ITEM_CONTAINERS_HPP

#include <boost/shared_ptr.hpp>

#include "game/item/ItemInstance.hpp"

class BlockPos;
class Container;
class Entity;
class Level;
class Random;

class Containers
{
public:
    Containers();
    ~Containers();

    static void dropItemStack(Level* level, double x, double y, double z, not_null_ptr<ItemInstance> itemInstance);
    static void dropContents(Level* level, double x, double y, double z, boost::shared_ptr<Container> container);
    static void dropContents(Level* level, const BlockPos& pos, boost::shared_ptr<Container> container);
    static void dropContents(Level* level, boost::shared_ptr<Entity> entity, boost::shared_ptr<Container> container);

private:
    static Random RANDOM;
};

#endif
