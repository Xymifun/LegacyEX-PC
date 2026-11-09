#ifndef GAME_WORLD_BLOCK_CAULDRONBLOCKENTITY_HPP
#define GAME_WORLD_BLOCK_CAULDRONBLOCKENTITY_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/world/block/BlockEntity.hpp"
#include "game/item/Container.hpp"
#include "game/item/NonNullList.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/core/CensoredString.hpp"
#include "game/item/Potion.hpp"

class BlockPos;
class CompoundTag;
class Level;
class Packet;
class Player;
class ItemInstance;

class CauldronBlockEntity : public BlockEntity, public Container
{
public:
    CauldronBlockEntity();
    virtual ~CauldronBlockEntity();

    static BlockEntity* create();
    virtual not_null_ptr<ItemInstance> getItem(unsigned int slot);
    virtual not_null_ptr<ItemInstance> removeItem(unsigned int slot, int count);
    virtual not_null_ptr<ItemInstance> removeItemNoUpdate(int slot);
    virtual void setItem(unsigned int slot, not_null_ptr<ItemInstance> item);
    virtual std::wstring getName();
    virtual std::wstring getCustomName();
    virtual bool hasCustomName();
    virtual int getMaxStackSize();
    virtual bool stillValid(boost::shared_ptr<Player> player);
    virtual unsigned int getContainerSize();
    virtual bool isEmpty();
    virtual void startOpen(boost::shared_ptr<Player> player);
    virtual void stopOpen(boost::shared_ptr<Player> player);
    virtual bool canPlaceItem(int slot, not_null_ptr<ItemInstance> item);
    virtual int getContainerData(int id);
    virtual void setContainerData(int id, int value);
    virtual int getContainerDataCount();
    virtual void clearContent();

    virtual std::wstring getDisplayName();
    std::wstring getPotionId() const;
    void setPotionId(const std::wstring& potionId);
    void setCustomColor(const int& color);
    int getPotionColor() const;
    int getCustomColor() const;
    int getColor() const;
    Potion::EType getPotionType() const;
    void setPotionType(Potion::EType type);
    int getMixDyeColor();
    void mixDyes();

    virtual void load(CompoundTag* tag);
    void loadFromPacket(Level* level, const BlockPos& pos, CompoundTag* tag);
    virtual CompoundTag* save(CompoundTag* tag);
    virtual void tick();
    virtual boost::shared_ptr<Packet> getUpdatePacket();
    virtual CompoundTag* getUpdateTag();
    virtual boost::shared_ptr<BlockEntity> clone();
    virtual void setChanged();
    virtual unsigned long long GetType();

    void setCustomName(const std::wstring& name, const std::wstring& uuid, bool chatCensored, bool uuidLocal);
    bool isSplashPotion() const;
    bool isLingeringPotion() const;

private:
    friend class BlockRenderer;
    friend class CauldronBlock;

    std::wstring mPotionId;
    Potion::EType mPotionType;
    bool mHasCustomColor;
    int mCustomColor;
    NonNullList<not_null_ptr<ItemInstance> > mItems;
    boost::shared_ptr<CensoredString> mCustomName;
};

#endif
