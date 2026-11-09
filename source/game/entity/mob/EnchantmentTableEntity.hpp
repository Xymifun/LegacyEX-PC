#ifndef GAME_ENTITY_MOB_ENCHANTMENTTABLEENTITY_HPP
#define GAME_ENTITY_MOB_ENCHANTMENTTABLEENTITY_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/core/CensoredString.hpp"
#include "game/core/InteractionObject.hpp"
#include "game/util/Random.hpp"
#include "game/world/block/BlockEntity.hpp"

class AbstractContainerMenu;
class CompoundTag;
class EnchantmentMenu;
class EnchantTableRenderer;
class Inventory;
class Level;
class Player;

class EnchantmentTableEntity : public BlockEntity, public InteractionObject
{
public:
    EnchantmentTableEntity();
    virtual ~EnchantmentTableEntity();

    static BlockEntity* create();
    virtual std::wstring getName();
    virtual std::wstring getCustomName();
    virtual bool hasCustomName();
    virtual std::wstring getDisplayName();
    virtual AbstractContainerMenu* createMenu(boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Player> player);
    virtual int getTypeId(bool large);
    virtual InteractionObject::NameData GetNetworkNameData();
    void setCustomName(const std::wstring& name, const std::wstring& uuid, bool chatCensored, bool uuidLocal);
    virtual CompoundTag* save(CompoundTag* tag);
    virtual void load(CompoundTag* tag);
    virtual void tick();
    virtual boost::shared_ptr<BlockEntity> clone();
    virtual unsigned long long GetType();

private:
    friend class EnchantTableRenderer;

    int mTime;
    float mFlip;
    float mFlipOld;
    float mFlipTarget;
    float mFlipSpeed;
    float mOpen;
    float mOpenOld;
    float mRot;
    float mRotOld;
    float mRotTarget;
    Random* mRandom;
    boost::shared_ptr<CensoredString> mCustomName;
};

#endif
