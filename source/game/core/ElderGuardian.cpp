#include "game/core/ElderGuardian.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/ai/RandomStrollGoal.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/network/ClientboundGameEventPacket.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/PlayerSelector.hpp"

const float ElderGuardian::ELDER_SIZE_SCALE = 2.35f;

ElderGuardian::ElderGuardian(Level* level) : Guardian(level, false)
{
    fjDerivedCtorCalls();
    setSize(mBbWidth * ELDER_SIZE_SCALE, mBbHeight * ELDER_SIZE_SCALE);
    setPersistenceRequired(true);
    if (mRandomStrollGoal != NULL)
    {
        mRandomStrollGoal->setInterval(400);
    }
}

void ElderGuardian::registerAttributes()
{
    Guardian::registerAttributes();
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.3F);
    getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(8.0);
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(80.0);
}

void ElderGuardian::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::ELDER_GUARDIAN);
}

ResourceLocation* ElderGuardian::getDefaultLootTable()
{
    return &BuiltInLootTables::ELDER_GUARDIAN;
}

unsigned int ElderGuardian::getAttackDuration()
{
    return 60;
}

void ElderGuardian::setGhost()
{
    mSpikesAnimation = 1.0f;
    mSpikesAnimationOld = 1.0f;
}

const SoundEvent* ElderGuardian::getAmbientSound()
{
    return isInWaterOrBubble() ? SoundEvents::ELDER_GUARDIAN_AMBIENT : SoundEvents::ELDER_GUARDIAN_AMBIENT_LAND;
}

const SoundEvent* ElderGuardian::getHurtSound(const DamageSource*)
{
    return isInWaterOrBubble() ? SoundEvents::ELDER_GUARDIAN_HURT : SoundEvents::ELDER_GUARDIAN_HURT_LAND;
}

const SoundEvent* ElderGuardian::getDeathSound()
{
    return isInWaterOrBubble() ? SoundEvents::ELDER_GUARDIAN_DEATH : SoundEvents::ELDER_GUARDIAN_DEATH_LAND;
}

const SoundEvent* ElderGuardian::getFlopSound()
{
    return SoundEvents::ELDER_GUARDIAN_FLOP;
}

void ElderGuardian::customServerAiStep()
{
    Mob::customServerAiStep();

    if ((mTickCount + getId()) % 1200 == 0)
    {
        MobEffect* effect = MobEffects::DIG_SLOWDOWN;
        PlayerSelector selector(this);
        std::vector<boost::shared_ptr<Player> >* players = mLevel->getPlayers(EntityType::SERVER_PLAYER, &selector);
        for (std::vector<boost::shared_ptr<Player> >::iterator playerIt = players->begin(); playerIt != players->end();
             ++playerIt)
        {
            boost::shared_ptr<ServerPlayer> player = boost::static_pointer_cast<ServerPlayer>(*playerIt);
            if (!player->hasEffect(effect) || player->getEffect(effect)->getAmplifier() < 2
                || player->getEffect(effect)->getDuration() < 1200)
            {
                player->mConnection->send(boost::shared_ptr<Packet>(
                    new ClientboundGameEventPacket(ClientboundGameEventPacket::GUARDIAN_ELDER_EFFECT, 0.0f)));
                player->addEffect(new MobEffectInstance(effect, 6000, 2), boost::shared_ptr<Entity>());
            }
        }
        delete players;
    }

    if (!hasRestriction())
    {
        restrictTo(BlockPos(this), 16);
    }
}

Entity* ElderGuardian::create(Level* level)
{
    return new ElderGuardian(level);
}

unsigned long long ElderGuardian::GetType()
{
    return EntityType::ELDER_GUARDIAN;
}
