#ifndef GAME_CORE_MINECARTCHEST_HPP
#define GAME_CORE_MINECARTCHEST_HPP

#include <boost/shared_ptr.hpp>

#include "game/item/MinecartContainer.hpp"

class AbstractContainerMenu;
class BlockState;
class DamageSource;
class DataFixerUpper;
class Inventory;
class Level;
class Player;

class MinecartChest : public MinecartContainer
{
public:
    MinecartChest(Level* level);
    MinecartChest(Level* level, double x, double y, double z);
    virtual ~MinecartChest();

    static Entity* create(Level* level);
    virtual unsigned int getContainerType();
    virtual unsigned int getContainerSize();
    virtual int getTypeId(bool large);
    virtual AbstractContainerMenu* createMenu(boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Player> player);
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void destroy(const DamageSource* source);
    virtual const Minecart::Type* getType();
    virtual const BlockState* getDefaultDisplayBlockState();
    virtual int getDefaultDisplayOffset();
    virtual unsigned long long GetType();
};

#endif
