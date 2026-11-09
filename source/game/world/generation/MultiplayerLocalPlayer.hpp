#ifndef GAME_WORLD_GENERATION_MULTIPLAYERLOCALPLAYER_HPP
#define GAME_WORLD_GENERATION_MULTIPLAYERLOCALPLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/generation/LocalPlayer.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class ClientPacketListener;
class DamageSource;
class ItemEntity;
class MobEffectInstance;
class Stat;
class Entity;

class MultiplayerLocalPlayer : public LocalPlayer
{
public:
    MultiplayerLocalPlayer(Minecraft* minecraft, Level* level, User* user, ClientPacketListener* packetListener);
    virtual ~MultiplayerLocalPlayer();

    MultiPlayerLevel* GetLevel();
    virtual bool hurt(const DamageSource* source, float damage);
    virtual void heal(float amount);
    virtual void tick();
    virtual boost::shared_ptr<ItemEntity> drop();
    not_null_ptr<ItemInstance> reallyDrop(boost::shared_ptr<ItemEntity> item);
    virtual void respawn();
    virtual void actuallyHurt(const DamageSource* source, float damage);
    virtual void completeUsingItem();
    virtual void onEffectAdded(MobEffectInstance* effect);
    virtual void onEffectUpdated(MobEffectInstance* effect, bool attributesChanged);
    virtual void onEffectRemoved(MobEffectInstance* effect);
    virtual void closeContainer();
    virtual void hurtTo(float amount, TelemetryDamageSource source);
    virtual void awardStat(Stat* stat, arrayWithLength<unsigned char> params);
    virtual bool isLocalPlayer();
    virtual bool startRiding(boost::shared_ptr<Entity> vehicle, bool force);
    virtual void sendRidingJump();
    virtual void sendOpenInventory();
    virtual void StopSleeping();
    virtual void setAndBroadcastCustomSkin(unsigned int skin);
    virtual void setAndBroadcastCustomCape(unsigned int cape);

    void fjResetBounceHeight();
    void clientSideCloseContainer();
    void awardStatFromServer(Stat* stat, arrayWithLength<unsigned char> params);
    void onSlimeBlockBounce();

private:
    bool mHurt;
    int mScoreboardWait;
    bool mHasSentOpenInventory;
    bool mHasSentRidingJump;
    bool mReceivedRespawn;
    bool mField0xa1b;
    int mLastSentStat;
    double* mBounceHeight;
    unsigned int mFreeDiverTicks;
};

#endif
