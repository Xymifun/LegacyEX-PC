#ifndef GAME_CORE_MINECARTHOPPER_HPP
#define GAME_CORE_MINECARTHOPPER_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/Hopper.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/item/MinecartContainer.hpp"
#include "game/world/block/BlockPos.hpp"

class AbstractContainerMenu;
class BlockState;
class CompoundTag;
class DamageSource;
class DataFixerUpper;
class Inventory;
class Level;
class Player;

class MinecartHopper : public MinecartContainer, public virtual Hopper
{
public:
    MinecartHopper(Level* level);
    MinecartHopper(Level* level, double x, double y, double z);
    virtual ~MinecartHopper();

    static Entity* create(Level* level);
    virtual unsigned int getContainerSize();
    virtual not_null_ptr<ItemInstance> getItem(unsigned int slot);
    virtual not_null_ptr<ItemInstance> removeItem(unsigned int slot, int count);
    virtual not_null_ptr<ItemInstance> removeItemNoUpdate(int slot);
    virtual void setItem(unsigned int slot, not_null_ptr<ItemInstance> item);
    virtual int getMaxStackSize();
    virtual void setChanged();
    virtual bool stillValid(boost::shared_ptr<Player> player);
    virtual void startOpen(boost::shared_ptr<Player> player);
    virtual void stopOpen(boost::shared_ptr<Player> player);
    virtual bool canPlaceItem(int slot, not_null_ptr<ItemInstance> item);
    virtual bool isEnabled();
    virtual void setEnabled(bool enabled);
    virtual Level* getLevel();
    virtual double getLevelX();
    virtual double getLevelY();
    virtual double getLevelZ();
    virtual bool suckInItems();
    virtual int getTypeId(bool large);
    virtual AbstractContainerMenu* createMenu(boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Player> player);

    void _init();
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual const Minecart::Type* getType();
    virtual const BlockState* getDefaultDisplayBlockState();
    virtual int getDefaultDisplayOffset();
    virtual bool interact(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual void activateMinecart(int x, int y, int z, bool powered);
    void setCooldown(int cooldown);
    bool isOnCooldown();
    virtual void tick();
    virtual void destroy(const DamageSource* source);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual unsigned long long GetType();

private:
    bool mEnabled;
    int mCooldown;
    BlockPos mPosition;
};

#endif
