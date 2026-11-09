#ifndef GAME_WORLD_BLOCK_SKULLBLOCKENTITY_HPP
#define GAME_WORLD_BLOCK_SKULLBLOCKENTITY_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/item/not_null_ptr.hpp"
#include "game/world/block/BlockEntity.hpp"

class CompoundTag;
class ItemInstance;
class Level;
class Mirror;
class Packet;
class Rotation;

class SkullBlockEntity : public BlockEntity
{
public:
    SkullBlockEntity();

    static BlockEntity* create();
    int getSkullType();
    static int ValidateSkullType(int type);
    void setSkullType(int type, const std::wstring& extraType);
    void setRotation(int rotation);
    virtual CompoundTag* save(CompoundTag* tag);
    virtual void load(CompoundTag* tag);
    virtual void tick();
    float getMouthAnimation(float partialTick);
    virtual boost::shared_ptr<Packet> getUpdatePacket();
    virtual CompoundTag* getUpdateTag();
    bool isRenderReady();
    int getRotation();
    virtual void rotate(Rotation* rotation);
    virtual void mirror(Mirror* mirror);
    virtual boost::shared_ptr<BlockEntity> clone();
    virtual unsigned long long GetType();
    std::wstring getExtraType();
    void setItemValues(not_null_ptr<ItemInstance> item);

private:
    int mSkullType;
    int mRotation;
    std::wstring mExtraType;
    int mMouthAnimation;
    bool mPowered;
};

#endif
