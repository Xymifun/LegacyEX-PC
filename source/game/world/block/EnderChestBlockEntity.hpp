#ifndef GAME_WORLD_BLOCK_ENDERCHESTBLOCKENTITY_HPP
#define GAME_WORLD_BLOCK_ENDERCHESTBLOCKENTITY_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/core/InteractionObject.hpp"
#include "game/core/CensoredString.hpp"
#include "game/world/block/BlockEntity.hpp"

class Player;
class EnderChestRenderer;

class EnderChestBlockEntity : public BlockEntity
{
public:
    EnderChestBlockEntity();

    static BlockEntity* create();
    void setCustomName(const std::wstring& name, const std::wstring& uuid, bool chatCensored, bool uuidLocal);
    virtual void tick();
    virtual bool triggerEvent(int id, int param);
    virtual void setRemoved();
    void startOpen();
    void stopOpen();
    bool stillValid(boost::shared_ptr<Player> player);
    InteractionObject::NameData GetNetworkNameData();
    virtual boost::shared_ptr<BlockEntity> clone();
    virtual unsigned long long GetType();
    bool hasCustomName();
    std::wstring getName();
    std::wstring getCustomName();

private:
    friend class EnderChestRenderer;

    float mOpen;
    float mOpenOld;
    int mOpenCount;
    int mTickCount;
    boost::shared_ptr<CensoredString> mName;
};

#endif
